#ifndef ESTRUTURA_H
#define ESTRUTURA_H

#include "query.h"
#include "tad.h"
#include "lista_cruz.h"

typedef struct estrutura ESTRUTURA;
typedef enum { ESTRUTURA_BASE, ESTRUTURA_RESULTADOS } ESTRUTURA_PAPEL;

/* Fachada comum para os oito TADs. Em ESTRUTURA_BASE, inserção bem-sucedida
 * transfere a propriedade lógica do ITEM para a estrutura; em RESULTADOS,
 * ela apenas armazena uma referência. A destruição libera os registros da
 * base e somente os nós da estrutura de resultados. Remover da base transfere
 * o ITEM ao chamador; remover de resultados retorna uma referência emprestada.
 * Se a estrutura for lista_ord com ORDER, manter QUERY viva até destruí-la. */
ESTRUTURA *estrutura_criar(const char *nome, ESTRUTURA_PAPEL papel,
                           const QUERY *query);
bool estrutura_inserir(ESTRUTURA *estrutura, ITEM *item);
void estrutura_percorrer(const ESTRUTURA *estrutura,
                         TAD_VISITANTE visitante, void *contexto);
void estrutura_percorrer_eixo(const ESTRUTURA *estrutura,
                              LISTA_CRUZ_EIXO eixo,
                              TAD_VISITANTE visitante, void *contexto);
ITEM *estrutura_remover(ESTRUTURA *estrutura, ITEM *item);
size_t estrutura_tamanho(const ESTRUTURA *estrutura);
void estrutura_apagar(ESTRUTURA **estrutura);

#endif
