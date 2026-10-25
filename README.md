# Projeto 1 — Busca e organização de dados

Trabalho de SCC0202 (Algoritmos e Estruturas de Dados I). A especificação completa está em [referencia/descricao-projeto1.pdf](referencia/descricao-projeto1.pdf) (material local ignorado pelo Git). O programa final deve ser escrito em C, compilado com `-Wall -Wextra -pedantic` e gerado pelo comando `make` como `./trabalho`.

## Estado atual

O aplicativo está integrado. `src/main.c` contém o fluxo principal; `src/argumentos.c` lê e valida as opções de linha de comando. O comando `make` gera `trabalho`. Os módulos possuem testes isolados em `tests/`.

## Compilar e executar

No Ubuntu, é necessário ter um compilador C e o GNU Make. Execute na raiz:

```sh
make
./trabalho \
  --reader nasa \
  --input data/asteroids.csv \
  --base lista_enc \
  --result lista_ord \
  --query consultas/q1.txt \
  --output resultado.csv \
  --top-k 100 \
  --limit 10000
```

A barra `\` no fim da linha continua o mesmo comando na linha seguinte. O último argumento não precisa dessa barra. Sem ela, Enter executa imediatamente o comando digitado.

O executável `trabalho` é gerado localmente e ignorado pelo Git; ele não deve ser distribuído como arquivo versionado. `make clean` remove apenas esse executável; `make` o recompila. Os fontes e o Makefile devem permanecer versionados.

Os dados locais estão em [data/asteroids.csv](data/asteroids.csv) e [data/mapa_empresas.csv](data/mapa_empresas.csv). São os mesmos arquivos fornecidos anteriormente com nomes mais longos: a comparação SHA-256 confirmou que a renomeação não alterou seu conteúdo. Para empresas, use `--reader empresas --input data/mapa_empresas.csv` e uma consulta com os nomes de seu esquema.

São obrigatórios `--reader`, `--input`, `--base`, `--result` e `--output`. Sem `--query`, todos os registros carregados são aceitos. `--limit 0` carrega toda a base; `--top-k 0` exporta todos os resultados. A estrutura de resultados `lista_gen` requer GROUP na consulta, e `lista_cruz` requer CROSS. Os limites omitidos também assumem zero.

O limite de carga seleciona os primeiros registros da entrada. Na consulta de exemplo do enunciado, o filtro de eventos futuros rejeita todos os primeiros 10.000 asteroides do dataset: zero resultados é uma resposta válida, e a saída contém somente o cabeçalho. `STATUS=OK` indica que a execução terminou corretamente, não que houve correspondências.

## Saída e medição de tempo

Após concluir a escrita do CSV, o programa imprime as chaves exigidas:

```text
STATUS=OK
INPUT_RECORDS=...
MATCHED_RECORDS=...
OUTPUT_RECORDS=...
LOAD_MS=...
SEARCH_MS=...
RESULT_INSERT_MS=...
WRITE_MS=...
```

As contagens são inteiras e os tempos são apresentados em milissegundos com três casas decimais. Erros são enviados para stderr, com retorno diferente de zero, sem publicar o resumo de sucesso.

A função `agora()` usa `clock_gettime(CLOCK_MONOTONIC)`, seguindo o exemplo do professor: mede tempo decorrido, não somente tempo de CPU. A carga inclui leitura e inserção na base. A busca inclui o percurso e a avaliação da consulta, descontando o tempo de inserção nos resultados, informado separadamente. A escrita inclui o cabeçalho, o percurso dos resultados, `fflush` e `fclose`. `top-k` limita a exportação, sem encerrar a busca antes de examinar a base inteira.

## Dados, leitura e propriedade da memória

- O [ITEM](lib/item.h) guarda campos tipados e referencia um esquema compartilhado. Cada leitor conhece os nomes e tipos do seu dataset; pilhas, filas e listas não devem conhecer campos específicos de asteroides ou empresas.
- Os TADs recebem referências a `ITEM` e nunca os liberam ao destruir seus nós. A [fachada comum](lib/estrutura.h) distingue BASE, que libera os registros inseridos, e RESULTADOS, que guarda referências. Percursos usam o callback `TAD_VISITANTE`.
- O [CSV](lib/csv.h) separa registros lógicos, incluindo campos entre aspas com vírgulas, aspas escapadas e quebras de linha. A representação original é preservada para exportação, sem reconstruir números ou perder valores ausentes.
- Os [leitores](lib/reader.h) consomem e preservam o cabeçalho e entregam um item por chamada. Não criam um vetor com toda a base: o chamador deve inserir cada item imediatamente no TAD escolhido, e controlar o limite de leitura.
- O chamador abre e fecha o arquivo. O leitor possui o esquema e o cabeçalho. Cada item entregue pertence ao chamador até a inserção bem-sucedida na base; em falha, o chamador deve apagá-lo. Resultados apenas referenciam registros da base.
- A ordem de liberação é: estrutura de resultados, registros/estrutura base, leitor (incluindo esquema). O arquivo pode ser fechado após terminar a leitura, pois a exportação utiliza os dados preservados em memória.
- O original de cada registro pertence ao item. Alterações em campos invalidam essa cópia para evitar exportar valores antigos silenciosamente. A exportação preservada destina-se aos registros lidos e não modificados.

### Particularidades do Mapa de Empresas

O arquivo fornecido tem 17 colunas: oito do bloco de municípios, uma separadora vazia e oito de uma tabela independente de estados. Um estado na mesma linha de um município **não representa necessariamente o estado desse município**.

O leitor preserva todas essas colunas na saída. No esquema de consulta, o bloco municipal mantém seus nomes; o bloco estadual recebe o prefixo `estado.` e a coluna separadora recebe o nome `_separador`. Esses nomes internos não substituem o cabeçalho original na exportação. Os códigos IBGE são textos (identificadores), as contagens são inteiros e os tempos são reais. O marcador `-` nos tempos representa ausência, não zero.

## Testes isolados

Execute na raiz do repositório. Os testes têm sua própria função principal, apenas para validação; não implementam a função principal do aplicativo.

```sh
cc -std=c11 -Wall -Wextra -pedantic -Werror -fsanitize=address,undefined -g \
  -Ilib tests/test_csv.c src/csv.c -o /tmp/test_csv
