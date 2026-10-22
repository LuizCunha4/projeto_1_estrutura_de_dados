#ifndef CLIENTE_H
#define CLIENTE_H

typedef struct {
    char reader[32];
    char input[256];
    char base[32];
    char result[32];
    char query[256];
    char output[256];
    int top_k;
    int limit;
} execucao;

execucao ler_argumentos(int argc, char *argv[]);

#endif
