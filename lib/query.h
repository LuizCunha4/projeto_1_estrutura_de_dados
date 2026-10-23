#ifndef QUERY_H
#define QUERY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "item.h"

typedef enum {
    FILTER,
    SIM,
    ACCEPT,
    ORDER,
    GROUP,
    CROSS
} TIPO_INSTRUCAO;

typedef struct {
    TIPO_INSTRUCAO tipo;
    const char *argumentos; /* Texto emprestado da query. */
    size_t indice_campo;    /* Usado somente por FILTER. */
    char operador[3];       /* Usado somente por FILTER. */
    CAMPO valor;            /* Usado somente por FILTER. */
    char *texto_filtro;     /* Propriedade da query, para valores textuais. */
    /* SIM: campo numérico, alvo, tolerância e peso. */
    double tolerancia;
    double peso;
    /* ACCEPT: limiar. ORDER: ascendente quando verdadeiro. */
    double limiar;
    bool ascendente;
    /* GROUP/CROSS: índices de campos, propriedade da query. */
    size_t *indices_campos;
    size_t quantidade_campos;
} INSTRUCAO;

typedef struct query QUERY;

/* Gramática: SIM campo alvo tolerância [peso]; ACCEPT limiar;
 * ORDER campo [ASC|DESC]; GROUP campo [campo ...]; CROSS campo campo.
 * FILTER campo operador valor. Campos são nomes exatos do esquema. */
bool query_ler(FILE *arquivo, const ESQUEMA *esquema, QUERY **saida);
void query_liberar(QUERY **query);

/* Retorna as instruções na ordem do arquivo; os argumentos são emprestados e
 * permanecem válidos até query_liberar. */
const INSTRUCAO *query_instrucoes(const QUERY *query,
                                              size_t *quantidade);

/* Similaridade ponderada dos critérios SIM, no intervalo [0,1]. Campos
 * ausentes contribuem com zero. Sem SIM, retorna 1. */
double query_item_similaridade(const QUERY *query, const ITEM *item);
/* Aplica FILTER e, se houver, o limiar ACCEPT à similaridade. */
bool query_item_aceito(const QUERY *query, const ITEM *item);

#endif
