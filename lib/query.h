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
    const char *argumentos; /* Cópia pertencente à query. */
    size_t indice_campo;    /* FILTER, SIM ou ORDER por campo. */
    char operador[3];       /* Usado somente por FILTER. */
    CAMPO valor;            /* FILTER e alvo numérico de SIM. */
    char *texto_filtro;     /* Propriedade da query, para valores textuais. */
    /* SIM: campo numérico, alvo, tolerância e peso. */
    double tolerancia;
    double peso;
    /* ACCEPT: limiar. ORDER: ascendente quando verdadeiro. */
    double limiar;
    bool ascendente;
    bool ordenar_por_similaridade;
    /* GROUP/CROSS: índices de campos, propriedade da query. */
    size_t *indices_campos;
    size_t quantidade_campos;
} INSTRUCAO;

typedef struct query QUERY;

/* O esquema deve permanecer vivo enquanto a QUERY existir. Gramática do
 * enunciado, com campos separados por '|':
 * FILTER|campo|operador|valor; SIM|campo|alvo|tolerância|peso;
 * ACCEPT|limiar; ORDER|campo|direção (ASC ou DESC);
 * ORDER|similarity|direção;
 * GROUP|campo|campo; CROSS|campo|campo. */
bool query_ler(FILE *arquivo, const ESQUEMA *esquema, QUERY **saida);
void query_liberar(QUERY **query);
/* Esquema emprestado; permanece válido enquanto a QUERY existir e deve ser
 * idêntico ao esquema dos itens avaliados/ordenados por ela. */
const ESQUEMA *query_esquema(const QUERY *query);

/* Retorna as instruções na ordem do arquivo; os argumentos são emprestados e
 * permanecem válidos até query_liberar. */
const INSTRUCAO *query_instrucoes(const QUERY *query,
                                              size_t *quantidade);

/* Similaridade ponderada dos critérios SIM, no intervalo [0,1]. Campos
 * ausentes contribuem com zero. Sem SIM, retorna 1. */
double query_item_similaridade(const QUERY *query, const ITEM *item);
/* Comparação segundo ORDER: retorna <0, 0 ou >0; ausentes ficam ao final.
 * Retorna zero se não houver ORDER ou se não for possível comparar. */
int query_item_comparar(const QUERY *query, const ITEM *a, const ITEM *b);
/* Adaptador compatível com LISTA_ORD_COMPARADOR; contexto deve ser QUERY *. */
int query_comparador_ordenacao(const ITEM *a, const ITEM *b, void *contexto);
/* Aplica FILTER e, se houver, o limiar ACCEPT à similaridade. */
bool query_item_aceito(const QUERY *query, const ITEM *item);

#endif
