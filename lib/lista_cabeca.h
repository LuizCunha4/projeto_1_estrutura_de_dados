#ifndef LISTA_CABECA_H
#define LISTA_CABECA_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_ LISTA;

LISTA *lista_criar(void);
void lista_apagar(LISTA **lista);
bool lista_inserir(LISTA *lista, ITEM *item);
ITEM *lista_buscar(LISTA *lista, int chave);
ITEM *lista_remover(LISTA *lista, int chave);
bool lista_vazia(const LISTA *lista);
int lista_tamanho(const LISTA *lista);
void lista_imprimir(const LISTA *lista);

#endif
