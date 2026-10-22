# Contexto do Projeto 1 de SCC0202

Este arquivo registra informações duráveis para futuros chats e ferramentas que trabalham neste repositório. Leia `referencia/descricao-projeto1.pdf` para a especificação completa; ela prevalece sobre resumos aqui e no `README.md`.

## Fontes de referência

- A pasta `referencia/` é ignorada pelo Git e deve ser usada para armazenar os códigos-base fornecidos pelo professor e outros materiais de referência locais.
- Ao analisar, planejar ou implementar qualquer parte deste projeto, procure primeiro nessa pasta as fontes do professor que sejam relevantes para a tarefa. Não substitua essas fontes por suposições quando elas estiverem disponíveis.
- O enunciado completo está em `referencia/descricao-projeto1.pdf`.

## Situação e decisões já tomadas

- O projeto é um trabalho de Algoritmos e Estruturas de Dados I, em C, dividido em duas partes de 5 pontos cada.
- A Parte 1 usa o CSV de asteroides em `data/asteroid_dataset_20251019.csv`. É necessário usar exatamente a versão fornecida para a disciplina.
- O conjunto de dados da Parte 2 **já foi escolhido e confirmado pelo grupo**. O arquivo presente é `data/07_ArquivoDados_Jul26 (Mapa de Empresas).csv`; não trate a confirmação como pendência. Se for necessário conhecer detalhes da confirmação no e-Disciplinas, peça-os somente quando forem relevantes.
- O usuário usa Ubuntu 24.04 e VS Code e quer ajuda didática com Git/GitHub (incluindo `status`, `fetch`, `pull`, branches, commits e conflitos). Antes de operações Git que alteram o estado, confira o estado local e explique o efeito do comando em linguagem simples. Não presuma que alterações locais possam ser descartadas.
- A leitura inicial do projeto foi feita sem implementar código. Não inicie implementação por conta própria; siga a tarefa pedida em cada conversa. Quando o usuário pedir código, essa solicitação autoriza o trabalho correspondente.

## Arquitetura acordada e próximos passos

- A função principal será implementada por último, após os TADs estarem funcionais e consistentes. Não antecipar sua implementação.
- `lib/item.h` e `src/item.c` implementam um ITEM genérico com esquema compartilhado e vetor de campos tipados. O esquema copia os nomes das colunas; os itens copiam seus textos e apenas referenciam o esquema. Os demais TADs ainda precisam ser adaptados. O item também possui uma cópia opcional do registro original, invalidada ao modificar campos e liberada junto ao item.
- A base é dona dos registros após inserção bem-sucedida. Resultados apenas os referenciam e não devem liberá-los. Em falha de inserção, o leitor continua responsável pelo item. Liberar resultados antes dos registros da base e liberar o esquema por último.
- Leitura implementada em `csv.c`, `reader.c`, `reader_nasa.c` e `reader_empresas.c`: os construtores retornam um READER que possui esquema e cabeçalho original, e empresta o FILE. O esquema é acessível por reader_esquema. O módulo CSV separa os campos; descritores específicos atribuem os tipos, validam o cabeçalho e permitem construir o esquema com um vetor temporário de COLUNA. Esse vetor é liberado após a cópia, inclusive em falhas.
- A criação do leitor consome o cabeçalho, deixando o arquivo no primeiro registro, sem fechar o FILE. reader_ler entrega um ITEM por chamada e distingue sucesso, fim e erro; o chamador controla --limit e insere imediatamente na base. O leitor não acumula os registros. Manter o leitor vivo enquanto existirem itens, pois ele possui o esquema compartilhado.
- A exportação usa o cabeçalho preservado no leitor e o original guardado no item. Não exportar silenciosamente um original invalidado por alteração dos campos. O chamador deve verificar também erros adiados no fflush/fclose da saída.
- O leitor de empresas mantém as 17 colunas: bloco municipal, separadora (_separador no esquema) e bloco estadual independente (prefixo estado. no esquema). Os nomes internos não alteram o cabeçalho exportado. Não inferir relação entre o município e o estado na mesma linha. Códigos IBGE são texto; '-' nos tempos representa ausência.

## Resultado exigido pelo enunciado

- `make` deve gerar o executável `trabalho`, compilado com `-Wall -Wextra -pedantic`.
- A execução por linha de comando deve aceitar `--reader`, `--input`, `--base`, `--result`, `--query`, `--output`, `--top-k` e `--limit`. Valor zero em `--top-k` ou `--limit` significa sem limite.
- As opções de estrutura são `pilha`, `fila`, `lista_seq`, `lista_enc`, `lista_cabeca`, `lista_ord`, `lista_gen` e `lista_cruz`. O grupo deve implementar seus próprios TADs. A estrutura `--base` recebe os registros durante a leitura e é percorrida na busca; os registros aceitos entram na estrutura `--result`. Qualquer combinação válida deve funcionar.
- A lista com cabeça deve ter sentinela real; a lista ordenada deve manter a ordem durante a inserção; a lista generalizada deve organizar grupos e subgrupos; a lista cruzada deve permitir dois percursos sobre o mesmo registro sem duplicá-lo.
- O arquivo de consulta deve aceitar `FILTER`, `SIM`, `ACCEPT`, `ORDER`, `GROUP` e `CROSS`. A similaridade de cada critério numérico usa `max(0, 1 - abs(x - alvo) / tolerancia)` e a similaridade total é a média ponderada. Sem `SIM`, os filtros bastam.
- O CSV de saída deve preservar cabeçalho, colunas, ordem e delimitador da entrada, sem colunas auxiliares. A saída padrão deve informar status, contagens e tempos de carga, busca, inserção nos resultados e escrita.
- A Parte 2 deve reutilizar os mesmos TADs; apenas o leitor que transforma uma linha em `ITEM` pode ser específico do dataset. Exige pelo menos duas consultas úteis e demonstração de uma estrutura avançada (`lista_cabeca`, `lista_gen` ou `lista_cruz`).
- A entrega inclui código-fonte e Makefile, relatório do projeto com seis exemplos completos (três por parte) e relatório experimental. Este último deve analisar as operações realmente usadas, variar tamanho da entrada e `top-k`, executar cada configuração pelo menos três vezes, mostrar médias e comparar medições com complexidade assintótica.

## Estado inicial observado em 19/10/2026

- Há implementações parciais em `src/` e interfaces em `lib/`; o projeto ainda não compila como está. `src/csv.c`, `src/query.c`, `lib/csv.h` e `lib/query.h` estão vazios; `makefile` só contém um comentário.
- Há erros de sintaxe e inconsistências entre interfaces e implementações. O leitor de asteroides atual separa campos por vírgula simples e precisará lidar corretamente com CSV entre aspas, valores ausentes e preservação dos dados exportados.
- O CSV de empresas contém dados de municípios e, nas primeiras linhas, uma segunda tabela de estados após uma coluna vazia. Inspecione o formato antes de definir o leitor específico.
- Reavalie este retrato inicial antes de planejar ou alterar código: colegas podem mudar o repositório em outros chats ou no GitHub.
