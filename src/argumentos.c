#include "argumentos.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool copiar(char *destino, size_t tamanho, const char *origem) {
    size_t n = strlen(origem);
    if (n >= tamanho) return false;
    memcpy(destino, origem, n + 1);
    return true;
}

static bool numero_ler(const char *texto, int *numero) {
    char *fim;
    long valor;

    if (texto[0] == '\0') return false;
    for (const char *p = texto; *p != '\0'; ++p)
        if (*p < '0' || *p > '9') return false;

    errno = 0;
    valor = strtol(texto, &fim, 10);
    if (errno != 0 || *fim != '\0' || valor > 2147483647L) return false;
    *numero = (int)valor;
    return true;
}

static bool estrutura_valida(const char *nome) {
    static const char *nomes[] = {
        "pilha", "fila", "lista_seq", "lista_enc", "lista_cabeca",
        "lista_ord", "lista_gen", "lista_cruz"
    };

    for (size_t i = 0; i < sizeof(nomes) / sizeof(nomes[0]); ++i)
        if (strcmp(nome, nomes[i]) == 0) return true;
    return false;
}

static bool opcao_copiar(EXECUCAO *opcoes, const char *nome,
                         const char *valor) {
    if (strcmp(nome, "--reader") == 0)
        return copiar(opcoes->reader, sizeof(opcoes->reader), valor);
    if (strcmp(nome, "--input") == 0)
        return copiar(opcoes->input, sizeof(opcoes->input), valor);
    if (strcmp(nome, "--base") == 0)
        return copiar(opcoes->base, sizeof(opcoes->base), valor);
    if (strcmp(nome, "--result") == 0)
        return copiar(opcoes->result, sizeof(opcoes->result), valor);
    if (strcmp(nome, "--query") == 0)
        return copiar(opcoes->query, sizeof(opcoes->query), valor);
    return copiar(opcoes->output, sizeof(opcoes->output), valor);
}

bool argumentos_ler(int argc, char *argv[], EXECUCAO *opcoes,
                    char *erro, size_t tamanho_erro) {
    bool reader = false, input = false, base = false, result = false;
    bool query = false, output = false, top_k = false, limit = false;

    if (opcoes == NULL || erro == NULL || tamanho_erro == 0) return false;
    *opcoes = (EXECUCAO){0};
    erro[0] = '\0';

    for (int i = 1; i < argc; ++i) {
        const char *nome = argv[i];
        const char *valor;
        bool *visto = NULL;

        if (strcmp(nome, "--reader") == 0) visto = &reader;
        else if (strcmp(nome, "--input") == 0) visto = &input;
        else if (strcmp(nome, "--base") == 0) visto = &base;
        else if (strcmp(nome, "--result") == 0) visto = &result;
        else if (strcmp(nome, "--query") == 0) visto = &query;
        else if (strcmp(nome, "--output") == 0) visto = &output;
        else if (strcmp(nome, "--top-k") == 0) visto = &top_k;
        else if (strcmp(nome, "--limit") == 0) visto = &limit;
        else {
            snprintf(erro, tamanho_erro, "opção desconhecida: %s", nome);
            return false;
        }

        if (*visto) {
            snprintf(erro, tamanho_erro, "opção repetida: %s", nome);
            return false;
        }
        *visto = true;

        if (i + 1 >= argc || strncmp(argv[i + 1], "--", 2) == 0) {
            snprintf(erro, tamanho_erro, "valor ausente para %s", nome);
            return false;
        }
        valor = argv[++i];
        if (valor[0] == '\0') {
            snprintf(erro, tamanho_erro, "valor vazio para %s", nome);
            return false;
        }

        if (strcmp(nome, "--top-k") == 0 || strcmp(nome, "--limit") == 0) {
            int numero;
            if (!numero_ler(valor, &numero)) {
                snprintf(erro, tamanho_erro, "número inválido para %s: %s",
                         nome, valor);
                return false;
            }
            if (strcmp(nome, "--top-k") == 0) opcoes->top_k = numero;
            else opcoes->limit = numero;
        } else if (!opcao_copiar(opcoes, nome, valor)) {
            snprintf(erro, tamanho_erro, "valor longo demais para %s", nome);
            return false;
        }
    }

    if (!reader || !input || !base || !result || !output) {
        snprintf(erro, tamanho_erro,
                 "use --reader, --input, --base, --result e --output");
        return false;
    }
    if (strcmp(opcoes->reader, "nasa") != 0 &&
        strcmp(opcoes->reader, "empresas") != 0) {
        snprintf(erro, tamanho_erro, "reader deve ser nasa ou empresas");
        return false;
    }
    if (!estrutura_valida(opcoes->base) || !estrutura_valida(opcoes->result)) {
        snprintf(erro, tamanho_erro, "estrutura base ou result inválida");
        return false;
    }
    if (strcmp(opcoes->input, opcoes->output) == 0 ||
        (opcoes->query[0] != '\0' &&
         strcmp(opcoes->query, opcoes->output) == 0)) {
        snprintf(erro, tamanho_erro,
                 "output deve ser diferente de input e query");
        return false;
    }
    return true;
}
