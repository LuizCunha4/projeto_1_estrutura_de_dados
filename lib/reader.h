<<<<<<< HEAD
#ifndef READER_H
#define READER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "item.h"

int ler_item(FILE* arquivo, ITEM* item);

#endif
=======
#ifndef READER_H
#define READER_H

#include "item.h"

//Ps Luiz (ou Rizziolli) do futuro: retorna um ponteiro para um vetor com todos os itens do arquivo 
ITEM** reader_ler_todos(const char *caminho_arquivo, int *total_lidos, char **cabecalho_out);

#endif
>>>>>>> a0de4d175a37e5f744049109f80c6a7ee0ed20a3
