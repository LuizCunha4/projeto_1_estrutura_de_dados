#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "item.h"
#include "reader.h"

int ler_item(FILE* arquivo, ITEM* item){
    char linha[1000];
    if (fgets(linha, sizeof(linha), arquivo) == NULL){
        return 0;
    }

    
    return 1;
}