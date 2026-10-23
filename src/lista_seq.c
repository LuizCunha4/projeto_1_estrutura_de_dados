#include "lista_seq.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CAPACIDADE_INICIAL 16

struct lista_seq {
    ITEM **itens;
    size_t capacidade;
    size_t tamanho;
};

LISTA_SEQ *lista_seq_criar(void) {
    LISTA_SEQ *lista = calloc(1, sizeof(*lista));
    if (lista == NULL) return NULL;
    lista->capacidade = CAPACIDADE_INICIAL;
    lista->itens = malloc(lista->capacidade * sizeof(*lista->itens));
    if (lista->itens == NULL) { free(lista); return NULL; }
    return lista;
}

void lista_seq_apagar(LISTA_SEQ **lista) {
    if (lista == NULL || *lista == NULL) return;
    free((*lista)->itens);
    free(*lista);
    *lista = NULL;
}

bool lista_seq_inserir(LISTA_SEQ *lista, ITEM *item) {
    if (lista == NULL || item == NULL) return false;
    if (lista->tamanho == lista->capacidade) {
        if (lista->capacidade > SIZE_MAX / 2 / sizeof(*lista->itens))
            return false;
        size_t nova_capacidade = lista->capacidade * 2;
        ITEM **novos = realloc(lista->itens,
                               nova_capacidade * sizeof(*novos));
        if (novos == NULL) return false;
        lista->itens = novos;
        lista->capacidade = nova_capacidade;
    }
    lista->itens[lista->tamanho++] = item;
    return true;
}

int lista_seq_tamanho(const LISTA_SEQ *lista) {
    return lista == NULL ? 0 : (int)lista->tamanho;
}

bool lista_seq_vazia(const LISTA_SEQ *lista) {
    return lista == NULL || lista->tamanho == 0;
}

bool lista_seq_cheia(const LISTA_SEQ *lista) { return lista == NULL; }

void lista_seq_imprimir(const LISTA_SEQ *lista) {
    if (lista == NULL) return;
    for (size_t i = 0; i < lista->tamanho; ++i) {
        const char *s = item_original(lista->itens[i]);
        puts(s != NULL ? s : "<item>");
    }
}

void lista_seq_percorrer(const LISTA_SEQ *lista, TAD_VISITANTE visitante,
                         void *contexto) {
    if (lista == NULL || visitante == NULL) return;
    for (size_t i = 0; i < lista->tamanho; ++i)
        visitante(lista->itens[i], contexto);
}

ITEM *lista_seq_remover(LISTA_SEQ *lista, int posicao) {
    if (lista == NULL || posicao < 0 || (size_t)posicao >= lista->tamanho)
        return NULL;
    ITEM *item = lista->itens[posicao];
    for (size_t i = (size_t)posicao; i + 1 < lista->tamanho; ++i)
        lista->itens[i] = lista->itens[i + 1];
    lista->itens[--lista->tamanho] = NULL;
    return item;
}

ITEM *lista_seq_remover_item(LISTA_SEQ *lista, ITEM *item) {
    if (lista == NULL || item == NULL) return NULL;
    for (size_t i=0;i<lista->tamanho;i++)
        if(lista->itens[i]==item)return lista_seq_remover(lista,(int)i);
    return NULL;
}
