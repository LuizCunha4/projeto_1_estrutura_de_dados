#ifndef READER_H
#define READER_H

#include <stdio.h>
#include "item.h"

typedef struct reader READER;
typedef enum { READER_ERRO = -1, READER_FIM = 0, READER_OK = 1 } READER_STATUS;

/* Consomem e validam o cabecalho. NULL indica erro. FILE e emprestado:
 * nunca e fechado, nem mesmo em falha. Usar streams em modo binario para
 * preservar bytes em plataformas que traduzem terminadores de linha. */
READER *reader_nasa_criar(FILE *arquivo);
READER *reader_empresas_criar(FILE *arquivo);

/* Entrega um item por chamada; exige saida != NULL e *saida == NULL.
 * Em sucesso, o chamador possui o ITEM (e deve apaga-lo se a base rejeitar
 * sua insercao). O reader possui somente o esquema e o cabecalho CSV.
 * EOF e erro sao distintos e persistentes; erros nao permitem continuar.
 * Conversoes numericas exigem LC_NUMERIC="C" (padrao inicial de C).
 * Entradas invalidas da API retornam erro sem consumir o stream. */
READER_STATUS reader_ler(READER *reader, ITEM **saida);

/* Retornos emprestados; validos ate reader_apagar. */
const ESQUEMA *reader_esquema(const READER *reader);
const char *reader_cabecalho(const READER *reader);
char reader_delimitador(const READER *reader);

bool reader_escrever_cabecalho(const READER *reader, FILE *saida);
/* Exporta o original, nao os campos atuais: somente exportar itens vivos,
 * produzidos por este reader, que permaneceram IMUTAVEIS desde a leitura.
 * Preserva o terminador existente; acrescenta LF quando nao houver nenhum,
 * permitindo reordenar registros sem juntar duas linhas na saida.
 * O chamador verifica fflush/fclose ao final para detectar erros adiados.
 * Acesso ao original O(1); alteracoes de campos invalidam a copia. */
bool reader_escrever_item(const READER *reader, FILE *saida, const ITEM *item);

/* Nao libera ITEMs nem fecha FILE. Manter reader vivo ate TODOS os itens
 * serem liberados: eles referenciam o esquema. Ordem: resultados, itens
 * da base, reader, FILE. O cabecalho e liberado aqui; cada ITEM libera seu original. */
void reader_apagar(READER **reader);
#endif
