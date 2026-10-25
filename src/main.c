/* Disponibiliza clock_gettime, como no exemplo POSIX do professor. */
#define _POSIX_C_SOURCE 200809L

#include "argumentos.h"

#include "estrutura.h"
#include "reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    const QUERY *consulta;
    ESTRUTURA *resultados;
    size_t examinados;
    size_t aceitos;
    bool erro;
    double tempo_busca;
    double tempo_insercao;
} BUSCA;

typedef struct {
    const READER *reader;
    FILE *arquivo;
    size_t limite;
    size_t escritos;
    bool erro;
} EXPORTACAO;

/* Relógio monotônico: mede tempo decorrido sem sofrer ajustes do relógio civil.
 * Convertemos os nanossegundos para segundos; durações são fim - início. */
static double agora(void) {
    struct timespec tempo;
    clock_gettime(CLOCK_MONOTONIC, &tempo);
    return (double)tempo.tv_sec + (double)tempo.tv_nsec / 1e9;
}

static void buscar_item(ITEM *item, void *contexto) {
    BUSCA *busca = contexto;
    bool aceito = busca->consulta == NULL ||
                  query_item_aceito(busca->consulta, item);
    ++busca->examinados;

    if (!aceito) return;
    ++busca->aceitos;
    if (busca->erro) return;

    /* A inserção acontece durante a busca, mas tem uma métrica separada. */
    double inicio = agora();
    if (!estrutura_inserir(busca->resultados, item)) busca->erro = true;
    busca->tempo_insercao += agora() - inicio;
}

static void exportar_item(ITEM *item, void *contexto) {
    EXPORTACAO *exportacao = contexto;
    if (exportacao->erro) return;
    if (exportacao->limite != 0 &&
        exportacao->escritos >= exportacao->limite) return;
    if (!reader_escrever_item(exportacao->reader, exportacao->arquivo, item)) {
        exportacao->erro = true;
        return;
    }
    ++exportacao->escritos;
}

static void liberar(FILE **entrada, FILE **arquivo_consulta, FILE **saida,
                    READER **reader, QUERY **consulta,
                    ESTRUTURA **base, ESTRUTURA **resultados) {
    estrutura_apagar(resultados);
    estrutura_apagar(base);
    query_liberar(consulta);
    reader_apagar(reader);
    if (*entrada != NULL) fclose(*entrada);
    if (*arquivo_consulta != NULL) fclose(*arquivo_consulta);
    if (*saida != NULL) fclose(*saida);
}

