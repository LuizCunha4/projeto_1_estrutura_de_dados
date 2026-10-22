/* Testes isolados: executar conforme README, sem a main do aplicativo. */
#include "csv.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static FILE *entrada(const char *texto, size_t tamanho) {
    FILE *f = tmpfile();
    assert(f && fwrite(texto, 1, tamanho, f) == tamanho);
    rewind(f);
    return f;
}

static void rejeitar(const char *texto, size_t tamanho) {
    FILE *f = entrada(texto, tamanho);
    CSV_REGISTRO *r = NULL;
    assert(csv_ler(f, ',', &r) == CSV_ERRO && r == NULL);
    assert(fclose(f) == 0);
}

int main(void) {
    const char *texto = "a,\"b,c\",\"aspa \"\" e\nlinha\",\r\n;fim;\rultimo";
    FILE *f = entrada(texto, strlen(texto));
    CSV_REGISTRO *r = NULL;
    assert(csv_ler(f, ',', &r) == CSV_OK && r->quantidade == 4);
    assert(strcmp(r->campos[1], "b,c") == 0);
    assert(strcmp(r->campos[2], "aspa \" e\nlinha") == 0);
    assert(strcmp(r->campos[3], "") == 0);
    FILE *out = tmpfile();
    assert(out && csv_escrever_original(out, r) && fflush(out) == 0);
    rewind(out);
    for (const unsigned char *p = (const unsigned char *)r->original; *p; ++p)
        assert(fgetc(out) == *p);
    assert(fgetc(out) == EOF && fclose(out) == 0);
    csv_apagar(&r);
    assert(csv_ler(f, ';', &r) == CSV_OK && r->quantidade == 3);
    assert(strcmp(r->campos[1], "fim") == 0);
    csv_apagar(&r);
    assert(csv_ler(f, ',', &r) == CSV_OK && strcmp(r->original, "ultimo") == 0);
    csv_apagar(&r);
    assert(csv_ler(f, ',', &r) == CSV_FIM && !r);
    assert(fclose(f) == 0);

    const char *invalidos[] = {"\"ab", "a\"b", "\"a\"x", "\"a\" ,b"};
    for (size_t i = 0; i < sizeof(invalidos) / sizeof(invalidos[0]); ++i)
        rejeitar(invalidos[i], strlen(invalidos[i]));
    rejeitar("a\0b", 3);
    assert(csv_ler(NULL, ',', &r) == CSV_ERRO);
    csv_apagar(NULL);

    /* Forca varias expansoes do buffer, sem limite fixo de linha. */
    size_t n = 100000;
    char *longo = malloc(n + 1);
    assert(longo);
    memset(longo, 'x', n);
    longo[n] = '\0';
    f = entrada(longo, n);
    assert(csv_ler(f, ',', &r) == CSV_OK && strlen(r->campos[0]) == n);
    assert(strcmp(r->original, longo) == 0);
    csv_apagar(&r);
    free(longo);
    assert(fclose(f) == 0);
    f = entrada("\n,", 2);
    assert(csv_ler(f, ',', &r) == CSV_OK && r->quantidade == 1);
    csv_apagar(&r);
    assert(csv_ler(f, ',', &r) == CSV_OK && r->quantidade == 2);
    csv_apagar(&r);
    assert(csv_ler(f, ',', &r) == CSV_FIM);
    assert(fclose(f) == 0);

    f = entrada("", 0);
    assert(csv_ler(f, ',', &r) == CSV_FIM && r == NULL);
    assert(csv_ler(f, ',', NULL) == CSV_ERRO);
    const char delimitadores_invalidos[] = {'\0', '"', '\r', '\n'};
    for (size_t i = 0; i < sizeof(delimitadores_invalidos); ++i)
        assert(csv_ler(f, delimitadores_invalidos[i], &r) == CSV_ERRO);
    assert(!csv_escrever_original(f, NULL));
    assert(!csv_escrever_original(NULL, NULL));
    assert(fclose(f) == 0);

    /* Muitos campos exercitam tambem o crescimento do vetor de ponteiros. */
    n = 20000;
    char *vazios = malloc(n);
    assert(vazios);
    memset(vazios, ',', n);
    f = entrada(vazios, n);
    free(vazios);
    assert(csv_ler(f, ',', &r) == CSV_OK && r->quantidade == n + 1);
    for (size_t i = 0; i < r->quantidade; ++i)
        assert(strcmp(r->campos[i], "") == 0);
    csv_apagar(&r);
    csv_apagar(&r);
    assert(fclose(f) == 0);

    /* /dev/full e especifico do Linux usado pelo projeto. Sem buffer, o erro
     * aparece no fwrite; com buffer, caberia ao chamador verificar fflush. */
    out = fopen("/dev/full", "wb");
    assert(out && setvbuf(out, NULL, _IONBF, 0) == 0);
    CSV_REGISTRO exemplo = {.original = "teste\n"};
    assert(!csv_escrever_original(out, &exemplo));
    fclose(out);
    puts("test_csv: OK");
    return 0;
}
