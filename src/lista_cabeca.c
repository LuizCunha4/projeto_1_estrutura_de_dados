#include "lista_cabeca.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;

struct no_ {
    ITEM *item;
    NO *proximo;
};

struct lista_cabeca {
    NO *cabeca;   
    NO *fim;      
    int tamanho;
};

LISTA_CABECA *lista_cabeca_criar(void) {
    LISTA_CABECA *lista = malloc(sizeof(LISTA_CABECA));
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

void lista_cabeca_apagar(LISTA_CABECA **lista) {
    if (lista == NULL || *lista == NULL)
        return;

    NO *p = (*lista)->cabeca->proximo;
    while (p != NULL) {
        NO *proximo = p->proximo;
        free(p);
        p = proximo;
    }

    free((*lista)->cabeca);
    free(*lista);
    *lista = NULL;
}

bool lista_cabeca_vazia(const LISTA_CABECA *lista) {
    return lista != NULL && lista->cabeca->proximo == NULL;
}

int lista_cabeca_tamanho(const LISTA_CABECA *lista) {
    return lista != NULL ? lista->tamanho : 0;
}

bool lista_cabeca_inserir(LISTA_CABECA *lista, ITEM *item) {
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

ITEM *lista_cabeca_buscar_item(const LISTA_CABECA *lista, const ITEM *item) {
    if (lista == NULL || item == NULL)
        return NULL;

    for (const NO *p = lista->cabeca->proximo; p != NULL; p = p->proximo)
        if (p->item == item) return p->item;

    return NULL;
}

ITEM *lista_cabeca_remover(LISTA_CABECA *lista, ITEM *item) {
    if (lista == NULL || item == NULL)
        return NULL;

    NO *p = lista->cabeca;
    while (p->proximo != NULL && p->proximo->item != item)
        p = p->proximo;

    if (p->proximo == NULL)
        return NULL;

    NO *removido = p->proximo;
    ITEM *retirado = removido->item;
    p->proximo = removido->proximo;

    if (removido == lista->fim)
        lista->fim = (p == lista->cabeca) ? NULL : p;

    free(removido);
    lista->tamanho--;
    return retirado;
}

void lista_cabeca_imprimir(const LISTA_CABECA *lista) {
    if (lista == NULL) {
        puts("(lista inexistente)");
        return;
    }

    printf("cabeca -> ");
    const NO *p = lista->cabeca->proximo;
    while (p != NULL) {
        const char *s = item_original(p->item);
        printf("%s", s != NULL ? s : "<item>");
        printf(" -> ");
        p = p->proximo;
    }
    printf("NULL  [tamanho=%d]\n", lista->tamanho);
}

void lista_cabeca_percorrer(const LISTA_CABECA *lista,
                            TAD_VISITANTE visitante, void *contexto) {
    if (lista == NULL || visitante == NULL) return;
    for (const NO *p=lista->cabeca->proximo; p!=NULL; p=p->proximo)
        visitante(p->item, contexto);
}
