#ifndef CLIENTE_H
#define CLIENTE_H

#include <stdbool.h>
#include <stdio.h>
#include "item.h"

#define CLIENTE_MAX_FILTROS 64

typedef struct {
    size_t indice;
    char operador[3];
    CAMPO valor;
    char *texto;
} FILTRO;

typedef struct {
    FILTRO *itens;
    size_t quantidade;
} CONSULTA_FILTROS;

typedef struct {
    char reader[32];
    char input[256];
    char base[32];
    char result[32];
    char query[256];
    char output[256];
    int top_k;
    int limit;
} Opcoes;

Opcoes parse_args(int argc, char *argv[]);
/* Formato aceito: uma condicao por linha: FILTER campo operador valor.
 * Operadores: ==, !=, <, <=, > e >=. Linhas vazias e comentarios # sao ignorados.
 * Retorna false para campo, tipo, operador ou valor invalido. */
bool filtros_ler(FILE *arquivo, const ESQUEMA *esquema,
                 CONSULTA_FILTROS *filtros);
bool cliente_item_aceito(const ITEM *item, const CONSULTA_FILTROS *filtros);
void filtros_apagar(CONSULTA_FILTROS *filtros);

#endif
