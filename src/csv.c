#include "csv.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *dados;
    size_t tamanho;
    size_t capacidade;
} TEXTO;

/* Crescimento geometrico evita realocar a cada byte. O limite e apenas o
 * tamanho representavel por size_t e a memoria disponivel. */
static bool acrescentar(TEXTO *texto, int byte) {
    if (texto->tamanho > SIZE_MAX - 2)
        return false;
    size_t necessario = texto->tamanho + 2;
    if (necessario > texto->capacidade) {
        size_t capacidade = texto->capacidade ? texto->capacidade : 64;
        while (capacidade < necessario) {
            if (capacidade > SIZE_MAX / 2) {
                capacidade = necessario;
                break;
            }
            capacidade *= 2;
        }
        char *novo = realloc(texto->dados, capacidade);
        if (novo == NULL)
            return false;
        texto->dados = novo;
        texto->capacidade = capacidade;
    }
    texto->dados[texto->tamanho++] = (char)byte;
    texto->dados[texto->tamanho] = '\0';
    return true;
}

static bool guardar_campo(CSV_REGISTRO *registro, TEXTO *campo,
                          size_t *capacidade) {
    if (registro->quantidade == *capacidade) {
        size_t limite = SIZE_MAX / sizeof(*registro->campos);
        if (*capacidade == limite)
            return false;
        size_t nova = *capacidade == 0 ? 8 :
                      (*capacidade > limite / 2 ? limite : *capacidade * 2);
        if (nova > limite)
            nova = limite;
        char **campos = realloc(registro->campos, nova * sizeof(*campos));
        if (campos == NULL)
            return false;
        registro->campos = campos;
        *capacidade = nova;
    }
    if (campo->dados == NULL && !acrescentar(campo, '\0'))
        return false;
    /* Transfere propriedade: o temporario deixa de possuir este texto. */
    registro->campos[registro->quantidade++] = campo->dados;
    *campo = (TEXTO){0};
    return true;
}

void csv_apagar(CSV_REGISTRO **registro) {
    if (registro == NULL || *registro == NULL)
        return;
    for (size_t i = 0; i < (*registro)->quantidade; ++i)
        free((*registro)->campos[i]);
    free((*registro)->campos);
    free((*registro)->original);
    free(*registro);
    *registro = NULL;
}

CSV_STATUS csv_ler(FILE *arquivo, char delimitador, CSV_REGISTRO **saida) {
    if (saida == NULL)
        return CSV_ERRO;
    *saida = NULL;
    if (arquivo == NULL || delimitador == '\0' || delimitador == '"' ||
        delimitador == '\r' || delimitador == '\n' || ferror(arquivo))
        return CSV_ERRO;

    CSV_REGISTRO *registro = calloc(1, sizeof(*registro));
    if (registro == NULL)
        return CSV_ERRO;
    TEXTO original = {0}, campo = {0};
    size_t capacidade = 0;
    enum { INICIO, SEM_ASPAS, ENTRE_ASPAS, APOS_ASPAS } estado = INICIO;
    CSV_STATUS status = CSV_ERRO;

    for (;;) {
        int byte = fgetc(arquivo);
        if (ferror(arquivo))
            goto terminar;
        if (byte == EOF) {
            if (estado == ENTRE_ASPAS)
                goto terminar;
            if (original.tamanho == 0) {
                status = CSV_FIM;
                goto terminar;
            }
            break;
        }
        if (byte == '\0' || !acrescentar(&original, byte))
            goto terminar;

        if (estado == ENTRE_ASPAS) {
            if (byte == '"')
                estado = APOS_ASPAS;
            else if (!acrescentar(&campo, byte))
                goto terminar;
        } else if (estado == APOS_ASPAS && byte == '"') {
            if (!acrescentar(&campo, byte))
                goto terminar;
            estado = ENTRE_ASPAS;
        } else if (byte == (unsigned char)delimitador) {
            if (!guardar_campo(registro, &campo, &capacidade))
                goto terminar;
            estado = INICIO;
        } else if (byte == '\n' || byte == '\r') {
            if (byte == '\r') {
                /* Reconhece CRLF sem consumir o primeiro byte da proxima
                 * linha. A biblioteca C garante um byte de pushback. */
                int proximo = fgetc(arquivo);
                if (ferror(arquivo))
                    goto terminar;
                if (proximo == '\n') {
                    if (!acrescentar(&original, proximo))
                        goto terminar;
                } else if (proximo != EOF && ungetc(proximo, arquivo) == EOF) {
                    goto terminar;
                }
            }
            break;
        } else if (estado == INICIO && byte == '"') {
            estado = ENTRE_ASPAS;
        } else {
            if (estado == APOS_ASPAS || byte == '"' ||
                !acrescentar(&campo, byte))
                goto terminar;
            estado = SEM_ASPAS;
        }
    }
    if (!guardar_campo(registro, &campo, &capacidade))
        goto terminar;
    registro->original = original.dados;
    original.dados = NULL;
    *saida = registro;
    registro = NULL;
    status = CSV_OK;

terminar:
    /* Cada alocacao tem um unico dono, inclusive em leituras incompletas. */
    free(original.dados);
    free(campo.dados);
    csv_apagar(&registro);
    return status;
}

bool csv_escrever_original(FILE *arquivo, const CSV_REGISTRO *registro) {
    if (arquivo == NULL || registro == NULL || registro->original == NULL ||
        ferror(arquivo))
        return false;
    size_t tamanho = strlen(registro->original);
    if (fwrite(registro->original, 1, tamanho, arquivo) != tamanho)
        return false;
    return !ferror(arquivo);
}
