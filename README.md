# Projeto 1 — Busca e organização de dados

Trabalho de SCC0202 (Algoritmos e Estruturas de Dados I). A especificação completa está em [referencia/descricao-projeto1.pdf](referencia/descricao-projeto1.pdf) (material local ignorado pelo Git). O programa final deve ser escrito em C, compilado com `-Wall -Wextra -pedantic` e gerado pelo comando `make` como `./trabalho`.

## Estado atual

O programa completo **ainda não compila**: os TADs de estruturas precisam ser adaptados ao item genérico, o módulo de consulta permanece pendente e o `makefile` ainda não gera o executável. Os comandos de execução abaixo são uma meta, não instruções para rodar a versão atual. A função principal será implementada por último, após os módulos estarem funcionais e consistentes.

## Dados, leitura e propriedade da memória

- O [ITEM](lib/item.h) guarda campos tipados e referencia um esquema compartilhado. Cada leitor conhece os nomes e tipos do seu dataset; pilhas, filas e listas não devem conhecer campos específicos de asteroides ou empresas.
- O [CSV](lib/csv.h) separa registros lógicos, incluindo campos entre aspas com vírgulas, aspas escapadas e quebras de linha. A representação original é preservada para exportação, sem reconstruir números ou perder valores ausentes.
- Os [leitores](lib/reader.h) consomem e preservam o cabeçalho e entregam um item por chamada. Não criam um vetor com toda a base: o chamador deve inserir cada item imediatamente no TAD escolhido, e controlar o limite de leitura.
- O chamador abre e fecha o arquivo. O leitor possui o esquema e o cabeçalho. Cada item entregue pertence ao chamador até a inserção bem-sucedida na base; em falha, o chamador deve apagá-lo. Resultados apenas referenciam registros da base.
- A ordem de liberação é: estrutura de resultados, registros/estrutura base, leitor (incluindo esquema). O arquivo pode ser fechado após terminar a leitura, pois a exportação utiliza os dados preservados em memória.
- O original de cada registro pertence ao item. Alterações em campos invalidam essa cópia para evitar exportar valores antigos silenciosamente. A exportação preservada destina-se aos registros lidos e não modificados.

### Particularidades do Mapa de Empresas

O arquivo fornecido tem 17 colunas: oito do bloco de municípios, uma separadora vazia e oito de uma tabela independente de estados. Um estado na mesma linha de um município **não representa necessariamente o estado desse município**.

O leitor preserva todas essas colunas na saída. No esquema de consulta, o bloco municipal mantém seus nomes; o bloco estadual recebe o prefixo `estado.` e a coluna separadora recebe o nome `_separador`. Esses nomes internos não substituem o cabeçalho original na exportação. Os códigos IBGE são textos (identificadores), as contagens são inteiros e os tempos são reais. O marcador `-` nos tempos representa ausência, não zero.

## Testes isolados de CSV e leitores

Execute na raiz do repositório. Os testes têm sua própria função principal, apenas para validação; não implementam a função principal do aplicativo.

```sh
cc -std=c11 -Wall -Wextra -pedantic -Werror -fsanitize=address,undefined -g \
  -Ilib tests/test_csv.c src/csv.c -o /tmp/test_csv
/tmp/test_csv
cc -std=c11 -Wall -Wextra -pedantic -Werror -fsanitize=address,undefined -g \
  -Ilib tests/test_reader.c src/reader.c src/reader_nasa.c \
  src/reader_empresas.c src/item.c src/csv.c -o /tmp/test_reader
/tmp/test_reader
```

Os testes de leitores incluem os dois datasets completos e comparação byte a byte da exportação. Conversões numéricas usam o locale `C` (padrão inicial de um programa C), com ponto decimal. Cabeçalhos incompatíveis, quantidades incorretas de campos e valores numéricos inválidos são rejeitados; fim de arquivo não é confundido com erro.

## Requisitos principais

- `--reader`, `--input`, `--base`, `--result`, `--query`, `--output`, `--top-k` e `--limit` devem funcionar pela linha de comando. Zero em `--top-k` e `--limit` significa sem limite.
- As opções de estrutura são `pilha`, `fila`, `lista_seq`, `lista_enc`, `lista_cabeca`, `lista_ord`, `lista_gen` e `lista_cruz`. A base escolhida precisa receber os itens durante a leitura, e a busca deve percorrer essa estrutura. Os resultados devem ser inseridos em outra estrutura escolhida pelo usuário.
- O arquivo de consulta deve aceitar `FILTER`, `SIM`, `ACCEPT`, `ORDER`, `GROUP` e `CROSS`. A saída CSV deve preservar o cabeçalho, as colunas, a ordem e o delimitador da entrada.
- A Parte 2 deve reutilizar os mesmos TADs, trocando apenas o leitor específico do dataset.
- A entrega também exige relatório do projeto, seis exemplos de execução, análise de complexidade e medições com pelo menos três execuções por configuração.

## Ordem de implementação sugerida

1. Adaptar os TADs ao `ITEM` genérico e ao contrato de propriedade da memória, corrigindo sua compilação e testando cada estrutura isoladamente.
2. Implementar o parser de consulta e o cálculo de similaridade, com testes pequenos e previsíveis.
3. Conectar os percursos, comparações, grupos e eixos às oito estruturas, preservando suas diferenças reais de organização.
4. Somente então implementar a função principal e integrar carga, busca, inserção de resultados e exportação; verificar cada combinação de `--base` e `--result`.
5. Medir tempos, produzir exemplos e escrever os dois relatórios.

## Exemplo de interface final

```sh
./trabalho \
  --reader nasa \
  --input data/asteroid_dataset_20251019.csv \
  --base lista_enc \
  --result lista_ord \
  --query consultas/q1.txt \
  --output resultado.csv \
  --top-k 100 \
  --limit 10000
```

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
