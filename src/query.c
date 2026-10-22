#include "query.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TAMANHO_LINHA_QUERY 4096

struct query {
    INSTRUCAO *instrucoes;
    size_t quantidade;
};

static char *pular_espacos(char *texto) {
    while (isspace((unsigned char)*texto))
        ++texto;
    return texto;
}

static char *copiar_texto(const char *texto) {
    size_t tamanho = strlen(texto);
    char *copia = malloc(tamanho + 1);
    if (copia != NULL)
        memcpy(copia, texto, tamanho + 1);
    return copia;
}

static bool operador_valido(const char *operador) {
    return strcmp(operador, "==") == 0 || strcmp(operador, "=") == 0 ||
           strcmp(operador, "!=") == 0 || strcmp(operador, "<") == 0 ||
           strcmp(operador, "<=") == 0 || strcmp(operador, ">") == 0 ||
           strcmp(operador, ">=") == 0;
}

static bool valor_filtro_ler(TIPO_CAMPO tipo, char *texto,
                              INSTRUCAO *instrucao) {
    char *fim;
    errno = 0;
    instrucao->valor.tipo = tipo;
    instrucao->valor.ausente = false;

    switch (tipo) {
    case CAMPO_TEXTO:
        if (*texto == '"') {
            size_t tamanho = strlen(texto);
            if (tamanho < 2 || texto[tamanho - 1] != '"')
                return false;
            texto[tamanho - 1] = '\0';
            ++texto;
        }
        instrucao->texto_filtro = copiar_texto(texto);
        if (instrucao->texto_filtro == NULL)
            return false;
        instrucao->valor.valor.texto = instrucao->texto_filtro;
        return true;

    case CAMPO_INTEIRO:
        instrucao->valor.valor.inteiro = strtoll(texto, &fim, 10);
        return errno == 0 && fim != texto && *fim == '\0';

    case CAMPO_REAL:
        instrucao->valor.valor.real = strtod(texto, &fim);
        return errno == 0 && fim != texto && *fim == '\0' &&
               isfinite(instrucao->valor.valor.real);

    case CAMPO_BOOLEANO:
        if (strcmp(texto, "true") == 0 || strcmp(texto, "True") == 0)
            instrucao->valor.valor.booleano = true;
        else if (strcmp(texto, "false") == 0 || strcmp(texto, "False") == 0)
            instrucao->valor.valor.booleano = false;
        else
            return false;
        return true;
    }
    return false;
}

static bool filtro_ler(char *argumentos, const ESQUEMA *esquema,
                        INSTRUCAO *instrucao) {
    char *nome = strtok(argumentos, " \t");
    char *operador = strtok(NULL, " \t");
    char *valor = strtok(NULL, "\r\n");
    if (nome == NULL || operador == NULL || valor == NULL ||
        !operador_valido(operador))
        return false;

    valor = pular_espacos(valor);
    size_t tamanho = strlen(valor);
    while (tamanho > 0 && isspace((unsigned char)valor[tamanho - 1]))
        valor[--tamanho] = '\0';
    if (*valor == '\0')
        return false;

    size_t indice;
    const COLUNA *coluna;
    if (!esquema_buscar(esquema, nome, &indice) ||
        (coluna = esquema_coluna(esquema, indice)) == NULL)
        return false;
    if (coluna->tipo == CAMPO_BOOLEANO &&
        strcmp(operador, "==") != 0 && strcmp(operador, "=") != 0 &&
        strcmp(operador, "!=") != 0)
        return false;

    instrucao->indice_campo = indice;
    strcpy(instrucao->operador, strcmp(operador, "=") == 0 ? "==" : operador);
    return valor_filtro_ler(coluna->tipo, valor, instrucao);
}

static bool tipo_instrucao_ler(const char *nome,
                                TIPO_INSTRUCAO *tipo) {
    if (strcmp(nome, "FILTER") == 0) *tipo = FILTER;
    else if (strcmp(nome, "SIM") == 0) *tipo = SIM;
    else if (strcmp(nome, "ACCEPT") == 0) *tipo = ACCEPT;
    else if (strcmp(nome, "ORDER") == 0) *tipo = ORDER;
    else if (strcmp(nome, "GROUP") == 0) *tipo = GROUP;
    else if (strcmp(nome, "CROSS") == 0) *tipo = CROSS;
    else return false;
    return true;
}

static void instrucao_limpar(INSTRUCAO *instrucao) {
    free((char *)instrucao->argumentos);
    free(instrucao->texto_filtro);
    *instrucao = (INSTRUCAO){0};
}

void query_liberar(QUERY **query) {
    if (query == NULL || *query == NULL)
        return;
    for (size_t i = 0; i < (*query)->quantidade; ++i)
        instrucao_limpar(&(*query)->instrucoes[i]);
    free((*query)->instrucoes);
    free(*query);
    *query = NULL;
}

