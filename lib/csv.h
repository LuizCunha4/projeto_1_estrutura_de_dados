#ifndef CSV_H
#define CSV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
    char **campos;
    size_t quantidade;
    char *original;
} CSV_REGISTRO;

typedef enum { CSV_ERRO = -1, CSV_FIM = 0, CSV_OK = 1 } CSV_STATUS;

/* Le um registro logico, inclusive campos entre aspas com varias linhas.
 * campos contem strings decodificadas (aspas duplas viram uma aspa).
 * original contem os bytes lidos, incluindo LF, CRLF ou CR final, se houver.
 * Ambos terminam em NUL; NUL na entrada e rejeitado. Linha vazia tem um
 * campo vazio; arquivo vazio retorna CSV_FIM. Nao remove espacos nem BOM.
 * Delimitador pode ser qualquer byte exceto NUL, aspa dupla, CR e LF.
 * Aspas so podem abrir no inicio do campo; depois de fecha-las, somente
 * delimitador, terminador ou EOF sao aceitos. Nao ha limite fixo de tamanho.
 *
 * saida deve apontar para uma variavel disponivel (libere o registro anterior
 * antes de reutiliza-la). Em CSV_OK, o chamador recebe a propriedade de toda
 * a memoria do registro e deve chamar csv_apagar. Nos demais casos, *saida
 * fica NULL. Erros de argumentos, sintaxe, memoria ou E/S retornam CSV_ERRO.
 * Em erro, bytes podem ter sido consumidos; nao ha recuperacao automatica.
 * FILE e emprestado: nunca e fechado. Use modo binario para bytes exatos. */
CSV_STATUS csv_ler(FILE *arquivo, char delimitador, CSV_REGISTRO **saida);

/* Libera campos, original, vetor e registro; aceita NULL e zera *registro.
 * Nao libere os membros separadamente nem use referencias apos esta chamada. */
void csv_apagar(CSV_REGISTRO **registro);

/* Escreve original sem reconstruir campos ou acrescentar terminador.
 * Nao assume propriedade nem fecha FILE. O chamador deve verificar fflush
 * ou fclose ao terminar para detectar falhas adiadas de escrita. Um retorno
 * false pode deixar uma escrita parcial no arquivo.
 * Exige registro valido, produzido por csv_ler, e arquivo em modo binario. */
bool csv_escrever_original(FILE *arquivo, const CSV_REGISTRO *registro);

#endif
