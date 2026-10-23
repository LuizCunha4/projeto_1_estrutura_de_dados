#ifndef LISTA_GEN_H
#define LISTA_GEN_H

#include <stdbool.h>
#include "item.h"
#include "tad.h"

typedef struct lista_gen_ LISTA_GEN;

LISTA_GEN *lista_gen_criar(void);
void lista_gen_apagar(LISTA_GEN **lista);
bool lista_gen_configurar_grupos(LISTA_GEN *lista, const size_t *campos,
                                 size_t quantidade);

bool lista_gen_inserir_atomo(LISTA_GEN *lista, ITEM *item);
LISTA_GEN *lista_gen_inserir_sublista(LISTA_GEN *lista);

bool lista_gen_contem_item(const LISTA_GEN *lista, const ITEM *item);
ITEM *lista_gen_remover_item(LISTA_GEN *lista, ITEM *item);

int lista_gen_profundidade(const LISTA_GEN *lista);
void lista_gen_imprimir(const LISTA_GEN *lista);
void lista_gen_percorrer(const LISTA_GEN *lista, TAD_VISITANTE visitante,
                         void *contexto);

#endif
