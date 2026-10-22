#include "lista_enc.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;

struct no_ {
    ITEM *item;
    NO *anterior;
    NO *proximo;
};

struct lista_enc {
    NO *inicio;
    NO *fim;
    int tamanho;
    bool ordenada;
};

static NO *no_criar(ITEM *item) {
    NO *novo = malloc(sizeof(NO));
    if (novo != NULL) {
        novo->item = item;
        novo->anterior = NULL;
        novo->proximo = NULL;
    }
    return novo;
}

static void ligar_entre(LISTA_ENC *lista, NO *anterior, NO *novo, NO *proximo) {
    novo->anterior = anterior;
    novo->proximo = proximo;

    if (anterior == NULL)
        lista->inicio = novo;
    else
        anterior->proximo = novo;

    if (proximo == NULL)
        lista->fim = novo;
    else
        proximo->anterior = novo;

    lista->tamanho++;
}

static bool inserir_ordenado(LISTA_ENC *lista, ITEM *item) {
    NO *novo = no_criar(item);
    if (novo == NULL)
        return false;

    const int chave = item_chave(item);
    NO *atual = lista->inicio;

    while (atual != NULL && item_chave(atual->item) < chave)
        atual = atual->proximo;

    NO *anterior = atual != NULL ? atual->anterior : lista->fim;
    ligar_entre(lista, anterior, novo, atual);
    return true;
}

LISTA_ENC *lista_enc_criar(bool ordenada) {
    LISTA_ENC *lista = malloc(sizeof(LISTA_ENC));
    if (lista == NULL)
        return NULL;

    lista->inicio = NULL;
    lista->fim = NULL;
    lista->tamanho = 0;
    lista->ordenada = ordenada;
    return lista;
}

void lista_enc_apagar(LISTA_ENC **lista) {
    if (lista == NULL || *lista == NULL)
        return;

    NO *p = (*lista)->inicio;
    while (p != NULL) {
        NO *proximo = p->proximo;
        item_apagar(&p->item);
        free(p);
        p = proximo;
    }

    free(*lista);
    *lista = NULL;
}

bool lista_enc_inserir_inicio(LISTA_ENC *lista, ITEM *item) {
    if (lista == NULL || item == NULL || lista->ordenada)
        return false;

    NO *novo = no_criar(item);
    if (novo == NULL)
        return false;

    ligar_entre(lista, NULL, novo, lista->inicio);
    return true;
}

bool lista_enc_inserir_fim(LISTA_ENC *lista, ITEM *item) {
    if (lista == NULL || item == NULL || lista->ordenada)
        return false;

    NO *novo = no_criar(item);
    if (novo == NULL)
        return false;

    ligar_entre(lista, lista->fim, novo, NULL);
    return true;
}

bool lista_enc_inserir(LISTA_ENC *lista, ITEM *item) {
    if (lista == NULL || item == NULL)
        return false;

    if (lista->ordenada)
        return inserir_ordenado(lista, item);

    return lista_enc_inserir_fim(lista, item);
}

ITEM *lista_enc_busca(const LISTA_ENC *lista, int chave) {
    if (lista == NULL)
        return NULL;

    const NO *p = lista->inicio;
    while (p != NULL) {
        int atual = item_chave(p->item);
        if (atual == chave)
            return p->item;
        if (lista->ordenada && atual > chave)
            return NULL;
        p = p->proximo;
    }
    return NULL;
}

ITEM *lista_enc_remover(LISTA_ENC *lista, int chave) {
    if (lista == NULL)
        return NULL;

    NO *p = lista->inicio;
    while (p != NULL) {
        int atual = item_chave(p->item);
        if (atual == chave)
            break;
        if (lista->ordenada && atual > chave)
            return NULL;
        p = p->proximo;
    }

    if (p == NULL)
        return NULL;

    if (p->anterior == NULL)
        lista->inicio = p->proximo;
    else
        p->anterior->proximo = p->proximo;

    if (p->proximo == NULL)
        lista->fim = p->anterior;
    else
        p->proximo->anterior = p->anterior;

    ITEM *item = p->item;
    p->anterior = NULL;
    p->proximo = NULL;
    free(p);
    lista->tamanho--;
    return item;
}

int lista_enc_tamanho(const LISTA_ENC *lista) {
    return lista != NULL ? lista->tamanho : 0;
}

bool lista_enc_vazia(const LISTA_ENC *lista) {
    return lista != NULL && lista->inicio == NULL;
}

bool lista_enc_cheia(const LISTA_ENC *lista) {
    (void)lista;
    return false;
}

void lista_enc_imprimir(const LISTA_ENC *lista) {
    if (lista == NULL) {
        printf("<lista inexistente>\n");
        return;
    }

    printf("inicio -> ");
    for (const NO *p = lista->inicio; p != NULL; p = p->proximo) {
        item_escrever_csv(p->item, stdout);
        printf(" <-> ");
    }
    printf("NULL\n");
}

void lista_enc_imprimir_reversa(const LISTA_ENC *lista) {
    if (lista == NULL) {
        printf("<lista inexistente>\n");
        return;
    }

    printf("fim -> ");
    for (const NO *p = lista->fim; p != NULL; p = p->anterior) {
        item_escrever_csv(p->item, stdout);
        printf(" <-> ");
    }
    printf("NULL\n");
}
