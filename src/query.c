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
    const ESQUEMA *esquema;
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

static char *aparar(char *s) {
    s = pular_espacos(s);
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    return s;
}

static size_t separar_pipe(char *texto, char **partes, size_t maximo) {
    size_t n = 0;
    char *inicio = texto;
    for (char *p = texto;; ++p) {
        if (*p == '|' || *p == '\0') {
            if (n == maximo) return 0;
            bool fim = *p == '\0';
            if (!fim) *p = '\0';
            partes[n++] = aparar(inicio);
            if (fim) break;
            inicio = p + 1;
        }
    }
    return n;
}

static bool filtro_partes_ler(char **p, size_t n, const ESQUEMA *esquema,
                               INSTRUCAO *instrucao) {
    if (n != 4 || *p[1] == '\0' || !operador_valido(p[2]) || *p[3] == '\0')
        return false;
    size_t indice;
    const COLUNA *coluna;
    if (!esquema_buscar(esquema, p[1], &indice) ||
        (coluna = esquema_coluna(esquema, indice)) == NULL)
        return false;
    if (coluna->tipo == CAMPO_BOOLEANO && strcmp(p[2], "==") != 0 &&
        strcmp(p[2], "=") != 0 && strcmp(p[2], "!=") != 0)
        return false;
    instrucao->indice_campo = indice;
    strcpy(instrucao->operador, strcmp(p[2], "=") == 0 ? "==" : p[2]);
    return valor_filtro_ler(coluna->tipo, p[3], instrucao);
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
    free(instrucao->indices_campos);
    *instrucao = (INSTRUCAO){0};
}

static bool numero_real(char *texto, double *valor) {
    char *fim;
    errno = 0;
    *valor = strtod(texto, &fim);
    return errno == 0 && fim != texto && *fim == '\0' && isfinite(*valor);
}

static bool campo_numerico(const ESQUEMA *esquema, const char *nome,
                           size_t *indice) {
    const COLUNA *coluna;
    return esquema_buscar(esquema, nome, indice) &&
           (coluna = esquema_coluna(esquema, *indice)) != NULL &&
           (coluna->tipo == CAMPO_INTEIRO || coluna->tipo == CAMPO_REAL);
}

static bool campos_partes_ler(char **p, size_t n, const ESQUEMA *esquema,
                              INSTRUCAO *instrucao, size_t minimo,
                              size_t maximo) {
    size_t quantidade = n - 1;
    if (quantidade < minimo || quantidade > maximo ||
        quantidade > SIZE_MAX / sizeof(*instrucao->indices_campos))
        return false;
    instrucao->indices_campos = malloc(quantidade * sizeof(*instrucao->indices_campos));
    if (instrucao->indices_campos == NULL) return false;
    for (size_t i = 0; i < quantidade; ++i) {
        if (*p[i + 1] == '\0' ||
            !esquema_buscar(esquema, p[i + 1], &instrucao->indices_campos[i]))
            return false;
        for (size_t j = 0; j < i; ++j)
            if (instrucao->indices_campos[j] == instrucao->indices_campos[i])
                return false;
    }
    instrucao->quantidade_campos = quantidade;
    return true;
}

