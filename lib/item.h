#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    CAMPO_TEXTO, CAMPO_INTEIRO, CAMPO_REAL, CAMPO_BOOLEANO
} TIPO_CAMPO;

typedef struct {
    const char *nome;
    TIPO_CAMPO tipo;
} COLUNA;

/* Somente o membro indicado por CAMPO.tipo pode ser acessado. */
typedef union {
    const char *texto;
    long long inteiro;
    double real;
    bool booleano;
} VALOR;

typedef struct {
    TIPO_CAMPO tipo;
    bool ausente; /* Diferencia ausencia de zero, false e texto vazio. */
    VALOR valor;
} CAMPO;

typedef struct esquema ESQUEMA;
typedef struct item ITEM;

/* Copia nomes e tipos. O esquema e imutavel e compartilhado pelos itens.
 * Rejeita quantidade zero, nomes vazios/duplicados e tipos invalidos.
 * Retorna NULL em entrada invalida ou falha de alocacao.
 * O responsavel pelo dataset libera o esquema APOS liberar todos os itens. */
ESQUEMA *esquema_criar(const COLUNA *colunas, size_t quantidade);
void esquema_apagar(ESQUEMA **esquema);
size_t esquema_quantidade(const ESQUEMA *esquema);

/* Ponteiro emprestado, valido ate apagar o esquema; NULL em indice invalido. */
const COLUNA *esquema_coluna(const ESQUEMA *esquema, size_t indice);

/* Busca exata por nome; em falha, nao modifica *indice. */
bool esquema_buscar(const ESQUEMA *esquema, const char *nome, size_t *indice);

/* Cria um vetor com um campo por coluna, todos inicialmente ausentes.
 * O item apenas referencia o esquema; nao o copia nem o libera.
 * O leitor possui o item ate inseri-lo com sucesso na BASE, que passa a ser
 * dona do registro. Em falha de insercao, o leitor deve liberar o item.
 * RESULTADOS apenas referenciam registros: nao devem chamar item_apagar.
 * Ordem de liberacao: resultados, registros da base, esquema compartilhado. */
ITEM *item_criar(const ESQUEMA *esquema);
/* Libera textos, vetor e item; nao libera o esquema. Aceita NULL e zera *item.
 * Todas as referencias ao registro ficam invalidas apos esta chamada. */
void item_apagar(ITEM **item);

const ESQUEMA *item_esquema(const ITEM *item);
size_t item_quantidade(const ITEM *item);
/* Retorno emprestado; nao modificar/liberar. Texto pode ser invalidado por
 * substituicao do campo; todo o retorno e invalidado ao apagar o item. */
const CAMPO *item_campo(const ITEM *item, size_t indice);
/* Exige tipo igual ao da coluna. Copia textos; nao assume a propriedade da
 * entrada. Se ausente, ignora valor. Falha preserva o campo anterior.
 * Inicialize o membro da union correspondente ao tipo informado.
 * Preencha antes de ordenar/indexar: mudancas posteriores podem invalidar
 * a organizacao das estruturas que referenciam o registro. */
bool item_definir_campo(ITEM *item, size_t indice, const CAMPO *campo);
bool item_marcar_ausente(ITEM *item, size_t indice);


/* Copia opcional de texto original, pertencente ao ITEM. Nao interpreta CSV.
 * NULL remove a copia; falha de alocacao preserva a anterior. Qualquer
 * definicao bem-sucedida de campo (inclusive ausencia) invalida a copia.
 * Definir somente depois de preencher todos os campos. */
bool item_definir_original(ITEM *item, const char *original);
/* Emprestado; valido ate alterar campo/original ou apagar o ITEM. */
const char *item_original(const ITEM *item);

/* Interpretacao/exportacao CSV pertencem ao leitor/modulo CSV.
 * Valores tipados nao preservam, sozinhos, a representacao original do CSV. */
#endif
