#include "lista_gen.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;
typedef enum { NO_ATOMO, NO_GRUPO } TIPO_NO;

struct no_ {
    TIPO_NO tipo;
    ITEM *item;
    NO *proximo;
    LISTA_GEN *sublista;
};

struct lista_gen_ {
    NO *inicio;
    NO *fim;
    size_t quantidade;
    size_t profundidade;
    size_t *campos;
    size_t quantidade_campos;
    bool dono_campos;
    const ESQUEMA *esquema;
};

LISTA_GEN *lista_gen_criar(void) {
    return calloc(1, sizeof(LISTA_GEN));
}

static bool lista_vazia(const LISTA_GEN *lista) {
    return lista == NULL || lista->inicio == NULL;
}

void lista_gen_apagar(LISTA_GEN **ponteiro) {
    NO *no;

    if (ponteiro == NULL || *ponteiro == NULL) return;
    no = (*ponteiro)->inicio;
    while (no != NULL) {
        NO *proximo = no->proximo;
        if (no->tipo == NO_GRUPO) lista_gen_apagar(&no->sublista);
        free(no);
        no = proximo;
    }
    if ((*ponteiro)->dono_campos) free((*ponteiro)->campos);
    free(*ponteiro);
    *ponteiro = NULL;
}

bool lista_gen_configurar_grupos(LISTA_GEN *lista, const size_t *campos,
                                 size_t quantidade) {
    size_t *copia;

    if (lista == NULL || campos == NULL || quantidade < 2 ||
        lista->quantidade != 0 || quantidade > SIZE_MAX / sizeof(*campos))
        return false;

    copia = malloc(quantidade * sizeof(*copia));
    if (copia == NULL) return false;
    for (size_t i = 0; i < quantidade; ++i) copia[i] = campos[i];

    if (lista->dono_campos) free(lista->campos);
    lista->campos = copia;
    lista->quantidade_campos = quantidade;
    lista->dono_campos = true;
    return true;
}

static LISTA_GEN *filha_criar(const LISTA_GEN *pai) {
    LISTA_GEN *filha = lista_gen_criar();
    if (filha == NULL) return NULL;

    filha->campos = pai->campos;
    filha->quantidade_campos = pai->quantidade_campos;
    filha->esquema = pai->esquema;
    filha->profundidade = pai->profundidade + 1;
    return filha;
}

static NO *no_criar(TIPO_NO tipo, ITEM *item) {
    NO *no = calloc(1, sizeof(*no));
    if (no != NULL) {
        no->tipo = tipo;
        no->item = item;
    }
    return no;
}

static void anexar(LISTA_GEN *lista, NO *no) {
    if (lista->fim != NULL) lista->fim->proximo = no;
    else lista->inicio = no;
    lista->fim = no;
    ++lista->quantidade;
}

static bool inserir_recursivo(LISTA_GEN *lista, ITEM *item) {
    if (lista == NULL || item == NULL) return false;

    if (lista->profundidade < lista->quantidade_campos) {
        size_t campo = lista->campos[lista->profundidade];
        for (NO *no = lista->inicio; no != NULL; no = no->proximo) {
            if (no->tipo == NO_GRUPO && no->item != NULL &&
                item_comparar_campo(no->item, item, campo) == 0)
                return inserir_recursivo(no->sublista, item);
        }

        LISTA_GEN *filha = filha_criar(lista);
        if (filha == NULL) return false;
        NO *grupo = no_criar(NO_GRUPO, item);
        if (grupo == NULL) {
            lista_gen_apagar(&filha);
            return false;
        }
        grupo->sublista = filha;
        if (!inserir_recursivo(filha, item)) {
            free(grupo);
            lista_gen_apagar(&filha);
            return false;
        }
        anexar(lista, grupo);
        return true;
    }

    NO *atomo = no_criar(NO_ATOMO, item);
    if (atomo == NULL) return false;
    anexar(lista, atomo);
    return true;
}

bool lista_gen_inserir_atomo(LISTA_GEN *lista, ITEM *item) {
    const ESQUEMA *esquema_anterior;

    if (lista == NULL || item == NULL ||
        (lista->esquema != NULL && lista->esquema != item_esquema(item)))
        return false;
    for (size_t i = 0; i < lista->quantidade_campos; ++i)
        if (item_campo(item, lista->campos[i]) == NULL) return false;

    esquema_anterior = lista->esquema;
    if (lista->esquema == NULL) lista->esquema = item_esquema(item);
    if (inserir_recursivo(lista, item)) return true;
    lista->esquema = esquema_anterior;
    return false;
}

