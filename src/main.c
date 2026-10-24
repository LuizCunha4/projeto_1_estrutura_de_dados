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

static double segundos(clock_t inicio, clock_t fim) {
    return (double)(fim - inicio) / CLOCKS_PER_SEC;
}

static void buscar_item(ITEM *item, void *contexto) {
    BUSCA *busca = contexto;
    clock_t inicio = clock();
    bool aceito = busca->consulta == NULL ||
                  query_item_aceito(busca->consulta, item);
    busca->tempo_busca += segundos(inicio, clock());
    ++busca->examinados;

    if (!aceito) return;
    ++busca->aceitos;
    if (busca->erro) return;

    inicio = clock();
    if (!estrutura_inserir(busca->resultados, item)) busca->erro = true;
    busca->tempo_insercao += segundos(inicio, clock());
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
        clock_t inicio = clock();
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
        tempo_carga = segundos(inicio, clock());
    }

    {
        BUSCA busca = {consulta, resultados, 0, 0, false, 0.0, 0.0};
        estrutura_percorrer(base, buscar_item, &busca);
        if (busca.erro) {
            fprintf(stderr, "Erro ao inserir registro nos resultados.\n");
            goto fim;
        }

        printf("Registros carregados: %zu\n", carregados);
        printf("Registros examinados: %zu\n", busca.examinados);
        printf("Registros aceitos: %zu\n", busca.aceitos);
        printf("Tempo de carga: %.6f s\n", tempo_carga);
        printf("Tempo de busca: %.6f s\n", busca.tempo_busca);
        printf("Tempo de inserção nos resultados: %.6f s\n",
               busca.tempo_insercao);
    }

    saida = fopen(opcoes->output, "wb");
    if (saida == NULL) {
        fprintf(stderr, "Erro ao abrir saída: %s\n", opcoes->output);
        goto fim;
    }

    {
        EXPORTACAO exportacao = {reader, saida, (size_t)opcoes->top_k, 0, false};
        clock_t inicio = clock();
        if (!reader_escrever_cabecalho(reader, saida)) exportacao.erro = true;
        else estrutura_percorrer(resultados, exportar_item, &exportacao);
        if (fflush(saida) != 0) exportacao.erro = true;
        if (fclose(saida) != 0) exportacao.erro = true;
        saida = NULL;

        if (exportacao.erro) {
            fprintf(stderr, "Erro ao escrever arquivo de saída.\n");
            goto fim;
        }
        printf("Registros escritos: %zu\n", exportacao.escritos);
        printf("Tempo de escrita: %.6f s\n", segundos(inicio, clock()));
    }

    printf("Status: sucesso\n");
    status = EXIT_SUCCESS;

fim:
    liberar(&entrada, &arquivo_consulta, &saida, &reader, &consulta,
            &base, &resultados);
    return status;
}
