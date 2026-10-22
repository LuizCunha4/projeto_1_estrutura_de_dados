/* Executar da raiz:
 * cc -std=c11 -Wall -Wextra -pedantic -Ilib tests/test_reader.c
 * src/reader.c src/reader_nasa.c src/reader_empresas.c src/item.c src/csv.c
 * -o /tmp/test_reader && /tmp/test_reader
 */
#include "reader.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static const char nasa[] =
    "asteroid_designation,asteroid_fullname,close_approach_date,year,month,"
    "distance_au,velocity_km_s,absolute_magnitude,days_until_approach,"
    "is_past_event,is_future_event,risk_score,panic_level,threat_category,"
    "panic_verdict,on_sentry_list,sentry_impact_prob,sentry_torino_scale,"
    "sentry_palermo_scale,sentry_diameter_km\r\n";
static const char empresas[] =
    "dt_referencia,dt_geracao,cod_ibge,nome,Empresas Abertas,Empresas Fechadas,"
    "Empresas Ativas,Tempo Médio de Abertura,,dt_referencia,dt_geracao,cod_ibge,"
    "Estado,Empresas Abertas,Empresas Fechadas,Empresas Ativas,Tempo Médio de Abertura\n";

static FILE *entrada(const char *cabecalho, const char *linha) {
    FILE *f = tmpfile();
    assert(f);
    assert(fputs(cabecalho, f) >= 0 && fputs(linha, f) >= 0);
    rewind(f);
    return f;
}

static void igual(FILE *f, const char *esperado) {
    rewind(f);
    for (const unsigned char *s = (const unsigned char *)esperado; *s; ++s)
        assert(fgetc(f) == *s);
    assert(fgetc(f) == EOF);
}

static void teste_nasa(void) {
    const char *linha = "A,\"nome, com \"\"aspas\"\"\nsegunda linha\",data,2026,1,.1,1e1,,-2,True,False,0,0,SAFE,texto,False,,0.0,,";
    FILE *f = entrada(nasa, linha), *out = tmpfile();
    assert(out);
    READER *r = reader_nasa_criar(f);
    assert(r && reader_delimitador(r) == ',');
    assert(strcmp(reader_cabecalho(r), nasa) == 0);
    ITEM *item = NULL;
    assert(reader_ler(r, &item) == READER_OK);
    assert(item_campo(item, 3)->valor.inteiro == 2026);
    assert(item_campo(item, 6)->valor.real == 10);
    assert(item_campo(item, 7)->ausente);
    assert(item_campo(item, 9)->valor.booleano);
    assert(!item_campo(item, 10)->valor.booleano);
    assert(strcmp(item_campo(item, 1)->valor.texto, "nome, com \"aspas\"\nsegunda linha") == 0);
    assert(reader_escrever_item(r, out, item));
    rewind(out);
    for (const char *s = linha; *s; ++s) assert(fgetc(out) == (unsigned char)*s);
    assert(fgetc(out) == '\n' && fgetc(out) == EOF);
    assert(reader_ler(r, &item) == READER_ERRO); /* Nao sobrescrever dono. */
    ITEM *fim = NULL;
    assert(reader_ler(r, &fim) == READER_FIM && !fim);
    assert(reader_ler(r, &fim) == READER_FIM);
    CAMPO errado = {.tipo = CAMPO_BOOLEANO, .valor.booleano = true};
    assert(!item_definir_campo(item, 0, &errado));
    assert(item_original(item)); /* Falha preserva original. */
    assert(item_definir_campo(item, 3, item_campo(item, 3)));
    assert(!item_original(item) && !reader_escrever_item(r, out, item));
    assert(item_definir_original(item, linha));
    assert(item_definir_original(item, item_original(item))); /* Autoatribuicao. */
    assert(item_marcar_ausente(item, 3) && !item_original(item));
    assert(item_definir_original(item, linha));
    assert(item_definir_original(item, NULL) && !item_original(item));
    item_apagar(&item);
    reader_apagar(&r);
    assert(!r && !item);
    rewind(f); /* Reader nao fecha FILE. */
    assert(fgetc(f) == 'a');
    fclose(f);
    fclose(out);
}

