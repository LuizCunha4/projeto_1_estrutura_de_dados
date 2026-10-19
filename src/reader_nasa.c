#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "item.h"
#include "reader.h"

char* extrair_campo(char **cursor) {
    if (*cursor == NULL) return NULL;

    char *inicio = *cursor;
    char *virgula = strchr(inicio, ',');

    if (virgula != NULL) {
        *virgula = '\0';        
        *cursor = virgula + 1;  
    } else {
        *cursor = NULL;         
    }

    return inicio;
}

static int converter_int(const char *str) {
    return (str != NULL && *str != '\0') ? atoi(str) : 0;
}

static double converter_double(const char *str) {
    return (str != NULL && *str != '\0') ? atof(str) : 0.0;
}

static ITEM* linha_em_item(FILE* arquivo){
    if (arquivo == NULL){
        return NULL;
    }
    char linha[1000];

if (fgets(linha, sizeof(linha), arquivo) != NULL) {
        
        linha[strcspn(linha, "\r\n")] = '\0';

        if (strlen(linha) == 0) {
            return NULL;
        }

        char *cursor = linha;

        char *str_designation = extrair_campo(&cursor);
        char *str_fullname = extrair_campo(&cursor);
        char *str_approach_date = extrair_campo(&cursor);
        char *str_year = extrair_campo(&cursor);
        char *str_month = extrair_campo(&cursor);
        char *str_distance_au = extrair_campo(&cursor);
        char *str_velocity_km_s = extrair_campo(&cursor);
        char *str_abs_mag = extrair_campo(&cursor);
        char *str_days_until = extrair_campo(&cursor);
        char *str_is_past = extrair_campo(&cursor);
        char *str_is_future = extrair_campo(&cursor);
        char *str_risk_score = extrair_campo(&cursor);
        char *str_panic_level = extrair_campo(&cursor);
        char *str_threat_cat = extrair_campo(&cursor);
        char *str_panic_verdict = extrair_campo(&cursor);
        char *str_on_sentry = extrair_campo(&cursor);
        char *str_sentry_prob = extrair_campo(&cursor);
        char *str_sentry_torino = extrair_campo(&cursor);
        char *str_sentry_palermo = extrair_campo(&cursor);
        char *str_sentry_diameter = extrair_campo(&cursor);

        int year = converter_int(str_year);
        int month = converter_int(str_month);
        double distance_au = converter_double(str_distance_au);
        double velocity_km_s = converter_double(str_velocity_km_s);
        double abs_mag  = converter_double(str_abs_mag);
        int days_until = converter_int(str_days_until);
        int is_past = converter_int(str_is_past);
        int is_future = converter_int(str_is_future);
        double risk_score = converter_double(str_risk_score);
        int panic_level = converter_int(str_panic_level);
        int on_sentry = converter_int(str_on_sentry);
        double sentry_prob = converter_double(str_sentry_prob);
        double sentry_torino = converter_double(str_sentry_torino);
        double sentry_palermo = converter_double(str_sentry_palermo);
        double sentry_diameter = converter_double(str_sentry_diameter);

        ITEM* item = item_criar(str_designation, str_fullname, str_approach_date, year, month, distance_au,
                                velocity_km_s, abs_mag, days_until, is_past, is_future, risk_score, panic_level, str_threat_cat,
                                str_panic_verdict, on_sentry, sentry_prob, sentry_torino, sentry_palermo, sentry_diameter);
        return item;
    }

    return NULL;
}

ITEM** arquivo_em_itens(const char* caminho_arquivo, int* total) {
    FILE *arquivo = fopen(caminho_arquivo, "r");
    if (arquivo == NULL) return NULL;

    char linha1[1000];
    fgets(linha1, sizeof(linha1), arquivo);

    int capacidade = 100;
    int qtd = 0;
    ITEM **itens = malloc(capacidade * sizeof(ITEM*));

    ITEM *novo_item = NULL;
    while ((novo_item = linha_em_item(arquivo)) != NULL) {
        if (qtd == capacidade) {
            capacidade *= 2;
            itens = realloc(itens, capacidade * sizeof(ITEM*));
        }

        itens[qtd] = novo_item;
        qtd++;
    }

    fclose(arquivo);
    *total = qtd;

    return itens;
}