int main(int argc, char *argv[]) {
    EXECUCAO configuracao;
    const EXECUCAO *opcoes = &configuracao;
    char erro[256];
    FILE *entrada = NULL, *arquivo_consulta = NULL, *saida = NULL;
    READER *reader = NULL;
    QUERY *consulta = NULL;
    ESTRUTURA *base = NULL, *resultados = NULL;
    size_t carregados = 0;
    double tempo_carga = 0.0;
    double tempo_escrita = 0.0;
    BUSCA busca = {0};
    EXPORTACAO exportacao = {0};
    int status = EXIT_FAILURE;

    if (!argumentos_ler(argc, argv, &configuracao, erro, sizeof(erro))) {
        fprintf(stderr, "Erro: %s\n", erro);
        return EXIT_FAILURE;
    }

    entrada = fopen(opcoes->input, "rb");
    if (entrada == NULL) {
        fprintf(stderr, "Erro ao abrir entrada: %s\n", opcoes->input);
        goto fim;
    }

    if (strcmp(opcoes->reader, "nasa") == 0)
        reader = reader_nasa_criar(entrada);
    else
        reader = reader_empresas_criar(entrada);
    if (reader == NULL) {
        fprintf(stderr, "Erro ao criar leitor ou cabeçalho inválido.\n");
        goto fim;
    }

    if (opcoes->query[0] != '\0') {
        arquivo_consulta = fopen(opcoes->query, "rb");
        if (arquivo_consulta == NULL ||
            !query_ler(arquivo_consulta, reader_esquema(reader), &consulta)) {
            fprintf(stderr, "Erro ao abrir ou interpretar consulta: %s\n",
                    opcoes->query);
            goto fim;
        }
    }

    base = estrutura_criar(opcoes->base, ESTRUTURA_BASE, consulta);
    resultados = estrutura_criar(opcoes->result, ESTRUTURA_RESULTADOS, consulta);
    if (base == NULL || resultados == NULL) {
        fprintf(stderr,
                "Erro ao criar estruturas. lista_gen precisa de GROUP; "
                "lista_cruz precisa de CROSS.\n");
        goto fim;
    }

    {
        /* A carga inclui a leitura e a inserção de cada registro na base. */
        double inicio = agora();
        while (opcoes->limit == 0 || carregados < (size_t)opcoes->limit) {
            ITEM *item = NULL;
            READER_STATUS leitura = reader_ler(reader, &item);
            if (leitura == READER_FIM) break;
            if (leitura != READER_OK) {
                fprintf(stderr, "Erro ao ler registro %zu.\n", carregados + 1);
                goto fim;
            }
            if (!estrutura_inserir(base, item)) {
                item_apagar(&item);
                fprintf(stderr, "Erro ao inserir registro %zu na base.\n",
                        carregados + 1);
                goto fim;
            }
            ++carregados;
        }
        tempo_carga = agora() - inicio;
    }

    {
        busca = (BUSCA){consulta, resultados, 0, 0, false, 0.0, 0.0};
        double inicio = agora();
        estrutura_percorrer(base, buscar_item, &busca);
        /* Inclui o percurso da base e a avaliação da consulta. Descontamos
         * as inserções, já medidas no visitante, para não contá-las duas vezes. */
        busca.tempo_busca = (agora() - inicio) - busca.tempo_insercao;
        if (busca.erro) {
            fprintf(stderr, "Erro ao inserir registro nos resultados.\n");
            goto fim;
        }
    }

    saida = fopen(opcoes->output, "wb");
    if (saida == NULL) {
        fprintf(stderr, "Erro ao abrir saída: %s\n", opcoes->output);
        goto fim;
    }

    {
        exportacao = (EXPORTACAO){reader, saida, (size_t)opcoes->top_k, 0, false};
        /* Inclui cabeçalho, registros e a conclusão da escrita no arquivo. */
        double inicio = agora();
        if (!reader_escrever_cabecalho(reader, saida)) exportacao.erro = true;
        else estrutura_percorrer(resultados, exportar_item, &exportacao);
        if (fflush(saida) != 0) exportacao.erro = true;
        if (fclose(saida) != 0) exportacao.erro = true;
        saida = NULL;

        if (exportacao.erro) {
            fprintf(stderr, "Erro ao escrever arquivo de saída.\n");
            goto fim;
        }
        tempo_escrita = agora() - inicio;
    }

    /* Publicar o resumo somente depois de confirmar a escrita do CSV.
     * Os tempos internos estão em segundos; o enunciado pede milissegundos. */
    printf("STATUS=OK\n");
    printf("INPUT_RECORDS=%zu\n", carregados);
    printf("MATCHED_RECORDS=%zu\n", busca.aceitos);
    printf("OUTPUT_RECORDS=%zu\n", exportacao.escritos);
    printf("LOAD_MS=%.3f\n", tempo_carga * 1000.0);
    printf("SEARCH_MS=%.3f\n", busca.tempo_busca * 1000.0);
    printf("RESULT_INSERT_MS=%.3f\n", busca.tempo_insercao * 1000.0);
    printf("WRITE_MS=%.3f\n", tempo_escrita * 1000.0);
    status = EXIT_SUCCESS;

fim:
    liberar(&entrada, &arquivo_consulta, &saida, &reader, &consulta,
            &base, &resultados);
    return status;
}
