#ifndef ARGUMENTOS_H
#define ARGUMENTOS_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char reader[32];
    char input[256];
    char base[32];
    char result[32];
    char query[256];
    char output[256];
    int top_k;
    int limit;
} EXECUCAO;

bool argumentos_ler(int argc, char *argv[], EXECUCAO *opcoes,
                    char *erro, size_t tamanho_erro);

#endif
