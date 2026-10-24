#include "lista_cruz.h"

#include <stdint.h>
#include <stdlib.h>

typedef struct grupo GRUPO;
typedef struct celula CELULA;
typedef struct item_no ITEM_NO;

struct item_no {
    ITEM *item;
    ITEM_NO *proximo;
    ITEM_NO *proximo_hash;
};

struct celula {
    GRUPO *linha;
    GRUPO *coluna;
    CELULA *proxima_linha;
    CELULA *proxima_coluna;
    ITEM_NO *itens;
};

struct grupo {
    ITEM *representante;
    CELULA *celulas;
    GRUPO *proximo;
    size_t quantidade;
};

struct lista_cruz_ {
    size_t campo_linha;
    size_t campo_coluna;
    size_t tamanho;
    size_t capacidade_hash;
    const ESQUEMA *esquema;
    GRUPO *linhas;
    GRUPO *colunas;
    ITEM_NO **hash;
};

static size_t hash_item(const LISTA_CRUZ *lista, const ITEM *item) {
    uintptr_t endereco = (uintptr_t)item;
    return ((endereco >> 4) ^ (endereco >> 13)) % lista->capacidade_hash;
}

static ITEM_NO *buscar_hash(const LISTA_CRUZ *lista, const ITEM *item) {
    size_t posicao;
    if (lista == NULL || item == NULL) return NULL;
    posicao = hash_item(lista, item);
    for (ITEM_NO *no = lista->hash[posicao]; no != NULL;
         no = no->proximo_hash)
        if (no->item == item) return no;
    return NULL;
}

static bool ampliar_hash(LISTA_CRUZ *lista) {
    size_t capacidade;
    ITEM_NO **novo_hash;

    if (lista->tamanho + 1 <=
        lista->capacidade_hash - lista->capacidade_hash / 4) return true;
    if (lista->capacidade_hash > SIZE_MAX / 2 / sizeof(*lista->hash))
        return false;

    capacidade = lista->capacidade_hash * 2;
    novo_hash = calloc(capacidade, sizeof(*novo_hash));
    if (novo_hash == NULL) return false;

    lista->capacidade_hash = capacidade;
    for (GRUPO *linha = lista->linhas; linha != NULL; linha = linha->proximo) {
        for (CELULA *celula = linha->celulas; celula != NULL;
             celula = celula->proxima_linha) {
            for (ITEM_NO *no = celula->itens; no != NULL; no = no->proximo) {
                size_t posicao = hash_item(lista, no->item);
                no->proximo_hash = novo_hash[posicao];
                novo_hash[posicao] = no;
            }
        }
    }
    free(lista->hash);
    lista->hash = novo_hash;
    return true;
}

static void remover_hash(LISTA_CRUZ *lista, ITEM *item) {
    size_t posicao = hash_item(lista, item);
    ITEM_NO **elo = &lista->hash[posicao];

    while (*elo != NULL && (*elo)->item != item)
        elo = &(*elo)->proximo_hash;
    if (*elo != NULL) {
        ITEM_NO *removido = *elo;
        *elo = removido->proximo_hash;
        removido->proximo_hash = NULL;
    }
}

LISTA_CRUZ *lista_cruz_criar(size_t campo_linha, size_t campo_coluna) {
    LISTA_CRUZ *lista;
    if (campo_linha == campo_coluna) return NULL;

    lista = calloc(1, sizeof(*lista));
    if (lista == NULL) return NULL;
    lista->campo_linha = campo_linha;
    lista->campo_coluna = campo_coluna;
    lista->capacidade_hash = 32;
    lista->hash = calloc(lista->capacidade_hash, sizeof(*lista->hash));
    if (lista->hash == NULL) {
        free(lista);
        return NULL;
    }
    return lista;
}

static GRUPO *buscar_grupo(GRUPO *grupos, const ITEM *item, size_t campo) {
    for (GRUPO *grupo = grupos; grupo != NULL; grupo = grupo->proximo)
        if (item_comparar_campo(grupo->representante, item, campo) == 0)
            return grupo;
    return NULL;
}

