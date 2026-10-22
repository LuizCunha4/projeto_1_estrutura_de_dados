#include "reader_privado.h"

READER *reader_empresas_criar(FILE *arquivo) {
    /* Os blocos municipal e estadual sao tabelas independentes, colocadas
     * lado a lado no CSV. Nao relacionar municipio e estado pela linha.
     * Nomes internos unicos nao modificam o cabecalho original de saida. */
    static const READER_COLUNA colunas[] = {
#define C(nome, tipo) {nome, nome, tipo, false}
#define E(nome, tipo) {nome, "estado." nome, tipo, false}
        C("dt_referencia", CAMPO_TEXTO), C("dt_geracao", CAMPO_TEXTO),
        C("cod_ibge", CAMPO_TEXTO), C("nome", CAMPO_TEXTO),
        C("Empresas Abertas", CAMPO_INTEIRO), C("Empresas Fechadas", CAMPO_INTEIRO),
        C("Empresas Ativas", CAMPO_INTEIRO),
        {"Tempo Médio de Abertura", "Tempo Médio de Abertura", CAMPO_REAL, true},
        {"", "_separador", CAMPO_TEXTO, false},
        E("dt_referencia", CAMPO_TEXTO), E("dt_geracao", CAMPO_TEXTO),
        E("cod_ibge", CAMPO_TEXTO), E("Estado", CAMPO_TEXTO),
        E("Empresas Abertas", CAMPO_INTEIRO), E("Empresas Fechadas", CAMPO_INTEIRO),
        E("Empresas Ativas", CAMPO_INTEIRO),
        {"Tempo Médio de Abertura", "estado.Tempo Médio de Abertura", CAMPO_REAL, true}
#undef C
#undef E
    };
    return reader_criar_interno(arquivo, ',', colunas,
                               sizeof(colunas) / sizeof(colunas[0]));
}
