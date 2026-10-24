#include "estrutura.h"

#include "fila.h"
#include "lista_cabeca.h"
#include "lista_enc.h"
#include "lista_gen.h"
#include "lista_ord.h"
#include "lista_seq.h"
#include "pilha.h"

#include <stdlib.h>
#include <string.h>

typedef enum {
    T_PILHA, T_FILA, T_SEQ, T_ENC, T_CABECA, T_ORD, T_GEN, T_CRUZ
} TIPO_TAD;

struct estrutura {
    TIPO_TAD tipo;
    ESTRUTURA_PAPEL papel;
    const ESQUEMA *esquema;
    const ESQUEMA *esquema_query;
    union {
        PILHA *pilha;
        FILA *fila;
        LISTA_SEQ *seq;
        LISTA_ENC *enc;
        LISTA_CABECA *cabeca;
        LISTA_ORD *ord;
        LISTA_GEN *gen;
        LISTA_CRUZ *cruz;
    } dado;
};

static const INSTRUCAO *buscar_instrucao(const QUERY *query,
                                         TIPO_INSTRUCAO tipo) {
    size_t quantidade;
    const INSTRUCAO *instrucoes = query_instrucoes(query, &quantidade);

    for (size_t i = 0; i < quantidade; ++i)
        if (instrucoes[i].tipo == tipo) return &instrucoes[i];
    return NULL;
}

static bool estrutura_criada(const ESTRUTURA *estrutura) {
    switch (estrutura->tipo) {
    case T_PILHA: return estrutura->dado.pilha != NULL;
    case T_FILA: return estrutura->dado.fila != NULL;
    case T_SEQ: return estrutura->dado.seq != NULL;
    case T_ENC: return estrutura->dado.enc != NULL;
    case T_CABECA: return estrutura->dado.cabeca != NULL;
    case T_ORD: return estrutura->dado.ord != NULL;
    case T_GEN: return estrutura->dado.gen != NULL;
    case T_CRUZ: return estrutura->dado.cruz != NULL;
    }
    return false;
}

ESTRUTURA *estrutura_criar(const char *nome, ESTRUTURA_PAPEL papel,
                           const QUERY *query) {
    ESTRUTURA *estrutura;
    const INSTRUCAO *instrucao;

    if (nome == NULL ||
        (papel != ESTRUTURA_BASE && papel != ESTRUTURA_RESULTADOS))
        return NULL;

    estrutura = calloc(1, sizeof(*estrutura));
    if (estrutura == NULL) return NULL;
    estrutura->papel = papel;
    estrutura->esquema_query = query_esquema(query);

    if (strcmp(nome, "pilha") == 0) {
        estrutura->tipo = T_PILHA;
        estrutura->dado.pilha = pilha_criar();
    } else if (strcmp(nome, "fila") == 0) {
        estrutura->tipo = T_FILA;
        estrutura->dado.fila = fila_criar();
    } else if (strcmp(nome, "lista_seq") == 0) {
        estrutura->tipo = T_SEQ;
        estrutura->dado.seq = lista_seq_criar();
    } else if (strcmp(nome, "lista_enc") == 0) {
        estrutura->tipo = T_ENC;
        estrutura->dado.enc = lista_enc_criar(false);
    } else if (strcmp(nome, "lista_cabeca") == 0) {
        estrutura->tipo = T_CABECA;
        estrutura->dado.cabeca = lista_cabeca_criar();
    } else if (strcmp(nome, "lista_ord") == 0) {
        estrutura->tipo = T_ORD;
        estrutura->dado.ord = lista_ord_criar();
        instrucao = buscar_instrucao(query, ORDER);
        if (estrutura->dado.ord != NULL && instrucao != NULL &&
            !lista_ord_ordenar_comparador(
                estrutura->dado.ord, query_comparador_ordenacao,
                (void *)query))
            lista_ord_apagar(&estrutura->dado.ord);
    } else if (strcmp(nome, "lista_gen") == 0) {
        const size_t campos_padrao[] = {0, 1};
        const size_t *campos;
        size_t quantidade;

        estrutura->tipo = T_GEN;
        estrutura->dado.gen = lista_gen_criar();
        instrucao = buscar_instrucao(query, GROUP);
        campos = instrucao != NULL ? instrucao->indices_campos : campos_padrao;
        quantidade = instrucao != NULL ? instrucao->quantidade_campos : 2;
        if (estrutura->dado.gen != NULL &&
            !lista_gen_configurar_grupos(estrutura->dado.gen, campos,
                                         quantidade))
            lista_gen_apagar(&estrutura->dado.gen);
        if (papel == ESTRUTURA_RESULTADOS && instrucao == NULL)
            lista_gen_apagar(&estrutura->dado.gen);
    } else if (strcmp(nome, "lista_cruz") == 0) {
        estrutura->tipo = T_CRUZ;
        instrucao = buscar_instrucao(query, CROSS);
        if (instrucao != NULL && instrucao->quantidade_campos == 2) {
            estrutura->dado.cruz = lista_cruz_criar(
                instrucao->indices_campos[0], instrucao->indices_campos[1]);
        } else if (papel == ESTRUTURA_BASE) {
            estrutura->dado.cruz = lista_cruz_criar(0, 1);
        }
    } else {
        free(estrutura);
        return NULL;
    }

    if (!estrutura_criada(estrutura)) {
        free(estrutura);
        return NULL;
    }
    return estrutura;
}

