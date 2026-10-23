#ifndef LISTA_CABECA_H
#define LISTA_CABECA_H

#include <stdbool.h>
#include "item.h"
#include "tad.h"

typedef struct lista_cabeca LISTA_CABECA;

LISTA_CABECA *lista_cabeca_criar(void);
void lista_cabeca_apagar(LISTA_CABECA **lista);
bool lista_cabeca_inserir(LISTA_CABECA *lista, ITEM *item);
ITEM *lista_cabeca_buscar_item(const LISTA_CABECA *lista, const ITEM *item);
ITEM *lista_cabeca_remover(LISTA_CABECA *lista, ITEM *item);
bool lista_cabeca_vazia(const LISTA_CABECA *lista);
int lista_cabeca_tamanho(const LISTA_CABECA *lista);
void lista_cabeca_imprimir(const LISTA_CABECA *lista);
void lista_cabeca_percorrer(const LISTA_CABECA *lista,
                            TAD_VISITANTE visitante, void *contexto);

#endif
