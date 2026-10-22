#include "cliente.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static bool copiar_opcao(char *destino, size_t capacidade, const char *origem) {
    size_t n = strlen(origem);
    if (n >= capacidade)
        return false;
    memcpy(destino, origem, n + 1);
    return true;
}

Opcoes parse_args(int argc, char *argv[]) {
    Opcoes opcoes = {0};
    for (int i = 1; i < argc; ++i) {
        char *valor;
        char *destino = NULL;
        size_t capacidade = 0;
        if (i + 1 >= argc)
            break;
        if (strcmp(argv[i], "--reader") == 0) {
            destino = opcoes.reader; capacidade = sizeof(opcoes.reader);
        } else if (strcmp(argv[i], "--input") == 0) {
            destino = opcoes.input; capacidade = sizeof(opcoes.input);
        } else if (strcmp(argv[i], "--base") == 0) {
            destino = opcoes.base; capacidade = sizeof(opcoes.base);
        } else if (strcmp(argv[i], "--result") == 0) {
            destino = opcoes.result; capacidade = sizeof(opcoes.result);
        } else if (strcmp(argv[i], "--query") == 0) {
            destino = opcoes.query; capacidade = sizeof(opcoes.query);
        } else if (strcmp(argv[i], "--output") == 0) {
            destino = opcoes.output; capacidade = sizeof(opcoes.output);
        } else if (strcmp(argv[i], "--top-k") == 0 ||
                   strcmp(argv[i], "--limit") == 0) {
            char *fim;
            errno = 0;
            long numero = strtol(argv[++i], &fim, 10);
            if (errno == 0 && *argv[i] != '\0' && *fim == '\0' &&
                numero >= 0 && numero <= 2147483647L) {
                if (strcmp(argv[i - 1], "--top-k") == 0)
                    opcoes.top_k = (int)numero;
                else
                    opcoes.limit = (int)numero;
            }
            continue;
        } else {
            continue;
        }
        valor = argv[++i];
        (void)copiar_opcao(destino, capacidade, valor);
    }
    return opcoes;
}

void filtros_apagar(CONSULTA_FILTROS *filtros) {
    if (filtros == NULL)
        return;
    for (size_t i = 0; i < filtros->quantidade; ++i)
        free(filtros->itens[i].texto);
    free(filtros->itens);
    *filtros = (CONSULTA_FILTROS){0};
}

static char *pular_espacos(char *p) {
    while (isspace((unsigned char)*p))
        ++p;
    return p;
}

static bool ler_valor(TIPO_CAMPO tipo, char *entrada, FILTRO *filtro) {
    char *fim;
    errno = 0;
    filtro->valor.tipo = tipo;
    filtro->valor.ausente = false;
    switch (tipo) {
    case CAMPO_TEXTO:
        if (*entrada == '"') {
            size_t n = strlen(entrada);
            if (n < 2 || entrada[n - 1] != '"')
                return false;
            entrada[n - 1] = '\0';
            ++entrada;
        }
        filtro->texto = malloc(strlen(entrada) + 1);
        if (filtro->texto == NULL)
            return false;
        strcpy(filtro->texto, entrada);
        filtro->valor.valor.texto = filtro->texto;
        return true;
    case CAMPO_INTEIRO:
        filtro->valor.valor.inteiro = strtoll(entrada, &fim, 10);
        if (errno != 0 || fim == entrada || *fim != '\0')
            return false;
        return true;
    case CAMPO_REAL:
        filtro->valor.valor.real = strtod(entrada, &fim);
        if (errno != 0 || fim == entrada || *fim != '\0' ||
            !isfinite(filtro->valor.valor.real))
            return false;
        return true;
    case CAMPO_BOOLEANO:
        if (strcmp(entrada, "true") == 0 || strcmp(entrada, "True") == 0)
            filtro->valor.valor.booleano = true;
        else if (strcmp(entrada, "false") == 0 || strcmp(entrada, "False") == 0)
            filtro->valor.valor.booleano = false;
        else
            return false;
        return true;
    }
    return false;
}

