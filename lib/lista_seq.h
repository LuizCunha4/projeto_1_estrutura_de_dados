#ifndef LISTA_SEQ_H
#define LISTA_SEQ_H

#include <stdbool.h>
#include "item.h"

#define TAM_MAX 10

typedef struct lista_seq LISTA_SEQ;

LISTA_SEQ *lista_seq_criar(void);
void lista_seq_apagar(LISTA_SEQ **lista);
bool lista_seq_inserir(LISTA_SEQ *lista, ITEM *item);
int lista_seq_tamanho(const LISTA_SEQ *lista);
bool lista_seq_vazia(const LISTA_SEQ *lista);
bool lista_seq_cheia(const LISTA_SEQ *lista);
void lista_seq_imprimir(const LISTA_SEQ *lista);

#endif
