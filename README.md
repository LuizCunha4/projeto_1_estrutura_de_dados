# Projeto 1 — Busca e organização de dados

Trabalho de SCC0202 (Algoritmos e Estruturas de Dados I). A especificação completa está em `descricao-projeto1.pdf` (arquivo local ainda não versionado). O programa final deve ser escrito em C, compilado com `-Wall -Wextra -pedantic` e gerado pelo comando `make` como `./trabalho`.

## Estado atual

O repositório contém código inicial das estruturas em `src/` e `lib/` e dois CSVs em `data/`, mas **ainda não compila**. Os módulos `csv` e `query` e o `makefile` estão vazios. Há também erros de sintaxe e inconsistências entre headers e implementações; portanto os comandos de execução abaixo são uma meta, não instruções para rodar a versão atual.

## Requisitos principais

- `--reader`, `--input`, `--base`, `--result`, `--query`, `--output`, `--top-k` e `--limit` devem funcionar pela linha de comando. Zero em `--top-k` e `--limit` significa sem limite.
- As opções de estrutura são `pilha`, `fila`, `lista_seq`, `lista_enc`, `lista_cabeca`, `lista_ord`, `lista_gen` e `lista_cruz`. A base escolhida precisa receber os itens durante a leitura, e a busca deve percorrer essa estrutura. Os resultados devem ser inseridos em outra estrutura escolhida pelo usuário.
- O arquivo de consulta deve aceitar `FILTER`, `SIM`, `ACCEPT`, `ORDER`, `GROUP` e `CROSS`. A saída CSV deve preservar o cabeçalho, as colunas, a ordem e o delimitador da entrada.
- A Parte 2 deve reutilizar os mesmos TADs, trocando apenas o leitor específico do dataset.
- A entrega também exige relatório do projeto, seis exemplos de execução, análise de complexidade e medições com pelo menos três execuções por configuração.

## Ordem de implementação sugerida

1. Definir uma interface única para `ITEM`, os leitores e os TADs, inclusive quem é dono da memória dos itens quando a base e o resultado apontam para o mesmo registro.
2. Corrigir a compilação e implementar leitura de CSV que suporte campos entre aspas, valores vazios e preservação da linha original para a saída.
3. Implementar o parser de consulta e o cálculo de similaridade, com testes pequenos e previsíveis.
4. Integrar as oito estruturas à carga, busca, inserção de resultados e exportação; verificar cada combinação de `--base` e `--result`.
5. Definir e implementar o leitor da Parte 2. O CSV atual combina uma tabela de municípios com outra de estados na mesma linha; a escolha dos campos do `ITEM` deve ser documentada.
6. Medir tempos, produzir exemplos e escrever os dois relatórios.

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
