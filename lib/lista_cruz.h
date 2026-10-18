#ifndef LISTA_CRUZ_H
#define LISTA_CRUZ_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_cruz_ LISTA_CRUZ;

LISTA_CRUZ *lista_cruz_criar(int linhas, int colunas);
void lista_cruz_apagar(LISTA_CRUZ **lista);

bool lista_cruz_inserir(LISTA_CRUZ *lista, int linha, int coluna, ITEM *item);
ITEM *lista_cruz_buscar(const LISTA_CRUZ *lista, int linha, int coluna);
ITEM *lista_cruz_remover(LISTA_CRUZ *lista, int linha, int coluna);

void lista_cruz_imprimir_linha(const LISTA_CRUZ *lista, int linha);
void lista_cruz_imprimir_coluna(const LISTA_CRUZ *lista, int coluna);

#endif
