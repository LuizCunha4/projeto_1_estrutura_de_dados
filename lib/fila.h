#ifndef FILA_H
#define FILA_H

#include <stdbool.h>
#include "item.h"
#include "tad.h"

typedef struct fila_ FILA;

FILA *fila_criar(void);
bool fila_inserir(FILA *fila, ITEM *item);
ITEM *fila_remover(FILA *fila);
ITEM *fila_remover_item(FILA *fila, ITEM *item);
void fila_apagar(FILA **fila);

ITEM *fila_frente(FILA *fila);
int fila_tamanho(FILA *fila);
bool fila_vazia(FILA *fila);
bool fila_cheia(FILA *fila);
void fila_imprimir(FILA *fila);
void fila_percorrer(const FILA *fila, TAD_VISITANTE visitante,
                    void *contexto);

#endif