static CELULA *buscar_celula(GRUPO *linha, GRUPO *coluna) {
    for (CELULA *celula = linha->celulas; celula != NULL;
         celula = celula->proxima_linha)
        if (celula->coluna == coluna) return celula;
    return NULL;
}

ITEM *lista_cruz_buscar(const LISTA_CRUZ *lista, const ITEM *item) {
    ITEM_NO *no = buscar_hash(lista, item);
    return no != NULL ? no->item : NULL;
}

bool lista_cruz_inserir(LISTA_CRUZ *lista, ITEM *item) {
    GRUPO *linha, *coluna, *nova_linha, *nova_coluna;
    CELULA *celula, *nova_celula;
    ITEM_NO *novo_item;

    if (lista == NULL || item == NULL ||
        item_campo(item, lista->campo_linha) == NULL ||
        item_campo(item, lista->campo_coluna) == NULL ||
        (lista->esquema != NULL && lista->esquema != item_esquema(item)) ||
        lista_cruz_buscar(lista, item) != NULL || !ampliar_hash(lista))
        return false;

    linha = buscar_grupo(lista->linhas, item, lista->campo_linha);
    coluna = buscar_grupo(lista->colunas, item, lista->campo_coluna);
    nova_linha = linha == NULL ? calloc(1, sizeof(*nova_linha)) : NULL;
    nova_coluna = coluna == NULL ? calloc(1, sizeof(*nova_coluna)) : NULL;
    if ((linha == NULL && nova_linha == NULL) ||
        (coluna == NULL && nova_coluna == NULL)) {
        free(nova_linha);
        free(nova_coluna);
        return false;
    }

    if (linha == NULL) {
        linha = nova_linha;
        linha->representante = item;
    }
    if (coluna == NULL) {
        coluna = nova_coluna;
        coluna->representante = item;
    }

    celula = buscar_celula(linha, coluna);
    nova_celula = celula == NULL ? calloc(1, sizeof(*nova_celula)) : NULL;
    novo_item = malloc(sizeof(*novo_item));
    if (novo_item == NULL || (celula == NULL && nova_celula == NULL)) {
        free(novo_item);
        free(nova_celula);
        free(nova_linha);
        free(nova_coluna);
        return false;
    }

    if (nova_linha != NULL) {
        nova_linha->proximo = lista->linhas;
        lista->linhas = nova_linha;
    }
    if (nova_coluna != NULL) {
        nova_coluna->proximo = lista->colunas;
        lista->colunas = nova_coluna;
    }
    if (nova_celula != NULL) {
        nova_celula->linha = linha;
        nova_celula->coluna = coluna;
        nova_celula->proxima_linha = linha->celulas;
        linha->celulas = nova_celula;
        nova_celula->proxima_coluna = coluna->celulas;
        coluna->celulas = nova_celula;
        celula = nova_celula;
    }

    novo_item->item = item;
    novo_item->proximo = celula->itens;
    celula->itens = novo_item;
    size_t posicao = hash_item(lista, item);
    novo_item->proximo_hash = lista->hash[posicao];
    lista->hash[posicao] = novo_item;

    ++linha->quantidade;
    ++coluna->quantidade;
    ++lista->tamanho;
    if (lista->esquema == NULL) lista->esquema = item_esquema(item);
    return true;
}

static void remover_celula(CELULA **inicio, CELULA *alvo, bool por_linha) {
    CELULA **elo = inicio;
    while (*elo != NULL && *elo != alvo)
        elo = por_linha ? &(*elo)->proxima_linha : &(*elo)->proxima_coluna;
    if (*elo != NULL)
        *elo = por_linha ? alvo->proxima_linha : alvo->proxima_coluna;
}

