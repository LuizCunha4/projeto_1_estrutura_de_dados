#include "lista_ord.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CAPACIDADE_INICIAL 16

struct lista_ord {
    ITEM **itens;
    size_t tamanho, capacidade, campo;
    bool ordenada, crescente;
    LISTA_ORD_COMPARADOR comparador;
    void *contexto_comparador;
};

LISTA_ORD *lista_ord_criar(void) {
    LISTA_ORD *l = calloc(1, sizeof(*l));
    if (l == NULL) return NULL;
    l->capacidade = CAPACIDADE_INICIAL;
    l->itens = malloc(l->capacidade * sizeof(*l->itens));
    if (l->itens == NULL) { free(l); return NULL; }
    l->ordenada = true;
    l->crescente = true;
    return l;
}

void lista_ord_apagar(LISTA_ORD **lista) {
    if (lista == NULL || *lista == NULL) return;
    free((*lista)->itens);
    free(*lista);
    *lista = NULL;
}

bool lista_ord_ordenar_por(LISTA_ORD *l, size_t campo, bool crescente) {
    if (l == NULL || l->tamanho != 0) return false;
    l->campo = campo;
    l->comparador = NULL;
    l->contexto_comparador = NULL;
    l->crescente = crescente;
    l->ordenada = true;
    return true;
}

bool lista_ord_ordenar_comparador(LISTA_ORD *l,
                                  LISTA_ORD_COMPARADOR comparador,
                                  void *contexto) {
    if (l == NULL || comparador == NULL || l->tamanho != 0) return false;
    l->comparador = comparador;
    l->contexto_comparador = contexto;
    l->ordenada = true;
    return true;
}

static int comparar(const LISTA_ORD *l, const ITEM *a, const ITEM *b) {
    if (l->comparador != NULL)
        return l->comparador(a, b, l->contexto_comparador);
    int c = item_comparar_campo(a, b, l->campo);
    return l->crescente ? c : -c;
}

bool lista_ord_inserir(LISTA_ORD *l, ITEM *item) {
    if (l == NULL || item == NULL) return false;
    if (l->tamanho == l->capacidade) {
        if (l->capacidade > SIZE_MAX / 2 / sizeof(*l->itens)) return false;
        size_t cap = l->capacidade * 2;
        ITEM **p = realloc(l->itens, cap * sizeof(*p));
        if (p == NULL) return false;
        l->itens = p; l->capacidade = cap;
    }
    size_t pos = l->tamanho;
    if (l->ordenada) {
        /* Inserção depois de chaves iguais preserva a ordem de chegada. */
        size_t lo = 0, hi = l->tamanho;
        while (lo < hi) {
            size_t mid = lo + (hi - lo) / 2;
            if (comparar(l, l->itens[mid], item) <= 0) lo = mid + 1;
            else hi = mid;
        }
        pos = lo;
        for (size_t i = l->tamanho; i > pos; --i) l->itens[i] = l->itens[i - 1];
    }
    l->itens[pos] = item;
    ++l->tamanho;
    return true;
}

ITEM *lista_ord_busca_item(const LISTA_ORD *l, const ITEM *item) {
    if (l == NULL || item == NULL) return NULL;
    for (size_t i = 0; i < l->tamanho; ++i)
        if (l->itens[i] == item) return l->itens[i];
    return NULL;
}

ITEM *lista_ord_remover_item(LISTA_ORD *l, ITEM *item) {
    if (l == NULL || item == NULL) return NULL;
    for (size_t i = 0; i < l->tamanho; ++i) {
        if (l->itens[i] != item) continue;
        for (size_t j = i; j + 1 < l->tamanho; ++j) l->itens[j] = l->itens[j + 1];
        l->itens[--l->tamanho] = NULL;
        return item;
    }
    return NULL;
}

int lista_ord_tamanho(const LISTA_ORD *l) { return l == NULL ? 0 : (int)l->tamanho; }
bool lista_ord_vazia(const LISTA_ORD *l) { return l == NULL || l->tamanho == 0; }
bool lista_ord_cheia(const LISTA_ORD *l) { return l == NULL; }

void lista_ord_imprimir(const LISTA_ORD *l) {
    if (l == NULL) return;
    for (size_t i = 0; i < l->tamanho; ++i) {
        const char *s = item_original(l->itens[i]);
        puts(s != NULL ? s : "<item>");
    }
}

void lista_ord_percorrer(const LISTA_ORD *l, TAD_VISITANTE visitante, void *ctx) {
    if (l == NULL || visitante == NULL) return;
    for (size_t i = 0; i < l->tamanho; ++i) visitante(l->itens[i], ctx);
}
