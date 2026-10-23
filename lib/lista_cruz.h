#ifndef LISTA_CRUZ_H
#define LISTA_CRUZ_H

#include "tad.h"

typedef struct lista_cruz_ LISTA_CRUZ;
typedef enum { CRUZ_POR_LINHA, CRUZ_POR_COLUNA } LISTA_CRUZ_EIXO;

/* Cada item é indexado simultaneamente pelos valores dos dois campos. Os
 * itens devem compartilhar o mesmo esquema; os índices precisam existir e os
 * valores desses campos devem permanecer estáveis enquanto estiverem na lista. */
LISTA_CRUZ *lista_cruz_criar(size_t campo_linha, size_t campo_coluna);
void lista_cruz_apagar(LISTA_CRUZ **lista);
bool lista_cruz_inserir(LISTA_CRUZ *lista, ITEM *item);
ITEM *lista_cruz_buscar(const LISTA_CRUZ *lista, const ITEM *item);
ITEM *lista_cruz_remover(LISTA_CRUZ *lista, ITEM *item);
void lista_cruz_percorrer(const LISTA_CRUZ *lista, LISTA_CRUZ_EIXO eixo,
                          TAD_VISITANTE visitante, void *contexto);
size_t lista_cruz_grupos(const LISTA_CRUZ *lista, LISTA_CRUZ_EIXO eixo);

#endif
