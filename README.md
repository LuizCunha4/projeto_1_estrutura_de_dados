# Busca e organização de dados em C

Aplicativo de linha de comando para consultar arquivos CSV de aproximações de asteroides e do Mapa de Empresas. Permite escolher independentemente a estrutura que armazena a base e a que organiza os resultados, combinando filtros, similaridade numérica, ordenação e agrupamento.

Implementado em C para a disciplina SCC0202 — Algoritmos e Estruturas de Dados I, com oito TADs próprios reutilizados pelos dois leitores.

## Recursos

- Oito estruturas disponíveis para a base e os resultados, incluindo listas com sentinela, ordenada, generalizada e cruzada.
- Filtros por campos textuais, numéricos e booleanos; similaridade com pesos e limiar de aceitação.
- Ordenação durante a inserção, grupos e subgrupos e organização de um mesmo registro em dois eixos.
- Leitura de CSV com campos entre aspas, delimitadores internos, aspas escapadas e quebras de linha dentro dos campos.
- Exportação com o cabeçalho, as colunas e o delimitador originais, sem colunas auxiliares.
- Contagens e tempos separados de carga, busca, inserção nos resultados e escrita.

## Compilação

Requisitos: Linux ou ambiente POSIX compatível com `clock_gettime`, compilador C11 e GNU Make. Não são necessárias bibliotecas externas além das bibliotecas padrão do sistema.

Na raiz do repositório:

```sh
make
```

O comando gera o executável `trabalho`, usando `-std=c11 -Wall -Wextra -pedantic`. Para remover o executável gerado:

```sh
make clean
```

## Execução

### Asteroides: ordenar por similaridade

```sh
./trabalho \
  --reader nasa \
  --input data/asteroids.csv \
  --base lista_enc \
  --result lista_ord \
  --query consultas/q1_asteroid.txt \
  --output resultado_asteroides.csv \
  --top-k 10 \
  --limit 0
```

Esse exemplo carrega toda a base, seleciona aproximações futuras próximas do perfil definido na consulta e exporta os dez primeiros resultados em ordem decrescente de similaridade.

### Empresas: agrupar por aberturas e fechamentos

```sh
./trabalho \
  --reader empresas \
  --input data/mapa_empresas.csv \
  --base lista_cruz \
  --result lista_gen \
  --query consultas/q6_empresas.txt \
  --output resultado_empresas.csv \
  --top-k 10 \
  --limit 0
```

Esse exemplo seleciona municípios com até dez empresas abertas e pelo menos cem empresas ativas. Usa dois eixos na base e grupos/subgrupos nos resultados, definidos pelas quantidades de aberturas e fechamentos.

Os caminhos dos exemplos são relativos à raiz do repositório. O diretório de saída deve existir; um arquivo de saída existente será sobrescrito. Use um arquivo de saída distinto dos arquivos de entrada e consulta, inclusive quando utilizar links simbólicos ou caminhos alternativos.

### Opções

| Opção | Obrigatória | Descrição |
| --- | --- | --- |
| `--reader` | Sim | Leitor: `nasa` ou `empresas`. |
| `--input` | Sim | Caminho do CSV de entrada. |
| `--base` | Sim | Estrutura que recebe os registros durante a leitura. |
| `--result` | Sim | Estrutura que organiza os registros aceitos. |
| `--query` | Não | Arquivo de consulta. Quando omitido, todos os registros carregados são aceitos. |
| `--output` | Sim | Caminho do CSV de saída. |
| `--top-k` | Não | Máximo de registros exportados. Padrão: `0`, sem limite. |
| `--limit` | Não | Máximo de registros carregados desde o início da entrada. Padrão: `0`, sem limite. |

`--top-k` seleciona os primeiros registros na ordem de acesso da estrutura de resultados. Não interrompe a busca nem significa necessariamente os registros de maior similaridade: para isso, use `lista_ord` e `ORDER|similarity|DESC`.

## Estruturas disponíveis

