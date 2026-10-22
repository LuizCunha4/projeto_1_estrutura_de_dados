#ifndef READER_PRIVADO_H
#define READER_PRIVADO_H
#include "reader.h"
/* Descritores estaticos dos datasets. Nome de entrada pode ser vazio;
 * nome interno de coluna precisa ser unico e nao vazio. */
typedef struct {
    const char *entrada;
    const char *nome;
    TIPO_CAMPO tipo;
    bool hifen_ausente;
} READER_COLUNA;
READER *reader_criar_interno(FILE *arquivo, char delimitador,
                           const READER_COLUNA *descritores, size_t quantidade);
#endif
