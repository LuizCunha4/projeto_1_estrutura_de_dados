#include "lista_cruz.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;

struct no_ {
    int linha;
    int coluna;
    ITEM *item;
    NO *prox_linha;
    NO *prox_coluna;
};

struct lista_cruz_ {
    int linhas;
    int colunas;
    NO **inicio_linha;
    NO **inicio_coluna;
};

static bool posicao_valida(const LISTA_CRUZ *lista, int linha, int coluna) {
    return lista != NULL &&
           linha >= 0 && linha < lista->linhas &&
           coluna >= 0 && coluna < lista->colunas;
}

static NO *achar_no(const LISTA_CRUZ *lista, int linha, int coluna) {
    for (NO *no = lista->inicio_linha[linha]; no != NULL; no = no->prox_linha)
        if (no->coluna == coluna)
            return no;
    return NULL;
}

LISTA_CRUZ *lista_cruz_criar(int linhas, int colunas) {
    if (linhas <= 0 || colunas <= 0)
        return NULL;

    LISTA_CRUZ *lista = malloc(sizeof(LISTA_CRUZ));
    if (lista == NULL)
        return NULL;

    lista->linhas = linhas;
    lista->colunas = colunas;
    lista->inicio_linha = calloc((size_t)linhas, sizeof(NO *));
    lista->inicio_coluna = calloc((size_t)colunas, sizeof(NO *));

    if (lista->inicio_linha == NULL || lista->inicio_coluna == NULL) {
        free(lista->inicio_linha);
        free(lista->inicio_coluna);
        free(lista);
        return NULL;
    }
    return lista;
}

void lista_cruz_apagar(LISTA_CRUZ **lista) {
    if (lista == NULL || *lista == NULL)
        return;

    for (int i = 0; i < (*lista)->linhas; i++) {
        NO *no = (*lista)->inicio_linha[i];
        while (no != NULL) {
            NO *prox = no->prox_linha;
            item_apagar(&no->item);
            free(no);
            no = prox;
        }
    }

    free((*lista)->inicio_linha);
    free((*lista)->inicio_coluna);
    free(*lista);
    *lista = NULL;
}

bool lista_cruz_inserir(LISTA_CRUZ *lista, int linha, int coluna, ITEM *item) {
    if (!posicao_valida(lista, linha, coluna) || item == NULL)
        return false;

    if (achar_no(lista, linha, coluna) != NULL)
        return false;

    NO *novo = malloc(sizeof(NO));
    if (novo == NULL)
        return false;

    novo->linha = linha;
    novo->coluna = coluna;
    novo->item = item;
    novo->prox_linha = lista->inicio_linha[linha];
    novo->prox_coluna = lista->inicio_coluna[coluna];

    lista->inicio_linha[linha] = novo;
    lista->inicio_coluna[coluna] = novo;
    return true;
}

ITEM *lista_cruz_buscar(const LISTA_CRUZ *lista, int linha, int coluna) {
    if (!posicao_valida(lista, linha, coluna))
        return NULL;

    NO *no = achar_no(lista, linha, coluna);
    return no != NULL ? no->item : NULL;
}

ITEM *lista_cruz_remover(LISTA_CRUZ *lista, int linha, int coluna) {
    if (!posicao_valida(lista, linha, coluna))
        return NULL;

    NO **elo_linha = &lista->inicio_linha[linha];
    while (*elo_linha != NULL && (*elo_linha)->coluna != coluna)
        elo_linha = &(*elo_linha)->prox_linha;

    NO *no = *elo_linha;
    if (no == NULL)
        return NULL;

    NO **elo_coluna = &lista->inicio_coluna[coluna];
    while (*elo_coluna != no)
        elo_coluna = &(*elo_coluna)->prox_coluna;

    *elo_linha = no->prox_linha;
    *elo_coluna = no->prox_coluna;

    ITEM *item = no->item;
    free(no);
    return item;
}

void lista_cruz_imprimir_linha(const LISTA_CRUZ *lista, int linha) {
    if (lista == NULL || linha < 0 || linha >= lista->linhas)
        return;

    for (const NO *no = lista->inicio_linha[linha]; no != NULL; no = no->prox_linha) {
        if (no != lista->inicio_linha[linha])
            printf(" ");
        item_imprimir(no->item);
    }
    printf("\n");
}

void lista_cruz_imprimir_coluna(const LISTA_CRUZ *lista, int coluna) {
    if (lista == NULL || coluna < 0 || coluna >= lista->colunas)
        return;

    for (const NO *no = lista->inicio_coluna[coluna]; no != NULL; no = no->prox_coluna) {
        if (no != lista->inicio_coluna[coluna])
            printf(" ");
        item_imprimir(no->item);
    }
    printf("\n");
}