| Nome | Organização |
| --- | --- |
| `pilha` | LIFO: percurso a partir do último item inserido. |
| `fila` | FIFO: percurso na ordem de inserção. |
| `lista_seq` | Vetor dinâmico, com inserção no fim. |
| `lista_enc` | Nós duplamente encadeados, com inserção no fim. |
| `lista_cabeca` | Lista encadeada com nó sentinela que não representa um registro. |
| `lista_ord` | Vetor dinâmico que mantém a ordem a cada inserção. |
| `lista_gen` | Grupos e subgrupos definidos por campos. |
| `lista_cruz` | Dois eixos de organização sobre os mesmos registros, sem duplicá-los. |

Base e resultados podem usar o mesmo tipo de estrutura, em instâncias distintas. A ordem de percurso da base determina a ordem de chegada dos itens aos resultados.

Para `--result lista_gen`, a consulta precisa de `GROUP`; para `--result lista_cruz`, precisa de `CROSS`. Na base, quando esses comandos não são fornecidos, essas estruturas usam as duas primeiras colunas. A lista ordenada usa `ORDER` quando presente; caso contrário, ordena pela primeira coluna em ordem crescente.

GROUP e CROSS organizam valores exatos, sem calcular somas ou contagens agregadas. Não implicam ordenação crescente dos grupos. A linha de comando exporta pelo eixo de linhas da lista cruzada; o segundo percurso está disponível na API.

## Arquivos de consulta

Cada linha contém um comando com argumentos separados por `|`. Os nomes dos campos devem corresponder ao esquema do leitor e podem conter espaços.

| Comando | Sintaxe | Função |
| --- | --- | --- |
| FILTER | `FILTER\|campo\|operador\|valor` | Condição obrigatória de seleção. |
| SIM | `SIM\|campo\|alvo\|tolerancia\|peso` | Similaridade de um campo numérico. |
| ACCEPT | `ACCEPT\|limiar` | Pontuação mínima de aceitação, entre 0 e 1. |
| ORDER | `ORDER\|campo\|ASC` ou `ORDER\|campo\|DESC` | Critério da lista ordenada; aceita `similarity` como campo especial. |
| GROUP | `GROUP\|campo\|campo` | Campos dos grupos e subgrupos da lista generalizada. |
| CROSS | `CROSS\|campo\|campo` | Dois campos distintos para os eixos da lista cruzada. |

Exemplo de [consulta por similaridade](consultas/q1_asteroid.txt):

```text
FILTER|is_future_event|==|true
FILTER|distance_au|<=|0.08
SIM|velocity_km_s|15.0|10.0|1.0
SIM|risk_score|0.70|0.30|2.0
ACCEPT|0.60
ORDER|similarity|DESC
```

Todos os filtros devem ser satisfeitos. Os operadores são `==`, `!=`, `<`, `<=`, `>` e `>=`; para booleanos, use `==` ou `!=` com `true` ou `false`. Números nas consultas usam ponto decimal.

Cada critério SIM calcula `max(0, 1 - abs(valor - alvo) / tolerancia)`. Tolerância e peso devem ser positivos. A pontuação final é a média ponderada dos critérios; campos ausentes contribuem com zero. Sem SIM, os filtros determinam a seleção. Sem ACCEPT, não há limiar adicional de similaridade.

ORDER, GROUP e CROSS configuram suas respectivas estruturas; não alteram a organização de uma fila ou pilha.

### Consultas de exemplo