LISTA_GEN *lista_gen_inserir_sublista(LISTA_GEN *lista) {
    LISTA_GEN *sublista;
    NO *grupo;

    if (lista == NULL) return NULL;
    sublista = filha_criar(lista);
    if (sublista == NULL) return NULL;
    grupo = no_criar(NO_GRUPO, NULL);
    if (grupo == NULL) {
        lista_gen_apagar(&sublista);
        return NULL;
    }
    grupo->sublista = sublista;
    anexar(lista, grupo);
    return sublista;
}

static const ITEM *primeiro_item(const LISTA_GEN *lista) {
    if (lista == NULL) return NULL;
    for (NO *no = lista->inicio; no != NULL; no = no->proximo) {
        const ITEM *item;
        if (no->tipo == NO_ATOMO) return no->item;
        item = primeiro_item(no->sublista);
        if (item != NULL) return item;
    }
    return NULL;
}

bool lista_gen_contem_item(const LISTA_GEN *lista, const ITEM *item) {
    if (lista == NULL || item == NULL) return false;
    for (NO *no = lista->inicio; no != NULL; no = no->proximo) {
        if (no->tipo == NO_ATOMO && no->item == item) return true;
        if (no->tipo == NO_GRUPO && lista_gen_contem_item(no->sublista, item))
            return true;
    }
    return false;
}

static void atualizar_fim(LISTA_GEN *lista) {
    lista->fim = NULL;
    for (NO *no = lista->inicio; no != NULL; no = no->proximo)
        lista->fim = no;
}

static bool remover_recursivo(LISTA_GEN *lista, ITEM *item) {
    NO **elo = &lista->inicio;

    while (*elo != NULL) {
        NO *no = *elo;
        if (no->tipo == NO_ATOMO && no->item == item) {
            *elo = no->proximo;
            if (lista->fim == no) atualizar_fim(lista);
            free(no);
            --lista->quantidade;
            return true;
        }

        if (no->tipo == NO_GRUPO && remover_recursivo(no->sublista, item)) {
            if (lista_vazia(no->sublista)) {
                *elo = no->proximo;
                if (lista->fim == no) atualizar_fim(lista);
                lista_gen_apagar(&no->sublista);
                free(no);
                --lista->quantidade;
            } else if (no->item == item) {
                no->item = (ITEM *)primeiro_item(no->sublista);
            }
            return true;
        }
        elo = &no->proximo;
    }
    return false;
}

ITEM *lista_gen_remover_item(LISTA_GEN *lista, ITEM *item) {
    return lista != NULL && item != NULL && remover_recursivo(lista, item)
        ? item : NULL;
}

int lista_gen_profundidade(const LISTA_GEN *lista) {
    int maior = 0;
    if (lista == NULL) return 0;
    for (NO *no = lista->inicio; no != NULL; no = no->proximo) {
        if (no->tipo == NO_GRUPO) {
            int profundidade = lista_gen_profundidade(no->sublista);
            if (profundidade > maior) maior = profundidade;
        }
    }
    return maior + 1;
}

void lista_gen_percorrer(const LISTA_GEN *lista, TAD_VISITANTE visitante,
                         void *contexto) {
    if (lista == NULL || visitante == NULL) return;
    for (NO *no = lista->inicio; no != NULL; no = no->proximo) {
        if (no->tipo == NO_ATOMO) visitante(no->item, contexto);
        else lista_gen_percorrer(no->sublista, visitante, contexto);
    }
}

void lista_gen_imprimir(const LISTA_GEN *lista) {
    if (lista == NULL) return;
    printf("(");
    for (NO *no = lista->inicio; no != NULL; no = no->proximo) {
        if (no->tipo == NO_ATOMO) {
            const char *original = item_original(no->item);
            printf("%s", original != NULL ? original : "<item>");
        } else {
            lista_gen_imprimir(no->sublista);
        }
        if (no->proximo != NULL) printf(", ");
    }
    printf(")");
}
