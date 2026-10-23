#include "pilha.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct no NO;

struct no {
    ITEM *item;
    NO *anterior;
};

struct pilha {
    NO *topo;
    int tamanho;
};

PILHA *pilha_criar(void) {
    PILHA *pilha = malloc(sizeof(PILHA));

    if (pilha != NULL) {
        pilha->topo = NULL;
        pilha->tamanho = 0;
    }

    return pilha;
}

void pilha_apagar(PILHA **pilha) {
    NO *atual;

    if (pilha == NULL || *pilha == NULL) {
        return;
    }

    atual = (*pilha)->topo;
    while (atual != NULL) {
        NO *anterior = atual->anterior;
        free(atual);
        atual = anterior;
    }

    free(*pilha);
    *pilha = NULL;
}

bool pilha_vazia(PILHA *pilha) {
    return pilha != NULL && pilha->topo == NULL;
}

bool pilha_cheia(PILHA *pilha) {
    (void) pilha;
    return false;
}

int pilha_tamanho(PILHA *pilha) {
    return pilha != NULL ? pilha->tamanho : -1;
}

ITEM *pilha_topo(PILHA *pilha) {
    if (pilha == NULL || pilha_vazia(pilha)) {
        return NULL;
    }

    return pilha->topo->item;
}

bool pilha_empilhar(PILHA *pilha, ITEM *item) {
    NO *novo;

    if (pilha == NULL || item == NULL) {
        return false;
    }

    novo = malloc(sizeof(NO));
    if (novo == NULL) {
        return false;
    }

    novo->item = item;
    novo->anterior = pilha->topo;
    pilha->topo = novo;
    pilha->tamanho++;
    return true;
}

ITEM *pilha_desempilhar(PILHA *pilha) {
    NO *removido;
    ITEM *item;

    if (pilha == NULL || pilha_vazia(pilha)) {
        return NULL;
    }

    removido = pilha->topo;
    item = removido->item;
    pilha->topo = removido->anterior;
    pilha->tamanho--;
    free(removido);
    return item;
}

ITEM *pilha_remover_item(PILHA *pilha, ITEM *item) {
    if (pilha == NULL || item == NULL) return NULL;
    NO **elo = &pilha->topo;
    while (*elo != NULL && (*elo)->item != item) elo = &(*elo)->anterior;
    if (*elo == NULL) return NULL;
    NO *removido = *elo;
    *elo = removido->anterior;
    ITEM *retirado = removido->item;
    free(removido);
    --pilha->tamanho;
    return retirado;
}

void pilha_imprimir(PILHA *pilha) {
    NO *atual;

    if (pilha == NULL) {
        return;
    }

    printf("Topo -> ");
    atual = pilha->topo;
    while (atual != NULL) {
        const char *original = item_original(atual->item);
        printf("%s ", original != NULL ? original : "<item>");
        atual = atual->anterior;
    }
    printf("<- Base\n");
}

void pilha_percorrer(const PILHA *pilha, TAD_VISITANTE visitante,
                     void *contexto) {
    if (pilha == NULL || visitante == NULL) return;
    for (const NO *no = pilha->topo; no != NULL; no = no->anterior)
        visitante(no->item, contexto);
}

void pilha_inverter(PILHA *pilha) {
    NO *atual;
    NO *anterior = NULL;

    if (pilha == NULL) {
        return;
    }

    atual = pilha->topo;
    while (atual != NULL) {
        NO *proximo = atual->anterior;
        atual->anterior = anterior;
        anterior = atual;
        atual = proximo;
    }

    pilha->topo = anterior;
}
