#ifndef LISTA_CABECA_H
#define LISTA_CABECA_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_ LISTA;

LISTA_CABECA *lista_cabeca_criar(void);
void lista_cabeca_apagar(LISTA **lista);
bool lista_cabeca_inserir(LISTA *lista, ITEM *item);
ITEM *lista_cabeca_buscar(LISTA *lista, int chave);
ITEM *lista_cabeca_remover(LISTA *lista, int chave);
bool lista_cabeca_vazia(const LISTA *lista);
int lista_cabeca_tamanho(const LISTA *lista);
void lista_cabeca_imprimir(const LISTA *lista);

#endif
