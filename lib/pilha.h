#ifndef PILHA_H
#define PILHA_H

#include "item.h"
#include "tad.h"

#include <stdbool.h>

typedef struct pilha PILHA;

PILHA *pilha_criar(void);
void pilha_apagar(PILHA **pilha);
bool pilha_vazia(PILHA *pilha);
bool pilha_cheia(PILHA *pilha);
int pilha_tamanho(PILHA *pilha);
ITEM *pilha_topo(PILHA *pilha);
bool pilha_empilhar(PILHA *pilha, ITEM *item);
ITEM *pilha_desempilhar(PILHA *pilha);
ITEM *pilha_remover_item(PILHA *pilha, ITEM *item);
void pilha_imprimir(PILHA *pilha);
void pilha_inverter(PILHA *pilha);
void pilha_percorrer(const PILHA *pilha, TAD_VISITANTE visitante,
                     void *contexto);

#endif
