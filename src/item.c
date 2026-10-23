#include "item.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

struct esquema {
    size_t quantidade;
    COLUNA *colunas;
};

struct item {
    const ESQUEMA *esquema; /* Emprestado; deve sobreviver ao item. */
    CAMPO *campos;          /* Vetor e textos pertencem ao item. */
    char *original;        /* Copia opcional; invalidada ao alterar campos. */
    const void *base_dona; /* Token da BASE que recebeu o item. */
};

static bool tipo_valido(TIPO_CAMPO tipo) {
    return tipo == CAMPO_TEXTO || tipo == CAMPO_INTEIRO ||
           tipo == CAMPO_REAL || tipo == CAMPO_BOOLEANO;
}

/* Copia sem truncamento, usando somente a biblioteca padrao C. */
static char *copiar_texto(const char *texto) {
    size_t tamanho = strlen(texto);
    if (tamanho == SIZE_MAX)
        return NULL;
    char *copia = malloc(tamanho + 1);
    if (copia != NULL)
        memcpy(copia, texto, tamanho + 1);
    return copia;
}

ESQUEMA *esquema_criar(const COLUNA *colunas, size_t quantidade) {
    if (colunas == NULL || quantidade == 0 ||
        quantidade > SIZE_MAX / sizeof(COLUNA))
        return NULL;
    ESQUEMA *esquema = malloc(sizeof(*esquema));
    if (esquema == NULL)
        return NULL;
    esquema->quantidade = 0;
    esquema->colunas = calloc(quantidade, sizeof(*esquema->colunas));
    if (esquema->colunas == NULL) {
        free(esquema);
        return NULL;
    }
    /* Contar somente nomes ja copiados permite desfazer construcao parcial. */
    for (size_t i = 0; i < quantidade; i++) {
        size_t repetido;
        if (colunas[i].nome == NULL || colunas[i].nome[0] == '\0' ||
            !tipo_valido(colunas[i].tipo) ||
            esquema_buscar(esquema, colunas[i].nome, &repetido)) {
            esquema_apagar(&esquema);
            return NULL;
        }
        char *nome = copiar_texto(colunas[i].nome);
        if (nome == NULL) {
            esquema_apagar(&esquema);
            return NULL;
        }
        esquema->colunas[i].nome = nome;
        esquema->colunas[i].tipo = colunas[i].tipo;
        esquema->quantidade++;
    }
    return esquema;
}

void esquema_apagar(ESQUEMA **esquema) {
    if (esquema == NULL || *esquema == NULL)
        return;
    for (size_t i = 0; i < (*esquema)->quantidade; i++)
        /* COLUNA.nome e declarado como const char * em item.h para impedir
         * alteracoes por essa interface. Aqui removemos o const no cast
         * para liberar a copia alocada por copiar_texto, pertencente ao esquema.
         * Nao estamos liberando o nome original recebido do chamador. */
        free((void *)(*esquema)->colunas[i].nome);
    free((*esquema)->colunas);
    free(*esquema);
    *esquema = NULL;
}

size_t esquema_quantidade(const ESQUEMA *esquema) {
    return esquema != NULL ? esquema->quantidade : 0;
}

const COLUNA *esquema_coluna(const ESQUEMA *esquema, size_t indice) {
    if (esquema == NULL || indice >= esquema->quantidade)
        return NULL;
    return &esquema->colunas[indice];
}

bool esquema_buscar(const ESQUEMA *esquema, const char *nome, size_t *indice) {
    if (esquema == NULL || nome == NULL || indice == NULL)
        return false;
    for (size_t i = 0; i < esquema->quantidade; i++) {
        if (strcmp(esquema->colunas[i].nome, nome) == 0) {
            *indice = i;
            return true;
        }
    }
    return false;
}

ITEM *item_criar(const ESQUEMA *esquema) {
    if (esquema == NULL || esquema->quantidade > SIZE_MAX / sizeof(CAMPO))
        return NULL;
    ITEM *item = malloc(sizeof(*item));
    if (item == NULL)
        return NULL;
    item->campos = calloc(esquema->quantidade, sizeof(*item->campos));
    if (item->campos == NULL) {
        free(item);
        return NULL;
    }
    item->esquema = esquema;
    item->original = NULL;
    item->base_dona = NULL;
    for (size_t i = 0; i < esquema->quantidade; i++) {
        item->campos[i].tipo = esquema->colunas[i].tipo;
        item->campos[i].ausente = true;
    }
    return item;
}

