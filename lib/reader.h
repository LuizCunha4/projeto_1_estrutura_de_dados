#ifndef READER_H
#define READER_H

#include "item.h"

//Ps Luiz (ou Rizziolli) do futuro: retorna um ponteiro para um vetor com todos os itens do arquivo 
ITEM** arquivo_em_itens(const char *caminho_arquivo, int *total_lidos, char **cabecalho_out);

#endif