/tmp/test_csv
cc -std=c11 -Wall -Wextra -pedantic -Werror -fsanitize=address,undefined -g \
  -Ilib tests/test_reader.c src/reader.c src/reader_nasa.c \
  src/reader_empresas.c src/item.c src/csv.c -o /tmp/test_reader
/tmp/test_reader
cc -std=c11 -Wall -Wextra -pedantic -Werror -fsanitize=address,undefined -g \
  -Ilib tests/test_query.c src/query.c src/item.c src/lista_ord.c \
  src/lista_gen.c src/lista_cruz.c -lm -o /tmp/test_query
/tmp/test_query
cc -std=c11 -Wall -Wextra -pedantic -Werror -fsanitize=address,undefined -g \
  -Ilib tests/test_tads.c src/estrutura.c src/query.c src/item.c src/pilha.c \
  src/fila.c src/lista_seq.c src/lista_enc.c src/lista_ord.c \
  src/lista_cabeca.c src/lista_gen.c src/lista_cruz.c -lm -o /tmp/test_tads
/tmp/test_tads
```

Os testes de leitores incluem os dois datasets completos e comparação byte a byte da exportação. Conversões numéricas usam o locale `C` (padrão inicial de um programa C), com ponto decimal. Cabeçalhos incompatíveis, quantidades incorretas de campos e valores numéricos inválidos são rejeitados; fim de arquivo não é confundido com erro.

## Teste de integração

```sh
make
sh tests/test_integracao.sh
```

O [teste de integração](tests/test_integracao.sh) usa ferramentas usuais do Ubuntu (`sh`, `awk`, `sort`, `cmp` e `mktemp`). Cria seus arquivos em um diretório temporário e os remove ao terminar, sem modificar os datasets ou as consultas do usuário. Um caminho de executável pode ser passado como primeiro argumento, permitindo testar uma compilação com sanitizadores.

Cobertura da verificação de 25/10/2026:

- Compilação de todos os fontes com `-std=c11 -Wall -Wextra -pedantic -Werror`.
- 64 combinações base/resultado para cada leitor, com 65 registros artificiais por entrada, exercitando crescimento dos vetores e do hash da base.
- Resultados esperados calculados independentemente do parser C: FILTER, média ponderada de SIM, ACCEPT, ORDER por similarity, nomes de campos com espaços, FIFO/LIFO e exportação do prefixo top-k. GROUP e CROSS são usados na matriz; os testes de TADs também exercitam os dois eixos da cruzada.
- Formato exato das oito linhas de saída, contagens e tempos não negativos em milissegundos.
- `--limit 0` e `--top-k 0` com os datasets reais completos: 89.227 asteroides e 5.570 registros de empresas, com CSV de saída idêntico à entrada byte a byte.
- Consulta sem correspondências, argumentos inválidos/repetidos/ausentes, entrada inexistente, CSV malformado, consulta inválida, GROUP/CROSS ausentes nos resultados correspondentes e falha de escrita em `/dev/full`.
- Os quatro testes C e a suíte de integração passaram também com AddressSanitizer, UBSan e detecção de vazamentos habilitada, sem diagnósticos desses sanitizadores.

Esses testes sustentam a conclusão de que o fluxo funcional está atendido nos casos verificados; não são prova de ausência de todo erro possível. Não há injeção sistemática de falhas de alocação nem cobertura exaustiva das APIs diretas dos TADs.

## Requisitos principais

- `--reader`, `--input`, `--base`, `--result`, `--query`, `--output`, `--top-k` e `--limit` devem funcionar pela linha de comando. Zero em `--top-k` e `--limit` significa sem limite.
- As opções de estrutura são `pilha`, `fila`, `lista_seq`, `lista_enc`, `lista_cabeca`, `lista_ord`, `lista_gen` e `lista_cruz`. A base escolhida precisa receber os itens durante a leitura, e a busca deve percorrer essa estrutura. Os resultados devem ser inseridos em outra estrutura escolhida pelo usuário.
- O arquivo de consulta usa campos separados por `|`: `FILTER|campo|operador|valor`, `SIM|campo|alvo|tolerancia|peso`, `ACCEPT|limiar`, `ORDER|campo|direcao` (direção `ASC` ou `DESC`), `GROUP|campo|campo` e `CROSS|campo|campo`. `ORDER|similarity|DESC` ordena pela pontuação. A saída CSV deve preservar o cabeçalho, as colunas, a ordem e o delimitador da entrada.
- A Parte 2 deve reutilizar os mesmos TADs, trocando apenas o leitor específico do dataset.
- A entrega também exige relatório do projeto, seis exemplos de execução, análise de complexidade e medições com pelo menos três execuções por configuração.

## Escopo restante

O aplicativo e sua interface de linha de comando estão implementados. Relatórios, experimentos e exemplos completos da entrega serão preparados pelo grupo em uma etapa posterior, conforme combinado. A entrega completa também precisa das duas consultas úteis da Parte 2 e da demonstração de uma estrutura avançada.

O percurso interrompível permanece uma melhoria opcional de eficiência. Os detalhes de robustez já identificados (representação pública de SIM inteiro, equivalência de caminhos de arquivos e validações no uso direto de TADs ordenados) não foram alterados nesta etapa, por orientação do usuário. Use caminhos distintos para entrada, consulta e saída.

## Git em grupo

Antes de começar uma tarefa nova, veja o estado local e as atualizações remotas:

```sh
git status
git fetch origin
git log --oneline --left-right HEAD...origin/main
```

`git fetch` apenas atualiza a referência remota local. Se não houver alterações locais em conflito, `git pull --ff-only origin main` incorpora os commits novos sem criar um merge automático. Para trabalhar em uma tarefa isolada:

```sh
git switch -c nome-da-tarefa
# edite e teste os arquivos
git status
git add caminho/do/arquivo
git commit -m "Descreve a alteracao"
git push -u origin nome-da-tarefa
```

Ao receber mudanças de colegas, confira `git status` antes do `pull`. Evite `git reset --hard` ou `git push --force` para resolver conflitos sem antes entender quais alterações seriam descartadas.

O grupo já confirmou a escolha do dataset da Parte 2 no e-Disciplinas.
