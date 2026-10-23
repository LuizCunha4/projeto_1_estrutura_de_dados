#ifndef TAD_H
#define TAD_H

#include "item.h"

/* Contrato compartilhado pelos TADs: inserção bem-sucedida armazena a
 * referência ao ITEM. O TAD nunca o apaga. A camada que representa a BASE
 * assume a propriedade dos registros; a camada RESULTADOS guarda apenas
 * referências. Percurso visita cada registro uma vez. */
typedef ITEM_VISITANTE TAD_VISITANTE;

#endif