/* Apenas campos textuais presentes possuem uma alocacao adicional. */
static void campo_limpar(CAMPO *campo) {
    if (campo->tipo == CAMPO_TEXTO && !campo->ausente)
        free((void *)campo->valor.texto);
    campo->valor = (VALOR){0};
    campo->ausente = true;
}

/* Quem gerencia o dataset deve apagar o esquema depois de todos os itens. */
void item_apagar(ITEM **item) {
    if (item == NULL || *item == NULL)
        return;
    for (size_t i = 0; i < (*item)->esquema->quantidade; i++)
        campo_limpar(&(*item)->campos[i]);
    free((*item)->campos);
    free((*item)->original);
    free(*item);
    *item = NULL;
}


const ESQUEMA *item_esquema(const ITEM *item) {
    return item != NULL ? item->esquema : NULL;
}

bool item_reservar_base(ITEM *item, const void *base) {
    if (item == NULL || base == NULL || item->base_dona != NULL) return false;
    item->base_dona = base;
    return true;
}

void item_liberar_base(ITEM *item, const void *base) {
    if (item != NULL && base != NULL && item->base_dona == base)
        item->base_dona = NULL;
}

size_t item_quantidade(const ITEM *item) {
    return item != NULL ? item->esquema->quantidade : 0;
}

const CAMPO *item_campo(const ITEM *item, size_t indice) {
    if (item == NULL || indice >= item->esquema->quantidade)
        return NULL;
    return &item->campos[indice];
}

int item_comparar_campo(const ITEM *a, const ITEM *b, size_t indice) {
    if (a == NULL || b == NULL || a->esquema != b->esquema ||
        indice >= a->esquema->quantidade)
        return 0;
    const CAMPO *x = &a->campos[indice], *y = &b->campos[indice];
    if (x->tipo != y->tipo) return 0;
    if (x->ausente != y->ausente) return x->ausente ? 1 : -1;
    if (x->ausente) return 0;
    switch (x->tipo) {
    case CAMPO_TEXTO: {
        int c = strcmp(x->valor.texto, y->valor.texto);
        return (c > 0) - (c < 0);
    }
    case CAMPO_INTEIRO:
        return (x->valor.inteiro > y->valor.inteiro) -
               (x->valor.inteiro < y->valor.inteiro);
    case CAMPO_REAL:
        return (x->valor.real > y->valor.real) -
               (x->valor.real < y->valor.real);
    case CAMPO_BOOLEANO:
        return (x->valor.booleano > y->valor.booleano) -
               (x->valor.booleano < y->valor.booleano);
    }
    return 0;
}

bool item_definir_campo(ITEM *item, size_t indice, const CAMPO *campo) {
    if (item == NULL || campo == NULL || indice >= item->esquema->quantidade ||
        campo->tipo != item->esquema->colunas[indice].tipo)
        return false;
    if (campo->ausente)
        return item_marcar_ausente(item, indice);
    CAMPO novo = *campo;
    if (novo.tipo == CAMPO_TEXTO) {
        if (novo.valor.texto == NULL)
            return false;
        /* Copiar antes de liberar permite autoatribuicao e preserva o
         * valor antigo em caso de falha de alocacao. */
        novo.valor.texto = copiar_texto(novo.valor.texto);
        if (novo.valor.texto == NULL)
            return false;
    }
    campo_limpar(&item->campos[indice]);
    item->campos[indice] = novo;
    free(item->original);
    item->original = NULL;
    return true;
}

bool item_marcar_ausente(ITEM *item, size_t indice) {
    if (item == NULL || indice >= item->esquema->quantidade)
        return false;
    campo_limpar(&item->campos[indice]);
    free(item->original);
    item->original = NULL;
    return true;
}

bool item_definir_original(ITEM *item, const char *original) {
    if (item == NULL) return false;
    char *copia = original != NULL ? copiar_texto(original) : NULL;
    if (original != NULL && copia == NULL) return false;
    free(item->original);
    item->original = copia;
    return true;
}

const char *item_original(const ITEM *item) {
    return item != NULL ? item->original : NULL;

}
