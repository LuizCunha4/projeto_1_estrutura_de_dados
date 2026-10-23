#include "lista_enc.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;
struct no_ { ITEM *item; NO *anterior, *proximo; };
struct lista_enc { NO *inicio, *fim; int tamanho; bool ordenada, crescente; size_t campo; };

static NO *no_criar(ITEM *item) {
    NO *n = malloc(sizeof(*n));
    if (n != NULL) *n = (NO){.item=item};
    return n;
}

LISTA_ENC *lista_enc_criar(bool ordenada) {
    LISTA_ENC *l = calloc(1, sizeof(*l));
    if (l != NULL) { l->ordenada = ordenada; l->crescente = true; }
    return l;
}

void lista_enc_apagar(LISTA_ENC **lista) {
    if (lista == NULL || *lista == NULL) return;
    NO *n = (*lista)->inicio;
    while (n != NULL) { NO *prox = n->proximo; free(n); n = prox; }
    free(*lista); *lista = NULL;
}

bool lista_enc_ordenar_por(LISTA_ENC *l, size_t campo, bool crescente) {
    if (l == NULL || l->tamanho != 0) return false;
    l->ordenada = true; l->campo = campo; l->crescente = crescente;
    return true;
}

static void ligar(LISTA_ENC *l, NO *ant, NO *novo, NO *prox) {
    novo->anterior = ant; novo->proximo = prox;
    if (ant != NULL) ant->proximo = novo; else l->inicio = novo;
    if (prox != NULL) prox->anterior = novo; else l->fim = novo;
    ++l->tamanho;
}

bool lista_enc_inserir_inicio(LISTA_ENC *l, ITEM *item) {
    if (l == NULL || item == NULL || l->ordenada) return false;
    NO *n = no_criar(item); if (n == NULL) return false;
    ligar(l, NULL, n, l->inicio); return true;
}

bool lista_enc_inserir_fim(LISTA_ENC *l, ITEM *item) {
    if (l == NULL || item == NULL || l->ordenada) return false;
    NO *n = no_criar(item); if (n == NULL) return false;
    ligar(l, l->fim, n, NULL); return true;
}

bool lista_enc_inserir(LISTA_ENC *l, ITEM *item) {
    if (l == NULL || item == NULL) return false;
    NO *n = no_criar(item); if (n == NULL) return false;
    if (!l->ordenada) { ligar(l, l->fim, n, NULL); return true; }
    NO *atual = l->inicio;
    while (atual != NULL) {
        int c = item_comparar_campo(atual->item, item, l->campo);
        if (!l->crescente) c = -c;
        if (c > 0) break;
        atual = atual->proximo;
    }
    ligar(l, atual != NULL ? atual->anterior : l->fim, n, atual);
    return true;
}

ITEM *lista_enc_busca_item(const LISTA_ENC *l, const ITEM *item) {
    if (l == NULL || item == NULL) return NULL;
    for (NO *n=l->inicio; n!=NULL; n=n->proximo) if (n->item==item) return n->item;
    return NULL;
}

ITEM *lista_enc_remover(LISTA_ENC *l, ITEM *item) {
    if (l == NULL || item == NULL) return NULL;
    for (NO *n=l->inicio; n!=NULL; n=n->proximo) if (n->item==item) {
        if(n->anterior) n->anterior->proximo=n->proximo; else l->inicio=n->proximo;
        if(n->proximo) n->proximo->anterior=n->anterior; else l->fim=n->anterior;
        ITEM *ret=n->item; free(n); --l->tamanho; return ret;
    }
    return NULL;
}

int lista_enc_tamanho(const LISTA_ENC *l) { return l ? l->tamanho : 0; }
bool lista_enc_vazia(const LISTA_ENC *l) { return l == NULL || l->inicio == NULL; }
bool lista_enc_cheia(const LISTA_ENC *l) { (void)l; return false; }

static void imprimir(const ITEM *item) { const char *s=item_original(item); puts(s?s:"<item>"); }
void lista_enc_imprimir(const LISTA_ENC *l) { if(l) for(NO*n=l->inicio;n;n=n->proximo) imprimir(n->item); }
void lista_enc_imprimir_reversa(const LISTA_ENC *l) { if(l) for(NO*n=l->fim;n;n=n->anterior) imprimir(n->item); }
void lista_enc_percorrer(const LISTA_ENC *l,TAD_VISITANTE f,void*c) { if(l&&f) for(NO*n=l->inicio;n;n=n->proximo) f(n->item,c); }
