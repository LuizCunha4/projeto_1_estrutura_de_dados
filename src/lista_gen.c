#include "lista_gen.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;

enum tipo_no { NO_ATOMO, NO_SUBLISTA };

struct no_ {
    enum tipo_no tipo;
    union {
        ITEM *atomo;
        LISTA_GEN *sublista;
    } info;
    NO *proximo;
};

struct lista_gen_ {
    NO *inicio;
};

static NO *no_criar(enum tipo_no tipo) {
    NO *no = malloc(sizeof(NO));
    if (no == NULL)
        return NULL;

    no->tipo = tipo;
    if (tipo == NO_ATOMO)
        no->info.atomo = NULL;
    else
        no->info.sublista = NULL;
    no->proximo = NULL;
    return no;
}

static void anexar_no(LISTA_GEN *lista, NO *novo) {
    NO **elo = &lista->inicio;
    while (*elo != NULL)
        elo = &(*elo)->proximo;
    *elo = novo;
}

LISTA_GEN *lista_gen_criar(void) {
    LISTA_GEN *lista = malloc(sizeof(LISTA_GEN));

    if (lista != NULL)
        lista->inicio = NULL;

    return lista;
}

void lista_gen_apagar(LISTA_GEN **lista) {
    if (lista == NULL || *lista == NULL)
        return;

    NO *no = (*lista)->inicio;
    while (no != NULL) {
        NO *prox = no->proximo;

        if (no->tipo == NO_ATOMO)
            item_apagar(&no->info.atomo);
        else
            lista_gen_apagar(&no->info.sublista);

        free(no);
        no = prox;
    }

    free(*lista);
    *lista = NULL;
}

bool lista_gen_inserir_atomo(LISTA_GEN *lista, ITEM *item) {
    if (lista == NULL || item == NULL)
        return false;

    NO *no = no_criar(NO_ATOMO);
    if (no == NULL)
        return false;

    no->info.atomo = item;
    anexar_no(lista, no);
    return true;
}

LISTA_GEN *lista_gen_inserir_sublista(LISTA_GEN *lista) {
    if (lista == NULL)
        return NULL;

    NO *no = no_criar(NO_SUBLISTA);
    if (no == NULL)
        return NULL;

    no->info.sublista = lista_gen_criar();
    if (no->info.sublista == NULL) {
        free(no);
        return NULL;
    }

    anexar_no(lista, no);
    return no->info.sublista;
}

bool lista_gen_buscar(const LISTA_GEN *lista, int chave) {
    if (lista == NULL)
        return false;

    for (const NO *no = lista->inicio; no != NULL; no = no->proximo) {
        if (no->tipo == NO_ATOMO && item_get_chave(no->info.atomo) == chave)
            return true;
        if (no->tipo == NO_SUBLISTA && lista_gen_buscar(no->info.sublista, chave))
            return true;
    }
    return false;
}

bool lista_gen_remover(LISTA_GEN *lista, int chave) {
    if (lista == NULL)
        return false;

    NO **elo = &lista->inicio;
    while (*elo != NULL) {
        NO *no = *elo;

        if (no->tipo == NO_ATOMO && item_get_chave(no->info.atomo) == chave) {
            *elo = no->proximo;
            item_apagar(&no->info.atomo);
            free(no);
            return true;
        }
        if (no->tipo == NO_SUBLISTA && lista_gen_remover(no->info.sublista, chave))
            return true;

        elo = &no->proximo;
    }
    return false;
}

int lista_gen_profundidade(const LISTA_GEN *lista) {
    if (lista == NULL)
        return 0;

    int maior = 0;
    for (const NO *no = lista->inicio; no != NULL; no = no->proximo) {
        if (no->tipo == NO_SUBLISTA) {
            int p = lista_gen_profundidade(no->info.sublista);
            if (p > maior)
                maior = p;
        }
    }
    return 1 + maior;
}

void lista_gen_imprimir(const LISTA_GEN *lista) {
    if (lista == NULL)
        return;

    printf("(");
    for (const NO *no = lista->inicio; no != NULL; no = no->proximo) {
        if (no->tipo == NO_ATOMO)
            item_imprimir(no->info.atomo);
        else
            lista_gen_imprimir(no->info.sublista);

        if (no->proximo != NULL)
            printf(", ");
    }
    printf(")");
}