bool estrutura_inserir(ESTRUTURA *estrutura, ITEM *item) {
    const ESQUEMA *esquema;
    bool inserido = false;

    if (estrutura == NULL || item == NULL) return false;
    esquema = item_esquema(item);
    if ((estrutura->esquema != NULL && estrutura->esquema != esquema) ||
        (estrutura->esquema_query != NULL &&
         estrutura->esquema_query != esquema)) return false;
    if ((estrutura->tipo == T_GEN || estrutura->tipo == T_CRUZ) &&
        esquema_quantidade(esquema) < 2) return false;
    if (estrutura->papel == ESTRUTURA_BASE &&
        !item_reservar_base(item, estrutura)) return false;

    switch (estrutura->tipo) {
    case T_PILHA: inserido = pilha_empilhar(estrutura->dado.pilha, item); break;
    case T_FILA: inserido = fila_inserir(estrutura->dado.fila, item); break;
    case T_SEQ: inserido = lista_seq_inserir(estrutura->dado.seq, item); break;
    case T_ENC: inserido = lista_enc_inserir(estrutura->dado.enc, item); break;
    case T_CABECA: inserido = lista_cabeca_inserir(estrutura->dado.cabeca, item); break;
    case T_ORD: inserido = lista_ord_inserir(estrutura->dado.ord, item); break;
    case T_GEN: inserido = lista_gen_inserir_atomo(estrutura->dado.gen, item); break;
    case T_CRUZ: inserido = lista_cruz_inserir(estrutura->dado.cruz, item); break;
    }

    if (estrutura->papel == ESTRUTURA_BASE && !inserido)
        item_liberar_base(item, estrutura);
    if (inserido && estrutura->esquema == NULL)
        estrutura->esquema = esquema;
    return inserido;
}

void estrutura_percorrer(const ESTRUTURA *estrutura,
                         TAD_VISITANTE visitante, void *contexto) {
    if (estrutura == NULL || visitante == NULL) return;
    switch (estrutura->tipo) {
    case T_PILHA: pilha_percorrer(estrutura->dado.pilha, visitante, contexto); break;
    case T_FILA: fila_percorrer(estrutura->dado.fila, visitante, contexto); break;
    case T_SEQ: lista_seq_percorrer(estrutura->dado.seq, visitante, contexto); break;
    case T_ENC: lista_enc_percorrer(estrutura->dado.enc, visitante, contexto); break;
    case T_CABECA: lista_cabeca_percorrer(estrutura->dado.cabeca, visitante, contexto); break;
    case T_ORD: lista_ord_percorrer(estrutura->dado.ord, visitante, contexto); break;
    case T_GEN: lista_gen_percorrer(estrutura->dado.gen, visitante, contexto); break;
    case T_CRUZ:
        lista_cruz_percorrer(estrutura->dado.cruz, CRUZ_POR_LINHA,
                             visitante, contexto);
        break;
    }
}

void estrutura_percorrer_eixo(const ESTRUTURA *estrutura,
                              LISTA_CRUZ_EIXO eixo,
                              TAD_VISITANTE visitante, void *contexto) {
    if (estrutura == NULL || visitante == NULL) return;
    if (estrutura->tipo == T_CRUZ)
        lista_cruz_percorrer(estrutura->dado.cruz, eixo, visitante, contexto);
    else
        estrutura_percorrer(estrutura, visitante, contexto);
}

ITEM *estrutura_remover(ESTRUTURA *estrutura, ITEM *item) {
    ITEM *removido = NULL;

    if (estrutura == NULL || item == NULL) return NULL;
    switch (estrutura->tipo) {
    case T_PILHA: removido = pilha_remover_item(estrutura->dado.pilha, item); break;
    case T_FILA: removido = fila_remover_item(estrutura->dado.fila, item); break;
    case T_SEQ: removido = lista_seq_remover_item(estrutura->dado.seq, item); break;
    case T_ENC: removido = lista_enc_remover(estrutura->dado.enc, item); break;
    case T_CABECA: removido = lista_cabeca_remover(estrutura->dado.cabeca, item); break;
    case T_ORD: removido = lista_ord_remover_item(estrutura->dado.ord, item); break;
    case T_GEN: removido = lista_gen_remover_item(estrutura->dado.gen, item); break;
    case T_CRUZ: removido = lista_cruz_remover(estrutura->dado.cruz, item); break;
    }
    if (removido != NULL && estrutura->papel == ESTRUTURA_BASE)
        item_liberar_base(removido, estrutura);
    return removido;
}

static void contar_item(ITEM *item, void *contexto) {
    (void)item;
    ++*(size_t *)contexto;
}

size_t estrutura_tamanho(const ESTRUTURA *estrutura) {
    size_t quantidade = 0;
    estrutura_percorrer(estrutura, contar_item, &quantidade);
    return quantidade;
}

static void liberar_item(ITEM *item, void *contexto) {
    (void)contexto;
    item_apagar(&item);
}

void estrutura_apagar(ESTRUTURA **ponteiro) {
    ESTRUTURA *estrutura;

    if (ponteiro == NULL || *ponteiro == NULL) return;
    estrutura = *ponteiro;
    if (estrutura->papel == ESTRUTURA_BASE)
        estrutura_percorrer(estrutura, liberar_item, NULL);

    switch (estrutura->tipo) {
    case T_PILHA: pilha_apagar(&estrutura->dado.pilha); break;
    case T_FILA: fila_apagar(&estrutura->dado.fila); break;
    case T_SEQ: lista_seq_apagar(&estrutura->dado.seq); break;
    case T_ENC: lista_enc_apagar(&estrutura->dado.enc); break;
    case T_CABECA: lista_cabeca_apagar(&estrutura->dado.cabeca); break;
    case T_ORD: lista_ord_apagar(&estrutura->dado.ord); break;
    case T_GEN: lista_gen_apagar(&estrutura->dado.gen); break;
    case T_CRUZ: lista_cruz_apagar(&estrutura->dado.cruz); break;
    }
    free(estrutura);
    *ponteiro = NULL;
}
