#include <stdio.h>
#include <stdlib.h>
#include "lista_ord.h"

struct lista_ord {
    ITEM *itens[TAM_MAX];
    int inicio;
    int fim;
    int tamanho;
    bool ordenada;
};

static int lista_ord_lower_bound(const LISTA_ORD *lista, int chave) {
    int inf = 0;
    int sup = lista->fim;
    while (inf < sup) {
        int meio = inf + (sup - inf) / 2;
        if (item_get_chave(lista->itens[meio]) < chave)
            inf = meio + 1;
        else
            sup = meio;
    }
    return inf;
}

static int lista_ord_busca_binaria(const LISTA_ORD *lista, int chave) {
    int pos = lista_lower_bound(lista, chave);
    if (pos < lista->fim && item_get_chave(lista->itens[pos]) == chave)
        return pos;
    return -1;
}

static int lista_ord_busca_sequencial(const LISTA_ORD *lista, int chave) {
    for (int i = 0; i < lista->fim; i++) {
        if (item_get_chave(lista->itens[i]) == chave)
            return i;
    }
    return -1;
}

static int lista_ord_busca_posicao(const LISTA_ORD *lista, int chave) {
    return lista->ordenada ? lista_ord_busca_binaria(lista, chave)
                           : lista_ord_busca_sequencial(lista, chave);
}

static bool lista_ord_inserir_posicao(LISTA *lista, int pos, ITEM *item) {
    if (lista_cheia(lista) || pos < 0 || pos > lista->fim)
        return false;
    for (int i = lista->fim; i > pos; i--)
        lista->itens[i] = lista->itens[i - 1];
    lista->itens[pos] = item;
    lista->fim++;
    lista->tamanho++;
    return true;
}

LISTA_ORD *lista_ord_criar(bool ordenada) {
    LISTA_ORD *lista = malloc(sizeof(LISTA_ORD));
    if (lista != NULL) {
        lista->inicio = 0;
        lista->fim = 0;
        lista->tamanho = 0;
        lista->ordenada = ordenada;
        for (int i = 0; i < TAM_MAX; i++)
            lista->itens[i] = NULL;
    }
    return lista;
}

void lista_ord_apagar(LISTA_oORD **lista) {
    if (lista == NULL || *lista == NULL)
        return;
    for (int i = 0; i < (*lista)->fim; i++)
        item_apagar(&(*lista)->itens[i]);
    free(*lista);
    *lista = NULL;
}

bool lista_ord_inserir(LISTA_ORD *lista, ITEM *item) {
    if (lista == NULL || item == NULL || lista_ord_cheia(lista))
        return false;
    int pos = lista->ordenada ? lista_lower_bound(lista, item_get_chave(item)) : lista->fim;
    return lista_ord_inserir_posicao(lista, pos, item);
}

ITEM *lista_ord_busca(const LISTA_ORD *lista, int chave) {
    if (lista == NULL || lista_vazia(lista))
        return NULL;
    int pos = lista_ord_busca_posicao(lista, chave);
    return pos >= 0 ? lista->itens[pos] : NULL;
}

ITEM *lista_ord_remover(LISTA_ORD *lista, int chave) {
    if (lista == NULL || lista_ord_vazia(lista))
        return NULL;

    int pos = lista_ord_busca_posicao(lista, chave);
    if (pos < 0)
        return NULL;

    ITEM *item = lista->itens[pos];
    for (int i = pos; i < lista->fim - 1; i++)
        lista->itens[i] = lista->itens[i + 1];

    lista->fim--;
    lista->itens[lista->fim] = NULL;
    lista->tamanho--;
    return item;
}

int lista_ord_tamanho(const LISTA *lista) {
    return lista == NULL ? 0 : lista->tamanho;
}

bool lista_ord_vazia(const LISTA_ORD *lista) {
    return lista == NULL || lista->tamanho == 0;
}

bool lista_ord_cheia(const LISTA_ORD *lista) {
    return lista == NULL || lista->tamanho == TAM_MAX;
}

void lista_ord_imprimir(const LISTA_ORD *lista) {
    if (lista == NULL)
        return;
    for (int i = 0; i < lista->fim; i++) {
        item_imprimir(lista->itens[i]);
        if (i + 1 < lista->fim)
            printf(" ");
    }
    printf("\n");
}
