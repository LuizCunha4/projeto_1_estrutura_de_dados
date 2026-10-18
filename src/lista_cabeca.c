#include "lista_cabeca.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;

struct no_ {
    ITEM *item;
    NO *proximo;
};

struct lista_ {
    NO *cabeca;   
    NO *fim;      
    int tamanho;
};

LISTA *lista_criar(void) {
    LISTA *lista = malloc(sizeof(LISTA));
    if (lista == NULL)
        return NULL;

    lista->cabeca = malloc(sizeof(NO));
    if (lista->cabeca == NULL) {
        free(lista);
        return NULL;
    }

    lista->cabeca->item = NULL;
    lista->cabeca->proximo = NULL;
    lista->fim = NULL;
    lista->tamanho = 0;
    return lista;
}

void lista_apagar(LISTA **lista) {
    if (lista == NULL || *lista == NULL)
        return;

    NO *p = (*lista)->cabeca->proximo;
    while (p != NULL) {
        NO *proximo = p->proximo;
        item_apagar(&p->item);
        free(p);
        p = proximo;
    }

    free((*lista)->cabeca);
    free(*lista);
    *lista = NULL;
}

bool lista_vazia(const LISTA *lista) {
    return lista != NULL && lista->cabeca->proximo == NULL;
}

int lista_tamanho(const LISTA *lista) {
    return lista != NULL ? lista->tamanho : 0;
}

bool lista_inserir(LISTA *lista, ITEM *item) {
    if (lista == NULL || item == NULL)
        return false;

    NO *novo = malloc(sizeof(NO));
    if (novo == NULL)
        return false;

    novo->item = item;
    novo->proximo = NULL;

    if (lista->fim == NULL)
        lista->cabeca->proximo = novo;
    else
        lista->fim->proximo = novo;

    lista->fim = novo;
    lista->tamanho++;
    return true;
}

ITEM *lista_buscar(LISTA *lista, int chave) {
    if (lista == NULL)
        return NULL;

    for (NO *p = lista->cabeca->proximo; p != NULL; p = p->proximo)
        if (item_chave(p->item) == chave)
            return p->item;

    return NULL;
}

ITEM *lista_remover(LISTA *lista, int chave) {
    if (lista == NULL)
        return NULL;

    /* Comecar no no-cabeca garante que p sempre seja o antecessor. */
    NO *p = lista->cabeca;
    while (p->proximo != NULL && item_chave(p->proximo->item) != chave)
        p = p->proximo;

    if (p->proximo == NULL)
        return NULL;

    NO *removido = p->proximo;
    ITEM *item = removido->item;
    p->proximo = removido->proximo;

    if (removido == lista->fim)
        lista->fim = (p == lista->cabeca) ? NULL : p;

    free(removido);
    lista->tamanho--;
    return item;
}

void lista_imprimir(const LISTA *lista) {
    if (lista == NULL) {
        puts("(lista inexistente)");
        return;
    }

    printf("cabeca -> ");
    const NO *p = lista->cabeca->proximo;
    while (p != NULL) {
        item_imprimir(p->item);
        printf(" -> ");
        p = p->proximo;
    }
    printf("NULL  [tamanho=%d]\n", lista->tamanho);
}
