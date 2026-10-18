#ifndef LISTA_H
#define LISTA_H

#include <stdbool.h>
#include "item.h"

#define TAM_MAX 10

typedef struct lista_ LISTA;

LISTA *lista_criar(void);
void lista_apagar(LISTA **lista);
bool lista_inserir(LISTA *lista, ITEM *item);
int lista_tamanho(const LISTA *lista);
bool lista_vazia(const LISTA *lista);
bool lista_cheia(const LISTA *lista);
void lista_imprimir(const LISTA *lista);

#endif
