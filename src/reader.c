#include "reader_privado.h"
#include "csv.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct reader {
    FILE *arquivo;
    char delimitador;
    ESQUEMA *esquema;
    CSV_REGISTRO *cabecalho;
    const READER_COLUNA *descritores; /* Descritores estaticos do leitor. */
    size_t quantidade;
    READER_STATUS estado;
};

READER *reader_criar_interno(FILE *arquivo, char delimitador,
                           const READER_COLUNA *descritores, size_t quantidade) {
    if (!arquivo || !descritores || !quantidade || quantidade > SIZE_MAX / sizeof(COLUNA))
        return NULL;
    READER *r = calloc(1, sizeof(*r));
    if (!r) return NULL;
    r->arquivo = arquivo;
    r->delimitador = delimitador;
    r->descritores = descritores;
    r->quantidade = quantidade;
    r->estado = READER_OK;
    if (csv_ler(arquivo, delimitador, &r->cabecalho) != CSV_OK)
        goto falha;
    if (r->cabecalho->quantidade != quantidade) goto falha;
    COLUNA *temporarias = malloc(quantidade * sizeof(*temporarias));
    if (!temporarias) goto falha;
    for (size_t i = 0; i < quantidade; ++i) {
        if (strcmp(r->cabecalho->campos[i], descritores[i].entrada) != 0) {
            free(temporarias);
            goto falha;
        }
        temporarias[i] = (COLUNA){descritores[i].nome, descritores[i].tipo};
    }
    r->esquema = esquema_criar(temporarias, quantidade);
    free(temporarias); /* esquema_criar copia os nomes. */
    if (!r->esquema) goto falha;
    return r;
falha:
    reader_apagar(&r);
    return NULL;
}

/* Gramatica decimal explicita: recusa espacos, hexadecimais, NaN/Inf e
 * sufixos. strtod exige LC_NUMERIC="C" (padrao ao iniciar um programa C).
 * Nao mudamos o locale global do aplicativo. ERANGE inclui underflow. */
static bool decimal(const char *s, bool real) {
    if (*s == '+' || *s == '-') ++s;
    size_t digitos = 0;
    while (*s >= '0' && *s <= '9') { ++s; ++digitos; }
    if (real && *s == '.') {
        ++s;
        while (*s >= '0' && *s <= '9') { ++s; ++digitos; }
    }
    if (!digitos) return false;
    if (real && (*s == 'e' || *s == 'E')) {
        ++s;
        if (*s == '+' || *s == '-') ++s;
        const char *inicio = s;
        while (*s >= '0' && *s <= '9') ++s;
        if (s == inicio) return false;
    }
    return *s == '\0';
}

static bool converter(const char *s, const READER_COLUNA *coluna, CAMPO *c) {
    *c = (CAMPO){.tipo = coluna->tipo, .ausente = false};
    if (!*s || (coluna->hifen_ausente && strcmp(s, "-") == 0)) {
        c->ausente = true;
        return true;
    }
    char *fim;
    switch (c->tipo) {
    case CAMPO_TEXTO: c->valor.texto = s; return true;
    case CAMPO_BOOLEANO:
        if (strcmp(s, "True") == 0) c->valor.booleano = true;
        else if (strcmp(s, "False") == 0) c->valor.booleano = false;
        else return false;
        return true;
    case CAMPO_INTEIRO:
        if (!decimal(s, false)) return false;
        errno = 0;
        c->valor.inteiro = strtoll(s, &fim, 10);
        return errno == 0 && *fim == '\0';
    case CAMPO_REAL:
        if (!decimal(s, true)) return false;
        errno = 0;
        c->valor.real = strtod(s, &fim);
        return errno == 0 && *fim == '\0' && isfinite(c->valor.real);
    }
    return false;
}

READER_STATUS reader_ler(READER *r, ITEM **saida) {
    if (!r || !saida || *saida) return READER_ERRO;
    if (r->estado != READER_OK) return r->estado;
    CSV_REGISTRO *csv = NULL;
    ITEM *item = NULL;
    CSV_STATUS status = csv_ler(r->arquivo, r->delimitador, &csv);
    /* Separar EOF de erro evita confundir registro malformado com fim. */
    if (status == CSV_FIM) {
        csv_apagar(&csv);
        return r->estado = READER_FIM;
    }
    if (status != CSV_OK || csv->quantidade != r->quantidade) goto falha;
    item = item_criar(r->esquema);
    if (!item) goto falha;
    for (size_t i = 0; i < r->quantidade; ++i) {
        /* Converter segundo o descritor; ITEM copia os textos temporarios. */
        CAMPO campo;
        if (!converter(csv->campos[i], &r->descritores[i], &campo) ||
            !item_definir_campo(item, i, &campo)) goto falha;
    }
    if (!item_definir_original(item, csv->original)) goto falha;
    csv_apagar(&csv);
    *saida = item; /* Somente aqui a propriedade sai desta funcao. */
    return READER_OK;
falha:
    item_apagar(&item);
    csv_apagar(&csv);
    return r->estado = READER_ERRO;
}

const ESQUEMA *reader_esquema(const READER *r) { return r ? r->esquema : NULL; }
const char *reader_cabecalho(const READER *r) {
    return r ? r->cabecalho->original : NULL;
}
char reader_delimitador(const READER *r) { return r ? r->delimitador : '\0'; }

bool reader_escrever_cabecalho(const READER *r, FILE *saida) {
    return r && saida && csv_escrever_original(saida, r->cabecalho);
}

bool reader_escrever_item(const READER *r, FILE *saida, const ITEM *item) {
    if (!r || !saida || !item || item_esquema(item) != r->esquema) return false;
    const char *original = item_original(item);
    if (!original || ferror(saida) || fputs(original, saida) == EOF) return false;
    /* Um registro que era o ultimo pode ficar no meio apos a ordenacao. */
    size_t tamanho = strlen(original);
    if (tamanho && original[tamanho - 1] != '\n' && original[tamanho - 1] != '\r')
        if (fputc('\n', saida) == EOF) return false;
    return !ferror(saida);
}

void reader_apagar(READER **reader) {
    if (!reader || !*reader) return;
    READER *r = *reader;
    csv_apagar(&r->cabecalho);
    esquema_apagar(&r->esquema);
    free(r);
    *reader = NULL;
}