static void teste_empresas(void) {
    const char *linha = "7/31/2026,8/4/2026,001234,Municipio,0,2,30,-,,7/31/2026,8/4/2026,01,Estado,5,6,7,-\r\n";
    FILE *f = entrada(empresas, linha), *out = tmpfile();
    assert(out);
    READER *r = reader_empresas_criar(f);
    assert(r);
    size_t i;
    assert(esquema_quantidade(reader_esquema(r)) == 17);
    assert(esquema_buscar(reader_esquema(r), "estado.cod_ibge", &i) && i == 11);
    assert(esquema_buscar(reader_esquema(r), "_separador", &i) && i == 8);
    ITEM *item = NULL;
    assert(reader_ler(r, &item) == READER_OK);
    assert(strcmp(item_campo(item, 2)->valor.texto, "001234") == 0);
    assert(item_campo(item, 7)->ausente && item_campo(item, 16)->ausente);
    assert(!item_campo(item, 4)->ausente && item_campo(item, 4)->valor.inteiro == 0);
    assert(reader_escrever_cabecalho(r, out));
    igual(out, empresas);
    FILE *out2 = tmpfile();
    assert(out2 && reader_escrever_item(r, out2, item));
    igual(out2, linha);
    FILE *f2 = entrada(empresas, "");
    READER *r2 = reader_empresas_criar(f2);
    assert(r2 && !reader_escrever_item(r2, out, item));
    reader_apagar(&r2);
    fclose(f2);
    item_apagar(&item);
    reader_apagar(&r);
    fclose(out2);
    fclose(out);
    fclose(f);
}

static void rejeitar(const char *valor, size_t coluna) {
    const char *campos[] = {"A", "B", "data", "2026", "1", "0.1", "1", "2",
        "-3", "True", "False", "0", "0", "SAFE", "texto", "False", "", "", "", ""};
    FILE *f = entrada(nasa, "");
    assert(fseek(f, 0, SEEK_END) == 0);
    for (size_t i = 0; i < 20; ++i) {
        assert(fputs(i == coluna ? valor : campos[i], f) >= 0);
        assert(fputc(i == 19 ? '\n' : ',', f) != EOF);
    }
    rewind(f);
    READER *r = reader_nasa_criar(f);
    ITEM *item = NULL;
    assert(r && reader_ler(r, &item) == READER_ERRO && !item);
    assert(reader_ler(r, &item) == READER_ERRO);
    reader_apagar(&r);
    fclose(f);
}

static void teste_erros(void) {
    const char *reais[] = {"nan", "inf", "1e9999", "1e-9999", "1x", " 1", "1 ", "0x1p2", ".", "1e+"};
    for (size_t i = 0; i < sizeof(reais)/sizeof(reais[0]); ++i) rejeitar(reais[i], 5);
    rejeitar("999999999999999999999999999999", 3);
    rejeitar("-999999999999999999999999999999", 3);
    rejeitar("1.0", 3);
    rejeitar("true", 9);
    rejeitar("1", 15);
    const char *linhas[] = {"A,B\n", "\"incompleto", "\n"};
    for (size_t i = 0; i < 3; ++i) {
        FILE *f = entrada(nasa, linhas[i]);
        READER *r = reader_nasa_criar(f);
        ITEM *item = NULL;
        assert(r && reader_ler(r, &item) == READER_ERRO && !item);
        reader_apagar(&r);
        fclose(f);
    }
    FILE *f = entrada("cabecalho errado\n", "");
    assert(!reader_nasa_criar(f));
    fclose(f);
    f = entrada("", "");
    assert(!reader_empresas_criar(f));
    fclose(f);
    assert(!reader_nasa_criar(NULL));
    assert(reader_ler(NULL, NULL) == READER_ERRO);
    assert(!item_definir_original(NULL, "a") && !item_original(NULL));
    reader_apagar(NULL);
}

/* Leitura completa e round-trip byte a byte, sem guardar vetor do dataset. */
static void dataset(const char *caminho, READER *(*criar)(FILE *)) {
    FILE *f = fopen(caminho, "rb"), *out = tmpfile();
    assert(f && out);
    READER *r = criar(f);
    assert(r && reader_escrever_cabecalho(r, out));
    ITEM *item = NULL;
    READER_STATUS status;
    size_t quantidade = 0;
    while ((status = reader_ler(r, &item)) == READER_OK) {
        assert(reader_escrever_item(r, out, item));
        item_apagar(&item);
        ++quantidade;
    }
    assert(status == READER_FIM && quantidade > 0);
    rewind(f);
    rewind(out);
    int c;
    while ((c = fgetc(f)) != EOF) assert(c == fgetc(out));
    assert(fgetc(out) == EOF && !ferror(f) && !ferror(out));
    printf("%s: %zu registros, round-trip exato\n", caminho, quantidade);
    reader_apagar(&r);
    fclose(f);
    fclose(out);
}

int main(void) {
    teste_nasa();
    teste_empresas();
    teste_erros();
    dataset("data/asteroid_dataset_20251019.csv", reader_nasa_criar);
    dataset("data/07_ArquivoDados_Jul26 (Mapa de Empresas).csv", reader_empresas_criar);
    puts("test_reader: OK");
    return 0;
}
