#include "cliente.h"

#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static bool copiar_opcao(char *destino, size_t capacidade, const char *origem) {
    size_t n = strlen(origem);
    if (n >= capacidade)
        return false;
    memcpy(destino, origem, n + 1);
    return true;
}

execucao ler_argumentos(int argc, char *argv[]) {
    execucao opcoes = {0};
    for (int i = 1; i < argc; ++i) {
        char *valor;
        char *destino = NULL;
        size_t capacidade = 0;
        if (i + 1 >= argc)
            break;
        if (strcmp(argv[i], "--reader") == 0) {
            destino = opcoes.reader; capacidade = sizeof(opcoes.reader);
        } else if (strcmp(argv[i], "--input") == 0) {
            destino = opcoes.input; capacidade = sizeof(opcoes.input);
        } else if (strcmp(argv[i], "--base") == 0) {
            destino = opcoes.base; capacidade = sizeof(opcoes.base);
        } else if (strcmp(argv[i], "--result") == 0) {
            destino = opcoes.result; capacidade = sizeof(opcoes.result);
        } else if (strcmp(argv[i], "--query") == 0) {
            destino = opcoes.query; capacidade = sizeof(opcoes.query);
        } else if (strcmp(argv[i], "--output") == 0) {
            destino = opcoes.output; capacidade = sizeof(opcoes.output);
        } else if (strcmp(argv[i], "--top-k") == 0 ||
                   strcmp(argv[i], "--limit") == 0) {
            char *fim;
            errno = 0;
            long numero = strtol(argv[++i], &fim, 10);
            if (errno == 0 && *argv[i] != '\0' && *fim == '\0' &&
                numero >= 0 && numero <= 2147483647L) {
                if (strcmp(argv[i - 1], "--top-k") == 0)
                    opcoes.top_k = (int)numero;
                else
                    opcoes.limit = (int)numero;
            }
            continue;
        } else {
            continue;
        }
        valor = argv[++i];
        (void)copiar_opcao(destino, capacidade, valor);
    }
    return opcoes;
}
