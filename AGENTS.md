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

- A função principal está em `src/main.c`, após os módulos. `src/argumentos.c` lê e valida as opções de linha de comando.
- `lib/item.h` e `src/item.c` implementam um ITEM genérico com esquema compartilhado e vetor de campos tipados. O esquema copia os nomes das colunas; os itens copiam seus textos e apenas referenciam o esquema. Os oito TADs agora armazenam ITEMs genéricos e percorrem referências sem liberar registros. `lib/estrutura.h` fornece uma fachada comum que diferencia propriedade da base e referências de resultados. O item também possui uma cópia opcional do registro original, invalidada ao modificar campos e liberada junto ao item.
- O parser em `query.c` segue a sintaxe oficial separada por `|`. `query_item_similaridade` calcula a média ponderada dos critérios SIM; `query_item_comparar` implementa ORDER e aceita `similarity`; GROUP expõe índices para `lista_gen`, e CROSS fornece os dois índices para `lista_cruz`. `src/estrutura.c` conecta esses metadados às estruturas.
- A base é dona dos registros após inserção bem-sucedida. Resultados apenas os referenciam e não devem liberá-los. Após reader_ler entregar um item, o chamador é responsável por ele até transferi-lo à base; se essa inserção falhar, o chamador deve liberá-lo, não o leitor automaticamente. Uma tentativa de reinserir um ponteiro já pertencente à base não devolve sua propriedade ao chamador. Liberar resultados antes dos registros da base e liberar o leitor/esquema por último. Remover da base transfere a propriedade ao chamador, que deve preservar o item enquanto houver referências de resultados.
- Leitura implementada em `csv.c`, `reader.c`, `reader_nasa.c` e `reader_empresas.c`: os construtores retornam um READER que possui esquema e cabeçalho original, e empresta o FILE. O esquema é acessível por reader_esquema. O módulo CSV separa os campos; descritores específicos atribuem os tipos, validam o cabeçalho e permitem construir o esquema com um vetor temporário de COLUNA. Esse vetor é liberado após a cópia, inclusive em falhas.
- A criação do leitor consome o cabeçalho, deixando o arquivo no primeiro registro, sem fechar o FILE. reader_ler entrega um ITEM por chamada e distingue sucesso, fim e erro; o chamador controla --limit e insere imediatamente na base. O leitor não acumula os registros. Manter o leitor vivo enquanto existirem itens, pois ele possui o esquema compartilhado.
- A exportação usa o cabeçalho preservado no leitor e o original guardado no item. Não exportar silenciosamente um original invalidado por alteração dos campos. O chamador deve verificar também erros adiados no fflush/fclose da saída.
- O leitor de empresas mantém as 17 colunas: bloco municipal, separadora (_separador no esquema) e bloco estadual independente (prefixo estado. no esquema). Os nomes internos não alteram o cabeçalho exportado. Não inferir relação entre o município e o estado na mesma linha. Códigos IBGE são texto; '-' nos tempos representa ausência.

## Estado atual verificado em 24/10/2026

- Os módulos de item, CSV e leitores já estão implementados. O CSV trata aspas escapadas, delimitadores e quebras de linha dentro de campos entre aspas, campos vazios e registros sem quebra de linha final. Os leitores validam os cabeçalhos esperados e fazem conversões tipadas; não inferem automaticamente o esquema de um CSV arbitrário.
- `src/reader_privado.h` é uma interface interna dos leitores. READER_COLUNA descreve nome esperado na entrada, nome interno, tipo e regra de ausência. COLUNA descreve nome/tipo no esquema; CAMPO contém o valor de um registro. Não confundir descritores estáticos, vetor temporário de criação do esquema e campos de cada item.
- `src/argumentos.c` valida as oito opções de linha de comando, incluindo valores numéricos, opções desconhecidas ou repetidas, valores ausentes e textos longos. `src/main.c` valida as opções e coordena leitores, estruturas, consulta, busca e exportação.
- `src/query.c` e `lib/query.h` implementam FILTER, SIM, ACCEPT, ORDER, GROUP e CROSS separados por `|`, incluindo nomes de campos com espaços, média ponderada e ORDER por similarity. O parser resolve nomes contra o esquema e rejeita campos inexistentes. O fluxo está integrado no aplicativo; ainda não há cobertura exaustiva de todos os casos de erro.
- Os oito TADs usam ITEM genérico e liberam apenas sua própria organização. `src/estrutura.c` implementa a fachada de base/resultados. A lista com cabeça possui sentinela real; a ordenada mantém a ordem na inserção; a generalizada usa grupos/subgrupos; a cruzada mantém dois eixos sem duplicar o ITEM. As listas sequencial e ordenada começam com capacidade 16 e dobram via realloc; a cruzada começa com hash de 32 posições e o amplia acima de 75% de ocupação. Permanecem as pendências específicas da revisão abaixo.
- `makefile` compila os fontes de `src/` com `-Wall -Wextra -pedantic` e gera o executável `trabalho`. `src/main.c` abre e fecha os arquivos; os leitores emprestam o `FILE`. Depois de terminar a leitura, o arquivo de entrada pode ser fechado; o leitor permanece vivo até os itens serem liberados, pois possui o esquema.
- Estão versionados `tests/test_csv.c`, `tests/test_reader.c`, `tests/test_query.c` e `tests/test_tads.c`. A pasta tests não está ignorada; o `.gitignore` atual ignora apenas referencia. Os testes possuem suas próprias funções main, independentes da futura função principal do aplicativo; podem ser compilados diretamente com o compilador, sem Makefile, conforme README.
- Validação histórica registrada: os módulos de leitura passaram com `-Wall -Wextra -pedantic -Werror` e sanitizadores, incluindo leitura/exportação dos 89.227 registros de asteroides e 5.570 registros de empresas com comparação byte a byte. Isso não revalida alterações posteriores nem os TADs atuais. A revisão anterior também registrou verificação de sintaxe de todos os fontes; sintaxe não comprova ligação ou execução completa.
- Em 24/10/2026, `make` compilou o executável; os quatro testes existentes passaram com `-Werror` e sanitizadores. O fluxo avançado passou com `lista_cruz` na base e `lista_gen` nos resultados. As 64 combinações de base/result também passaram com uma consulta contendo GROUP e CROSS, em amostras de 25 registros. Isso não substitui a revisão de contratos e a ampliação de cobertura listadas abaixo.
- Reavaliar arquivos e estado do Git antes de novas alterações: colegas podem atualizar o repositório em outros chats ou no GitHub.

