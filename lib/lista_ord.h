#ifndef LISTA_ORD_H
#define LISTA_ORD_H

#include <stdbool.h>
#include "item.h"

#define TAM_MAX 10

typedef struct lista_ord LISTA_ORD;

LISTA_ORD *lista_ord_criar(bool ordenada);
void lista_ord_apagar(LISTA_ORD **lista);
bool lista_ord_inserir(LISTA_ORD *lista, ITEM *item);
ITEM *lista_ord_busca(const LISTA_ORD *lista, int chave);
ITEM *lista_ord_remover(LISTA_ORD *lista, int chave);
int lista_ord_tamanho(const LISTA_ORD *lista);
bool lista_ord_vazia(const LISTA_ORD *lista);
bool lista_ord_cheia(const LISTA_ORD *lista);
void lista_ord_imprimir(const LISTA_ORD *lista);

#endif