| Arquivo | Objetivo | Base → resultados sugeridos |
| --- | --- | --- |
| [q1_asteroid.txt](consultas/q1_asteroid.txt) | Aproximações futuras selecionadas por similaridade de velocidade e risco. | `lista_enc` → `lista_ord` |
| [q2_asteroid.txt](consultas/q2_asteroid.txt) | Aproximações próximas entre 2024 e 2026, agrupadas por categoria e ano. | `fila` → `lista_gen` |
| [q3_asteroid.txt](consultas/q3_asteroid.txt) | Aproximações futuras organizadas por categoria e ano em dois eixos. | `lista_seq` → `lista_cruz` |
| [q4_empresas.txt](consultas/q4_empresas.txt) | Municípios ordenados pelo menor tempo médio de abertura. | `lista_cabeca` → `lista_ord` |
| [q5_empresas.txt](consultas/q5_empresas.txt) | Municípios próximos de um perfil de empresas ativas e tempo de abertura. | `pilha` → `lista_ord` |
| [q6_empresas.txt](consultas/q6_empresas.txt) | Municípios agrupados pelas quantidades de empresas abertas e fechadas. | `lista_cruz` → `lista_gen` |

## Datasets e particularidades

Os leitores validam cabeçalhos e tipos conhecidos; não interpretam automaticamente qualquer CSV.

- **Asteroides:** [data/asteroids.csv](data/asteroids.csv), com 89.227 registros. Os indicadores de eventos passados e futuros são os valores presentes no dataset, não recalculados pela data de execução. Os primeiros 10.000 registros não satisfazem o filtro de eventos futuros da primeira consulta; use `--limit 0` para reproduzir o exemplo completo.
- **Mapa de Empresas:** [data/mapa_empresas.csv](data/mapa_empresas.csv), com 5.570 registros e referência em julho de 2026. Contém um bloco municipal e um bloco estadual independente, separados por uma coluna vazia. O estado na mesma linha não corresponde necessariamente ao município.

Nas consultas de empresas, os campos municipais mantêm os nomes originais; os estaduais recebem o prefixo `estado.`, e a separadora é identificada como `_separador`. Esses nomes internos não modificam o cabeçalho exportado. Códigos IBGE são tratados como texto. O marcador `-` nos tempos indica ausência, não zero.

## Saída e métricas

O CSV preserva as colunas e sua ordem original; a ordem dos registros segue a estrutura de resultados. A pontuação de similaridade não é acrescentada ao arquivo. Quando não há correspondências, a saída contém somente o cabeçalho.

Após concluir a escrita, o programa informa:

| Chave | Significado |
| --- | --- |
| `STATUS` | `OK` quando a execução termina com sucesso, mesmo sem correspondências. |
| `INPUT_RECORDS` | Registros carregados. |
| `MATCHED_RECORDS` | Registros aceitos pela consulta, antes do limite de exportação. |
| `OUTPUT_RECORDS` | Registros efetivamente exportados. |
| `LOAD_MS` | Leitura, criação dos itens e inserção na base. |
| `SEARCH_MS` | Percurso e avaliação da consulta, descontando as inserções nos resultados. |
| `RESULT_INSERT_MS` | Soma dos tempos de inserção nos resultados. |
| `WRITE_MS` | Escrita do cabeçalho e registros, percurso dos resultados e conclusão da escrita. |

Os tempos são decorridos, medidos com relógio monotônico e apresentados em milissegundos. Sua soma não inclui toda a execução, como abertura dos arquivos, interpretação da consulta e liberação final. Erros são enviados para `stderr` e retornam código de saída diferente de zero.

## Testes

Após compilar, execute a suíte de integração:

```sh
make
sh tests/test_integracao.sh
```

A suíte verifica as 64 combinações de base/resultados para os dois leitores com dados artificiais, além de consultas, limites, formato de saída e cenários de erro. Utiliza ferramentas usuais do Linux, como `awk`, `sort`, `cmp` e `mktemp`, e remove seus arquivos temporários ao terminar.

Os testes unitários de CSV, leitores, consultas e TADs estão em [tests/](tests/).

## Organização do código

- [src/](src/): implementações e fluxo principal da aplicação.
- [lib/](lib/): interfaces públicas e comentários sobre contratos de uso, propriedade da memória e tempo de vida dos objetos.
- [data/](data/): conjuntos de dados suportados.
- [consultas/](consultas/): seis consultas de exemplo.
- [tests/](tests/): testes unitários e de integração.