static bool instrucao_partes_ler(INSTRUCAO *instrucao, char **p, size_t n,
                                  const ESQUEMA *esquema) {
    if (instrucao->tipo == SIM) {
        double alvo, tol, peso;
        if (n != 5 || !campo_numerico(esquema, p[1], &instrucao->indice_campo) ||
            !numero_real(p[2], &alvo) || !numero_real(p[3], &tol) || tol <= 0.0 ||
            !numero_real(p[4], &peso) || peso <= 0.0) return false;
        instrucao->valor.tipo = esquema_coluna(esquema, instrucao->indice_campo)->tipo;
        instrucao->valor.valor.real = alvo;
        instrucao->tolerancia = tol;
        instrucao->peso = peso;
        return true;
    }
    if (instrucao->tipo == ACCEPT)
        return n == 2 && numero_real(p[1], &instrucao->limiar) &&
               instrucao->limiar >= 0.0 && instrucao->limiar <= 1.0;
    if (instrucao->tipo == ORDER) {
        if (n != 3 || (strcmp(p[2], "ASC") != 0 && strcmp(p[2], "DESC") != 0))
            return false;
        instrucao->ascendente = strcmp(p[2], "ASC") == 0;
        if (strcmp(p[1], "similarity") == 0) {
            instrucao->ordenar_por_similaridade = true;
            return true;
        }
        return esquema_buscar(esquema, p[1], &instrucao->indice_campo);
    }
    if (instrucao->tipo == GROUP)
        return campos_partes_ler(p, n, esquema, instrucao, 2, SIZE_MAX);
    if (instrucao->tipo == CROSS)
        return campos_partes_ler(p, n, esquema, instrucao, 2, 2);
    return false;
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
    query->esquema = esquema;

    char linha[TAMANHO_LINHA_QUERY];
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        size_t tamanho = strlen(linha);
        if (tamanho == sizeof(linha) - 1 && linha[tamanho - 1] != '\n' &&
            !feof(arquivo))
            goto falha;
        while (tamanho > 0 && (linha[tamanho - 1] == '\n' ||
                               linha[tamanho - 1] == '\r'))
            linha[--tamanho] = '\0';

        char *linha_aparada = aparar(linha);
        if (*linha_aparada == '\0' || *linha_aparada == '#')
            continue;
        char *separador = strchr(linha_aparada, '|');
        if (separador == NULL)
            goto falha;
        char *argumentos_copia = copiar_texto(separador + 1);
        if (argumentos_copia == NULL)
            goto falha;
        char *partes[256];
        size_t n_partes = separar_pipe(linha_aparada, partes,
                                       sizeof(partes) / sizeof(partes[0]));

        INSTRUCAO instrucao = {.argumentos = argumentos_copia};
        if (n_partes < 2 || !tipo_instrucao_ler(partes[0], &instrucao.tipo))
            goto falha_instrucao;
        if (instrucao.tipo == FILTER) {
            if (!filtro_partes_ler(partes, n_partes, esquema, &instrucao))
                goto falha_instrucao;
        } else if (!instrucao_partes_ler(&instrucao, partes, n_partes, esquema)) {
            goto falha_instrucao;
        }
        if (instrucao.tipo != FILTER && instrucao.tipo != SIM) {
            for (size_t i = 0; i < query->quantidade; ++i)
                if (query->instrucoes[i].tipo == instrucao.tipo)
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

double query_item_similaridade(const QUERY *query, const ITEM *item) {
    if (query == NULL || item == NULL || item_esquema(item) != query->esquema)
        return 0.0;
    long double soma = 0.0L, pesos = 0.0L;
    for (size_t i = 0; i < query->quantidade; ++i) {
        const INSTRUCAO *instrucao = &query->instrucoes[i];
        if (instrucao->tipo != SIM) continue;
        const CAMPO *campo = item_campo(item, instrucao->indice_campo);
        long double valor = 0.0L;
        if (campo != NULL && !campo->ausente) {
            long double atual = campo->tipo == CAMPO_INTEIRO
                ? (long double)campo->valor.inteiro
                : (long double)campo->valor.real;
            long double distancia = fabsl(atual - instrucao->valor.valor.real);
            valor = 1.0L - distancia / instrucao->tolerancia;
            if (valor < 0.0L) valor = 0.0L;
        }
        soma += valor * instrucao->peso;
        pesos += instrucao->peso;
    }
    return pesos == 0.0L ? 1.0 : (double)(soma / pesos);
}

int query_item_comparar(const QUERY *query, const ITEM *a, const ITEM *b) {
    if (query == NULL || a == NULL || b == NULL ||
        item_esquema(a) != query->esquema || item_esquema(b) != query->esquema)
        return 0;
    const INSTRUCAO *ordem = NULL;
    for (size_t i = 0; i < query->quantidade; ++i)
        if (query->instrucoes[i].tipo == ORDER) ordem = &query->instrucoes[i];
    if (ordem == NULL) return 0;
    int comparacao = 0;
    if (ordem->ordenar_por_similaridade) {
        double sa = query_item_similaridade(query, a);
        double sb = query_item_similaridade(query, b);
        comparacao = (sa > sb) - (sa < sb);
    } else {
        const CAMPO *ca = item_campo(a, ordem->indice_campo);
        const CAMPO *cb = item_campo(b, ordem->indice_campo);
        if (ca == NULL || cb == NULL || ca->tipo != cb->tipo) return 0;
        if (ca->ausente != cb->ausente) return ca->ausente ? 1 : -1;
        if (ca->ausente) return 0;
        switch (ca->tipo) {
        case CAMPO_TEXTO:
            comparacao = (strcmp(ca->valor.texto, cb->valor.texto) > 0) -
                         (strcmp(ca->valor.texto, cb->valor.texto) < 0);
            break;
        case CAMPO_INTEIRO:
            comparacao = (ca->valor.inteiro > cb->valor.inteiro) -
                         (ca->valor.inteiro < cb->valor.inteiro);
            break;
        case CAMPO_REAL:
            comparacao = (ca->valor.real > cb->valor.real) -
                         (ca->valor.real < cb->valor.real);
            break;
        case CAMPO_BOOLEANO:
            comparacao = (ca->valor.booleano > cb->valor.booleano) -
                         (ca->valor.booleano < cb->valor.booleano);
            break;
        }
    }
    return ordem->ascendente ? comparacao : -comparacao;
}

int query_comparador_ordenacao(const ITEM *a, const ITEM *b, void *contexto) {
    return query_item_comparar((const QUERY *)contexto, a, b);
}

bool query_item_aceito(const QUERY *query, const ITEM *item) {
    if (query == NULL || item == NULL || item_esquema(item) != query->esquema)
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
    for (size_t i = 0; i < query->quantidade; ++i) {
        const INSTRUCAO *instrucao = &query->instrucoes[i];
        if (instrucao->tipo == ACCEPT &&
            query_item_similaridade(query, item) < instrucao->limiar)
            return false;
    }
    return true;
}
