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
- A base é dona dos registros após inserção bem-sucedida. Resultados apenas os referenciam e não devem liberá-los. Após reader_ler entregar um item, o chamador é seu dono até transferi-lo à base; se a inserção falhar, o chamador deve apagá-lo. Liberar resultados antes dos registros da base e liberar o esquema por último.
- Leitura implementada em `csv.c`, `reader.c`, `reader_nasa.c` e `reader_empresas.c`: os construtores retornam um READER que possui esquema e cabeçalho original, mas apenas recebe um FILE emprestado do chamador. O esquema é acessível por reader_esquema. O módulo CSV separa os campos; descritores específicos atribuem os tipos, validam o cabeçalho e permitem construir o esquema com um vetor temporário de COLUNA. Esse vetor é liberado após a cópia, inclusive em falhas.
- A criação do leitor consome o cabeçalho, deixando o arquivo no primeiro registro, sem fechar o FILE. reader_ler entrega um ITEM por chamada e distingue sucesso, fim e erro; o chamador controla --limit e insere imediatamente na base. O leitor não acumula os registros. Manter o leitor vivo enquanto existirem itens, pois ele possui o esquema compartilhado.
- A exportação usa o cabeçalho preservado no leitor e o original guardado no item. Não exportar silenciosamente um original invalidado por alteração dos campos. O chamador deve verificar também erros adiados no fflush/fclose da saída.
- O leitor de empresas mantém as 17 colunas: bloco municipal, separadora (_separador no esquema) e bloco estadual independente (prefixo estado. no esquema). Os nomes internos não alteram o cabeçalho exportado. Não inferir relação entre o município e o estado na mesma linha. Códigos IBGE são texto; '-' nos tempos representa ausência.

## Estado atual verificado em 23/10/2026

- Os módulos de item, CSV e leitores já estão implementados. O CSV trata aspas escapadas, delimitadores e quebras de linha dentro de campos entre aspas, campos vazios e registros sem quebra de linha final. Os leitores validam os cabeçalhos esperados e fazem conversões tipadas; não inferem automaticamente o esquema de um CSV arbitrário.
- `src/reader_privado.h` é uma interface interna dos leitores. READER_COLUNA descreve nome esperado na entrada, nome interno, tipo e regra de ausência. COLUNA descreve nome/tipo no esquema; CAMPO contém o valor de um registro. Não confundir descritores estáticos, vetor temporário de criação do esquema e campos de cada item.
- `src/cliente.c` e `lib/cliente.h` agora incluem --limit, além de leitura, avaliação e liberação de filtros simples. O formato implementado é `FILTER campo operador valor`, separado por espaços, e ainda não atende ao formato `FILTER|campo|operador|valor` exigido pelo enunciado. Também não resolve nomes de campos com espaços, como os de contagens do dataset de empresas. Não tratar essa implementação parcial como linguagem de consulta concluída.
- `src/query.c` e `lib/query.h` continuam vazios. Ainda faltam a linguagem completa de consulta, similaridade e integração de ORDER, GROUP e CROSS às estruturas.
- Os TADs de estruturas ainda precisam de revisão e adaptação às interfaces atuais e à propriedade da memória de base/resultados. Há chamadas antigas e inconsistências; a lista sequencial ainda tem capacidade fixa de 10 itens. Não considerar o programa completo compilável só porque os módulos de leitura passaram em testes isolados.
- `makefile` ainda contém apenas um comentário. A função principal do aplicativo e a integração completa permanecem pendentes, conforme a ordem de trabalho acordada.
- Abrir e fechar arquivos cabe ao chamador/coordenador, não aos leitores. Essa coordenação poderá ficar no cliente, mas ainda não está implementada ali. Depois de terminar a leitura, o arquivo de entrada pode ser fechado; o leitor deve permanecer vivo até os itens serem liberados, pois possui o esquema.
- `tests/test_csv.c` e `tests/test_reader.c` estão versionados. A pasta tests não está ignorada; o `.gitignore` atual ignora apenas referencia. Os testes possuem suas próprias funções main, independentes da futura função principal do aplicativo; podem ser compilados diretamente com o compilador, sem Makefile, conforme README.
- Na validação anterior desta conversa, os módulos de leitura passaram com `-Wall -Wextra -pedantic -Werror` e sanitizadores, incluindo leitura/exportação dos 89.227 registros de asteroides e 5.570 registros de empresas com comparação byte a byte. Isso registra uma execução anterior, não uma revalidação automática de alterações posteriores ao merge.
- Reavaliar arquivos e estado do Git antes de novas alterações: colegas podem atualizar o repositório em outros chats ou no GitHub.

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

Esta seção é somente histórica. Para planejar trabalho novo, usar o estado atual acima e conferir o código.

- Havia implementações parciais em `src/` e interfaces em `lib/`, com erros de sintaxe e inconsistências. Os módulos CSV e consulta estavam vazios; o makefile só continha um comentário.
- O leitor de asteroides separava campos por vírgula simples. Essa limitação foi resolvida pela implementação do módulo CSV e dos novos leitores; não permanece como pendência atual.
- O formato de empresas, com tabelas de municípios e estados lado a lado, ainda precisava ser interpretado. A organização adotada está documentada na arquitetura atual.
