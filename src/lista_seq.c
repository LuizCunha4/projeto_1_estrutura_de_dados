#include <stdio.h>
#include <stdlib.h>
#include "lista_seq.h"

struct lista_ seq{
    ITEM *itens[TAM_MAX];
    int inicio;
    int fim;
    int tamanho;
};

LISTA_SEQ *lista_seq_criar(void) {
    LISTA_SEQ *lista = malloc(sizeof(LISTA_SEQ));
    if (lista != NULL) {
        lista->inicio = 0;
        lista->fim = 0;
        lista->tamanho = 0;
        for (int i = 0; i < TAM_MAX; i++)
            lista->itens[i] = NULL;
    }
    return lista;
}

void lista_seq_apagar(LISTA_SEQ **lista) {
    if (lista == NULL || *lista == NULL)
        return;
    for (int i = 0; i < (*lista)->fim; i++)
        item_apagar(&(*lista)->itens[i]);
    free(*lista);
    *lista = NULL;
}

bool lista_seq_inserir(LISTA_SEQ *lista, ITEM *item) {
    if (lista == NULL || item == NULL || lista_cheia(lista))
        return false;
    lista->itens[lista->fim] = item;
    lista->fim++;
    lista->tamanho++;
    return true;
}

int lista_seq_tamanho(const LISTA_SEQ *lista) {
    return lista == NULL ? 0 : lista->tamanho;
}

bool lista_seq_vazia(const LISTA_SEQ *lista) {
    return lista == NULL || lista->tamanho == 0;
}

bool lista_seq_cheia(const LISTA_SEQ *lista) {
    return lista == NULL || lista->tamanho == TAM_MAX;
}

void lista_seq_imprimir(const LISTA_SEQ *lista) {
    if (lista == NULL)
        return;
    for (int i = lista->inicio; i < lista->fim; i++) {
        item_imprimir(lista->itens[i]);
        if (i + 1 < lista->fim)
            printf(" ");
    }
    printf("\n");
}