static void remover_grupo(GRUPO **inicio, GRUPO *alvo) {
    GRUPO **elo = inicio;
    while (*elo != NULL && *elo != alvo) elo = &(*elo)->proximo;
    if (*elo != NULL) *elo = alvo->proximo;
    free(alvo);
}

ITEM *lista_cruz_remover(LISTA_CRUZ *lista, ITEM *item) {
    if (lista == NULL || item == NULL) return NULL;

    for (GRUPO *linha = lista->linhas; linha != NULL; linha = linha->proximo) {
        CELULA **elo_celula = &linha->celulas;
        while (*elo_celula != NULL) {
            CELULA *celula = *elo_celula;
            ITEM_NO **elo_item = &celula->itens;
            while (*elo_item != NULL && (*elo_item)->item != item)
                elo_item = &(*elo_item)->proximo;

            if (*elo_item != NULL) {
                GRUPO *coluna = celula->coluna;
                ITEM_NO *removido = *elo_item;
                *elo_item = removido->proximo;
                remover_hash(lista, item);
                free(removido);
                --linha->quantidade;
                --coluna->quantidade;
                --lista->tamanho;

                if (celula->itens == NULL) {
                    remover_celula(&linha->celulas, celula, true);
                    remover_celula(&coluna->celulas, celula, false);
                    free(celula);
                }
                if (linha->quantidade == 0)
                    remover_grupo(&lista->linhas, linha);
                else if (linha->representante == item)
                    linha->representante = linha->celulas->itens->item;
                if (coluna->quantidade == 0)
                    remover_grupo(&lista->colunas, coluna);
                else if (coluna->representante == item)
                    coluna->representante = coluna->celulas->itens->item;
                return item;
            }
            elo_celula = &celula->proxima_linha;
        }
    }
    return NULL;
}

void lista_cruz_apagar(LISTA_CRUZ **ponteiro) {
    GRUPO *linha;

    if (ponteiro == NULL || *ponteiro == NULL) return;
    linha = (*ponteiro)->linhas;
    while (linha != NULL) {
        GRUPO *proxima_linha = linha->proximo;
        CELULA *celula = linha->celulas;
        while (celula != NULL) {
            CELULA *proxima_celula = celula->proxima_linha;
            ITEM_NO *no = celula->itens;
            while (no != NULL) {
                ITEM_NO *proximo = no->proximo;
                free(no);
                no = proximo;
            }
            free(celula);
            celula = proxima_celula;
        }
        free(linha);
        linha = proxima_linha;
    }

    GRUPO *coluna = (*ponteiro)->colunas;
    while (coluna != NULL) {
        GRUPO *proximo = coluna->proximo;
        free(coluna);
        coluna = proximo;
    }
    free((*ponteiro)->hash);
    free(*ponteiro);
    *ponteiro = NULL;
}

void lista_cruz_percorrer(const LISTA_CRUZ *lista, LISTA_CRUZ_EIXO eixo,
                          TAD_VISITANTE visitante, void *contexto) {
    GRUPO *grupos;
    if (lista == NULL || visitante == NULL) return;
    grupos = eixo == CRUZ_POR_LINHA ? lista->linhas : lista->colunas;

    for (GRUPO *grupo = grupos; grupo != NULL; grupo = grupo->proximo) {
        CELULA *celula = grupo->celulas;
        while (celula != NULL) {
            ITEM_NO *no = celula->itens;
            while (no != NULL) {
                visitante(no->item, contexto);
                no = no->proximo;
            }
            celula = eixo == CRUZ_POR_LINHA
                ? celula->proxima_linha : celula->proxima_coluna;
        }
    }
}

size_t lista_cruz_grupos(const LISTA_CRUZ *lista, LISTA_CRUZ_EIXO eixo) {
    size_t quantidade = 0;
    GRUPO *grupo;
    if (lista == NULL) return 0;
    grupo = eixo == CRUZ_POR_LINHA ? lista->linhas : lista->colunas;
    for (; grupo != NULL; grupo = grupo->proximo) ++quantidade;
    return quantidade;
}
