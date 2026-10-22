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
} INSTRUCAO;

typedef struct query QUERY;

/* Lê uma instrução por linha. FILTER é validado contra o esquema e convertido
 * em valor tipado. SIM, ACCEPT, ORDER, GROUP e CROSS são reconhecidos e seus
 * argumentos são preservados para processamento posterior. Linhas vazias e
 * comentários iniciados por # são ignorados. Inicialize *saida com NULL; em
 * erro, ela permanece NULL. */
bool query_ler(FILE *arquivo, const ESQUEMA *esquema, QUERY **saida);
void query_liberar(QUERY **query);

/* Retorna as instruções na ordem do arquivo; os argumentos são emprestados e
 * permanecem válidos até query_liberar. */
const INSTRUCAO *query_instrucoes(const QUERY *query,
                                              size_t *quantidade);

/* Aplica todas as instruções FILTER como conjunção. Sem FILTER, aceita o item.
 * As demais instruções são armazenadas, mas ainda não alteram esta decisão. */
bool query_item_aceito(const QUERY *query, const ITEM *item);

#endif
