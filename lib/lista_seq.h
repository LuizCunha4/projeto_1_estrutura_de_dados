#ifndef LISTA_SEQ_H
#define LISTA_SEQ_H

#include <stdbool.h>
#include "item.h"
#include "tad.h"

typedef struct lista_seq LISTA_SEQ;

LISTA_SEQ *lista_seq_criar(void);
void lista_seq_apagar(LISTA_SEQ **lista);
bool lista_seq_inserir(LISTA_SEQ *lista, ITEM *item);
int lista_seq_tamanho(const LISTA_SEQ *lista);
bool lista_seq_vazia(const LISTA_SEQ *lista);
bool lista_seq_cheia(const LISTA_SEQ *lista);
void lista_seq_imprimir(const LISTA_SEQ *lista);
void lista_seq_percorrer(const LISTA_SEQ *lista, TAD_VISITANTE visitante,
                         void *contexto);
ITEM *lista_seq_remover(LISTA_SEQ *lista, int posicao);
ITEM *lista_seq_remover_item(LISTA_SEQ *lista, ITEM *item);

#endif
