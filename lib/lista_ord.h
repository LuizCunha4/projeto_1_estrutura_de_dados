#ifndef LISTA_ORD_H
#define LISTA_ORD_H

#include <stdbool.h>
#include "item.h"
#include "tad.h"

typedef struct lista_ord LISTA_ORD;
typedef int (*LISTA_ORD_COMPARADOR)(const ITEM *a, const ITEM *b,
                                    void *contexto);
/* Os campos usados na ordenação (ou pelo comparador) devem permanecer
 * estáveis enquanto os itens estiverem na lista. O contexto do comparador é
 * emprestado e deve sobreviver até a lista ser apagada ou reconfigurada. */

LISTA_ORD *lista_ord_criar(void);
void lista_ord_apagar(LISTA_ORD **lista);
bool lista_ord_inserir(LISTA_ORD *lista, ITEM *item);
ITEM *lista_ord_busca_item(const LISTA_ORD *lista, const ITEM *item);
ITEM *lista_ord_remover_item(LISTA_ORD *lista, ITEM *item);
bool lista_ord_ordenar_por(LISTA_ORD *lista, size_t campo, bool crescente);
bool lista_ord_ordenar_comparador(LISTA_ORD *lista,
                                  LISTA_ORD_COMPARADOR comparador,
                                  void *contexto);
int lista_ord_tamanho(const LISTA_ORD *lista);
bool lista_ord_vazia(const LISTA_ORD *lista);
bool lista_ord_cheia(const LISTA_ORD *lista);
void lista_ord_imprimir(const LISTA_ORD *lista);
void lista_ord_percorrer(const LISTA_ORD *lista, TAD_VISITANTE visitante,
                         void *contexto);

#endif