static bool operador_valido(const char *op) {
    return strcmp(op, "==") == 0 || strcmp(op, "=") == 0 ||
           strcmp(op, "!=") == 0 || strcmp(op, "<") == 0 ||
           strcmp(op, "<=") == 0 || strcmp(op, ">") == 0 ||
           strcmp(op, ">=") == 0;
}

bool filtros_ler(FILE *arquivo, const ESQUEMA *esquema,
                 CONSULTA_FILTROS *filtros) {
    if (arquivo == NULL || esquema == NULL || filtros == NULL)
        return false;
    *filtros = (CONSULTA_FILTROS){0};
    char linha[1024];
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        char *p = pular_espacos(linha);
        if (*p == '\0' || *p == '\n' || *p == '#')
            continue;
        char *comando = strtok(p, " \t\r\n");
        char *nome = strtok(NULL, " \t\r\n");
        char *op = strtok(NULL, " \t\r\n");
        char *valor = strtok(NULL, "\r\n");
        if (comando == NULL || strcmp(comando, "FILTER") != 0 ||
            nome == NULL || op == NULL || valor == NULL ||
            !operador_valido(op))
            goto falha;
        valor = pular_espacos(valor);
        size_t nvalor = strlen(valor);
        while (nvalor > 0 && isspace((unsigned char)valor[nvalor - 1]))
            valor[--nvalor] = '\0';
        if (*valor == '\0')
            goto falha;

        size_t indice;
        const COLUNA *coluna;
        if (!esquema_buscar(esquema, nome, &indice) ||
            (coluna = esquema_coluna(esquema, indice)) == NULL)
            goto falha;
        if (coluna->tipo == CAMPO_BOOLEANO &&
            strcmp(op, "==") != 0 && strcmp(op, "=") != 0 && strcmp(op, "!=") != 0)
            goto falha;

        FILTRO novo = {0};
        novo.indice = indice;
        strcpy(novo.operador, strcmp(op, "=") == 0 ? "==" : op);
        if (!ler_valor(coluna->tipo, valor, &novo)) {
            free(novo.texto);
            goto falha;
        }
        if (filtros->quantidade == CLIENTE_MAX_FILTROS) {
            free(novo.texto);
            goto falha;
        }
        FILTRO *itens = realloc(filtros->itens,
            (filtros->quantidade + 1) * sizeof(*itens));
        if (itens == NULL) {
            free(novo.texto);
            goto falha;
        }
        filtros->itens = itens;
        filtros->itens[filtros->quantidade++] = novo;
    }
    if (ferror(arquivo))
        goto falha;
    return true;

falha:
    filtros_apagar(filtros);
    return false;
}

static bool comparar(long double atual, long double alvo, const char *op) {
    if (strcmp(op, "==") == 0) return atual == alvo;
    if (strcmp(op, "!=") == 0) return atual != alvo;
    if (strcmp(op, "<") == 0) return atual < alvo;
    if (strcmp(op, "<=") == 0) return atual <= alvo;
    if (strcmp(op, ">") == 0) return atual > alvo;
    return atual >= alvo;
}

bool cliente_item_aceito(const ITEM *item, const CONSULTA_FILTROS *filtros) {
    if (item == NULL || filtros == NULL)
        return false;
    for (size_t i = 0; i < filtros->quantidade; ++i) {
        const FILTRO *f = &filtros->itens[i];
        const CAMPO *campo = item_campo(item, f->indice);
        if (campo == NULL || campo->ausente)
            return false;
        if (campo->tipo == CAMPO_TEXTO) {
            int ordem = strcmp(campo->valor.texto, f->valor.valor.texto);
            if (!comparar(ordem, 0, f->operador))
                return false;
        } else if (campo->tipo == CAMPO_INTEIRO) {
            if (!comparar((long double)campo->valor.inteiro,
                          (long double)f->valor.valor.inteiro, f->operador))
                return false;
        } else if (campo->tipo == CAMPO_REAL) {
            if (!comparar((long double)campo->valor.real,
                          (long double)f->valor.valor.real, f->operador))
                return false;
        } else if (!comparar(campo->valor.booleano ? 1 : 0,
                             f->valor.valor.booleano ? 1 : 0, f->operador)) {
            return false;
        }
    }
    return true;
}
