#include <stdio.h>
#include <stdlib.h>
#include "lista.h"

struct lista_ {
    ITEM *itens[TAM_MAX];
    int inicio;
    int fim;
    int tamanho;
};

LISTA *lista_criar(void) {
    LISTA *lista = malloc(sizeof(LISTA));
    if (lista != NULL) {
        lista->inicio = 0;
        lista->fim = 0;
        lista->tamanho = 0;
        for (int i = 0; i < TAM_MAX; i++)
            lista->itens[i] = NULL;
    }
    return lista;
}

void lista_apagar(LISTA **lista) {
    if (lista == NULL || *lista == NULL)
        return;
    for (int i = 0; i < (*lista)->fim; i++)
        item_apagar(&(*lista)->itens[i]);
    free(*lista);
    *lista = NULL;
}

bool lista_inserir(LISTA *lista, ITEM *item) {
    if (lista == NULL || item == NULL || lista_cheia(lista))
        return false;
    lista->itens[lista->fim] = item;
    lista->fim++;
    lista->tamanho++;
    return true;
}

int lista_tamanho(const LISTA *lista) {
    return lista == NULL ? 0 : lista->tamanho;
}

bool lista_vazia(const LISTA *lista) {
    return lista == NULL || lista->tamanho == 0;
}

bool lista_cheia(const LISTA *lista) {
    return lista == NULL || lista->tamanho == TAM_MAX;
}

void lista_imprimir(const LISTA *lista) {
    if (lista == NULL)
        return;
    for (int i = lista->inicio; i < lista->fim; i++) {
        item_imprimir(lista->itens[i]);
        if (i + 1 < lista->fim)
            printf(" ");
    }
    printf("\n");
}
