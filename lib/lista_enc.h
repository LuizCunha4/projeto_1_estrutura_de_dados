#ifndef LISTA_ENC_H
#define LISTA_ENC_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_enc LISTA_ENC;

LISTA_ENC *lista_enc_criar(bool ordenada);
void lista_enc_apagar(LISTA_ENC **lista);
bool lista_enc_inserir(LISTA_ENC *lista, ITEM *item);
ITEM *lista_enc_busca(const LISTA_ENC *lista, int chave);
ITEM *lista_enc_remover(LISTA_ENC *lista, int chave);
int lista_enc_tamanho(const LISTA_ENC *lista);
bool lista_enc_vazia(const LISTA_ENC *lista);
bool lista_enc_cheia(const LISTA_ENC *lista);
void lista_enc_imprimir(const LISTA_ENC *lista);
bool lista_enc_inserir_inicio(LISTA_ENC *lista, ITEM *item);
bool lista_enc_inserir_fim(LISTA_ENC *lista, ITEM *item);
void lista_enc_imprimir_reversa(const LISTA_ENC *lista);

#endif

