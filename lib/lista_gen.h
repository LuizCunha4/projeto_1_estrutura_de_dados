#ifndef LISTA_GEN_H
#define LISTA_GEN_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_gen_ LISTA_GEN;

LISTA_GEN *lista_gen_criar(void);
void lista_gen_apagar(LISTA_GEN **lista);

bool lista_gen_inserir_atomo(LISTA_GEN *lista, ITEM *item);
LISTA_GEN *lista_gen_inserir_sublista(LISTA_GEN *lista);

bool lista_gen_buscar(const LISTA_GEN *lista, int chave);
bool lista_gen_remover(LISTA_GEN *lista, int chave);

int lista_gen_profundidade(const LISTA_GEN *lista);
void lista_gen_imprimir(const LISTA_GEN *lista);

#endif