## Revisão dos contratos em 23/10/2026

O enunciado e os exemplos locais do professor de lista sequencial ordenada dinâmica, nó cabeça e percurso encadeado foram consultados. Os exemplos liberam ITEMs nos próprios TADs; neste projeto essa responsabilidade foi centralizada na fachada BASE para permitir resultados emprestados.

Em 23/10/2026, a fachada passou a reservar cada ITEM para uma única BASE, rejeitando ponteiros repetidos ou já pertencentes a outra base em tempo constante; uma falha de inserção desfaz a reserva e preserva a propriedade do chamador. Remover da BASE desfaz a reserva e transfere a propriedade ao chamador. RESULTADOS continuam aceitando referências emprestadas.

- **Reconfiguração de grupos:** `lista_gen_configurar_grupos` agora libera uma configuração anterior pertencente à lista somente depois de alocar e copiar a nova. Se a alocação falhar, a configuração antiga permanece. Vetores compartilhados pelas sublistas não são liberados por elas.
- **Índices e esquemas:** a fachada exige que todos os itens tenham identidade de esquema igual à primeira inserção e, quando há QUERY, igual ao esquema dela. A lista generalizada valida os índices configurados em cada item; a cruzada valida os dois índices e recusa esquemas diferentes. GROUP/CROSS padrão usam índices 0 e 1 e a fachada rejeita esquemas com menos de duas colunas. Continua pendente revisar uniformidade de esquema para uso direto de todos os TADs fora da fachada e explicitar validação em TADs ordenados configurados diretamente por campo.
- **Estabilidade dos campos:** os contratos públicos agora orientam preencher campos antes da inserção e não alterar campos usados por ORDER, GROUP ou CROSS enquanto o item participa dessas organizações; para alterar, remover das estruturas afetadas e reinserir. A QUERY emprestada à lista ordenada precisa permanecer viva até sua destruição ou reconfiguração. O contrato de falha de inserção identifica o chamador como responsável pelo ITEM.
- **Interrupção do percurso:** o visitante retorna void e os oito percursos continuam até o fim. Um contexto pode evitar novas escritas após erro/limite, mas não interrompe a navegação. Proposta: oferecer percurso com retorno que distinga continuar, parar e erro. Pelo enunciado, top-k limita os primeiros registros exportados na ordem de acesso dos RESULTADOS; não autoriza encerrar a busca da BASE após k aceitos, pois isso pode alterar resultados e contagens.
- **Cobertura insuficiente:** o teste de TADs usa 12 itens, abaixo da capacidade inicial 16 dos vetores e do limiar 24 do hash. Já cobre operações básicas, os dois eixos da cruzada, rejeição de repetição na cruzada e alguns cenários de propriedade via fachada, mas não testa as 64 combinações base/resultado, reconfiguração repetida, falhas de alocação, índices inválidos ou esquemas incompatíveis de forma abrangente.

### Próximos passos

1. Acrescentar testes de regressão para reconfiguração, reserva/liberação de propriedade, ponteiros duplicados, esquemas incompatíveis e índices inválidos; compilar e executar com avisos rigorosos e sanitizadores.
2. Revisar uniformidade de esquema e validação de índices nos TADs quando usados diretamente, fora da fachada.
3. Acrescentar percurso interrompível com semântica explícita de parada/erro, preservando a ordem própria de cada TAD e usando top-k na exportação dos resultados.
4. Ampliar testes de crescimento (16/17, 32/33 nos vetores e 24/25 no hash), remoção de representantes, falhas de alocação e propriedade nas 64 combinações base/resultado.
5. Concluir a revisão dos módulos, preparar as consultas e exemplos exigidos e produzir os relatórios do projeto e experimental.

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