static bool query_anexar(QUERY *query,
                            INSTRUCAO *instrucao) {
    if (query->quantidade >= SIZE_MAX / sizeof(*query->instrucoes))
        return false;
    INSTRUCAO *novas = realloc(
        query->instrucoes,
        (query->quantidade + 1) * sizeof(*query->instrucoes));
    if (novas == NULL)
        return false;
    query->instrucoes = novas;
    query->instrucoes[query->quantidade++] = *instrucao;
    *instrucao = (INSTRUCAO){0};
    return true;
}

bool query_ler(FILE *arquivo, const ESQUEMA *esquema, QUERY **saida) {
    if (arquivo == NULL || esquema == NULL || saida == NULL || *saida != NULL)
        return false;
    QUERY *query = calloc(1, sizeof(*query));
    if (query == NULL)
        return false;

    char linha[TAMANHO_LINHA_QUERY];
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        size_t tamanho = strlen(linha);
        if (tamanho == sizeof(linha) - 1 && linha[tamanho - 1] != '\n' &&
            !feof(arquivo))
            goto falha;
        while (tamanho > 0 && (linha[tamanho - 1] == '\n' ||
                               linha[tamanho - 1] == '\r'))
            linha[--tamanho] = '\0';

        char *comando = pular_espacos(linha);
        if (*comando == '\0' || *comando == '#')
            continue;
        char *argumentos = comando;
        while (*argumentos != '\0' && !isspace((unsigned char)*argumentos))
            ++argumentos;
        if (*argumentos != '\0')
            *argumentos++ = '\0';
        argumentos = pular_espacos(argumentos);

        INSTRUCAO instrucao = {0};
        if (!tipo_instrucao_ler(comando, &instrucao.tipo))
            goto falha_instrucao;
        if (*argumentos == '\0')
            goto falha_instrucao;
        instrucao.argumentos = copiar_texto(argumentos);
        if (instrucao.argumentos == NULL)
            goto falha_instrucao;
        if (instrucao.tipo == FILTER) {
            char *argumentos_filtro = copiar_texto(instrucao.argumentos);
            if (argumentos_filtro == NULL)
                goto falha_instrucao;
            bool valido = filtro_ler(argumentos_filtro, esquema, &instrucao);
            free(argumentos_filtro);
            if (!valido)
                goto falha_instrucao;
        }
        if (!query_anexar(query, &instrucao))
            goto falha_instrucao;
        continue;

falha_instrucao:
        instrucao_limpar(&instrucao);
        goto falha;
    }
    if (ferror(arquivo))
        goto falha;
    *saida = query;
    return true;

falha:
    query_liberar(&query);
    return false;
}

const INSTRUCAO *query_instrucoes(const QUERY *query,
                                              size_t *quantidade) {
    if (quantidade != NULL)
        *quantidade = query != NULL ? query->quantidade : 0;
    return query != NULL ? query->instrucoes : NULL;
}

static bool comparar(long double atual, long double alvo, const char *operador) {
    if (strcmp(operador, "==") == 0) return atual == alvo;
    if (strcmp(operador, "!=") == 0) return atual != alvo;
    if (strcmp(operador, "<") == 0) return atual < alvo;
    if (strcmp(operador, "<=") == 0) return atual <= alvo;
    if (strcmp(operador, ">") == 0) return atual > alvo;
    return atual >= alvo;
}

bool query_item_aceito(const QUERY *query, const ITEM *item) {
    if (query == NULL || item == NULL)
        return false;
    for (size_t i = 0; i < query->quantidade; ++i) {
        const INSTRUCAO *instrucao = &query->instrucoes[i];
        if (instrucao->tipo != FILTER)
            continue;
        const CAMPO *campo = item_campo(item, instrucao->indice_campo);
        if (campo == NULL || campo->ausente)
            return false;
        if (campo->tipo == CAMPO_TEXTO) {
            int ordem = strcmp(campo->valor.texto,
                               instrucao->valor.valor.texto);
            if (!comparar(ordem, 0, instrucao->operador))
                return false;
        } else if (campo->tipo == CAMPO_INTEIRO) {
            if (!comparar((long double)campo->valor.inteiro,
                          (long double)instrucao->valor.valor.inteiro,
                          instrucao->operador))
                return false;
        } else if (campo->tipo == CAMPO_REAL) {
            if (!comparar((long double)campo->valor.real,
                          (long double)instrucao->valor.valor.real,
                          instrucao->operador))
                return false;
        } else if (!comparar(campo->valor.booleano ? 1 : 0,
                             instrucao->valor.valor.booleano ? 1 : 0,
                             instrucao->operador)) {
            return false;
        }
    }
    return true;
}
