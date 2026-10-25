#!/bin/sh
# Executar da raiz: make && sh tests/test_integracao.sh
# Opcional: passar o caminho de um executável compilado com sanitizadores.
set -eu
export LC_ALL=C
cd "$(dirname "$0")/.."
programa=${1:-./trabalho}
temporario=$(mktemp -d "${TMPDIR:-/tmp}/test_integracao.XXXXXX")
trap 'rm -rf "$temporario"' EXIT HUP INT TERM

falhar() {
    echo "test_integracao: $*" >&2
    exit 1
}

# Os exemplos artificiais têm campos simples, sem vírgulas internas.
# Isso permite calcular os resultados esperados com awk, sem usar o parser C.
head -n 1 data/asteroids.csv > "$temporario/nasa.csv"
awk 'BEGIN {
    for (i=1; i<=65; i++)
        printf "R%03d,Asteroide%03d,2026-01-01,%d,%d,%.3f,15,20,1,False,%s,%.2f,0,G%d,ok,False,,,,\n", \
            i,i,2000+i,i%4+1,i/1000,(i%2==0 ? "True" : "False"),i/100,i%3
}' >> "$temporario/nasa.csv"
head -n 1 data/mapa_empresas.csv > "$temporario/empresas.csv"
awk 'BEGIN {
    for (i=1; i<=65; i++)
        printf "D%d,G,%05d,Grupo%d,%d,%d,%d,%d,,S,T,%02d,Estado%d,1,2,3,4\n", \
            i%4,i,i%3,2*i,i,10*i,i,i%2,i%2
}' >> "$temporario/empresas.csv"

cat > "$temporario/nasa.query" <<'EOF'
FILTER|is_future_event|==|true
FILTER|distance_au|<=|0.06
SIM|velocity_km_s|15|10|1
SIM|risk_score|1|1|2
ACCEPT|0.61
ORDER|similarity|DESC
GROUP|threat_category|month
CROSS|threat_category|month
EOF
cat > "$temporario/empresas.query" <<'EOF'
FILTER|Empresas Ativas|>=|420
FILTER|Empresas Ativas|<=|600
SIM|Tempo Médio de Abertura|0|100|1
ACCEPT|0.45
ORDER|similarity|DESC
GROUP|nome|dt_referencia
CROSS|nome|dt_referencia
EOF

# Conferir as oito linhas, os contadores e a representação em milissegundos.
conferir_metricas() {
    awk -F= -v entrada="$1" -v aceitos="$2" -v escritos="$3" '
        BEGIN { split("STATUS INPUT_RECORDS MATCHED_RECORDS OUTPUT_RECORDS LOAD_MS SEARCH_MS RESULT_INSERT_MS WRITE_MS", nomes, " ") }
        $1 != nomes[NR] { exit 1 }
        NR==1 && $2!="OK" { exit 1 }
        NR==2 && $2!=entrada { exit 1 }
        NR==3 && $2!=aceitos { exit 1 }
        NR==4 && $2!=escritos { exit 1 }
        NR>=5 && $2 !~ /^[0-9]+\.[0-9][0-9][0-9]$/ { exit 1 }
        END { if (NR!=8) exit 1 }
    ' "$temporario/status" || falhar "métricas inesperadas"
}

executar() {
    "$programa" --reader "$leitor" --input "$entrada" \
        --base "$base" --result "$resultado" --query "$consulta" \
        --output "$temporario/saida.csv" --limit 65 --top-k "$1" \
        > "$temporario/status"
}

estruturas='pilha fila lista_seq lista_enc lista_cabeca lista_ord lista_gen lista_cruz'
for leitor in nasa empresas; do
    entrada="$temporario/$leitor.csv"
    consulta="$temporario/$leitor.query"
    if [ "$leitor" = nasa ]; then
        # SIM = (1 + 2*i/100)/3; filtros e limiar aceitam pares de 42 a 60.
        awk -F, 'NR>1 { i=substr($1,2)+0; if(i>=42 && i<=60 && i%2==0) print }' \
            "$entrada" > "$temporario/esperado"
        sort -t, -k1,1r "$temporario/esperado" > "$temporario/ordenado"
        aceitos=10
    else
        # SIM = 1-i/100; contagens e limiar aceitam registros de 42 a 55.
        awk -F, 'NR>1 && $3+0>=42 && $3+0<=55' "$entrada" > "$temporario/esperado"
        sort -t, -k3,3 "$temporario/esperado" > "$temporario/ordenado"
        aceitos=14
    fi
    sort "$temporario/esperado" > "$temporario/conjunto"
    for base in $estruturas; do
        for resultado in $estruturas; do
            executar 0
            conferir_metricas 65 "$aceitos" "$aceitos"
            head -n 1 "$entrada" > "$temporario/cabecalho"
            head -n 1 "$temporario/saida.csv" > "$temporario/cabecalho_saida"
            cmp "$temporario/cabecalho" "$temporario/cabecalho_saida"
            tail -n +2 "$temporario/saida.csv" > "$temporario/registros"
            sort "$temporario/registros" > "$temporario/obtido"
            cmp "$temporario/conjunto" "$temporario/obtido"
            if [ "$resultado" = lista_ord ]; then
                cmp "$temporario/ordenado" "$temporario/registros"
            fi
            # FIFO preserva a chegada; uma pilha na base ou nos resultados a inverte.
            if [ "$base" = fila ] && [ "$resultado" = fila ]; then
                cmp "$temporario/esperado" "$temporario/registros"
            fi
            if { [ "$base" = pilha ] && [ "$resultado" = fila ]; } ||
               { [ "$base" = fila ] && [ "$resultado" = pilha ]; }; then
                awk '{ linha[NR]=$0 } END { for(i=NR;i>0;i--) print linha[i] }' \
                    "$temporario/esperado" > "$temporario/invertido"
                cmp "$temporario/invertido" "$temporario/registros"
            fi
            head -n 4 "$temporario/saida.csv" > "$temporario/primeiros"
            executar 3
            conferir_metricas 65 "$aceitos" 3
            cmp "$temporario/primeiros" "$temporario/saida.csv"
        done
    done
    echo "$leitor: 64 combinações, seleção, ORDER, FIFO/LIFO e top-k: OK"
done

# Sem filtros e com ambos os limites zero, a passagem por filas deve preservar
# os datasets reais completos, byte a byte, incluindo o cabeçalho.
for leitor in nasa empresas; do
    if [ "$leitor" = nasa ]; then
        entrada=data/asteroids.csv
        quantidade=89227
    else
        entrada=data/mapa_empresas.csv
        quantidade=5570
    fi
    "$programa" --reader "$leitor" --input "$entrada" --base fila --result fila \
        --output "$temporario/saida.csv" --limit 0 --top-k 0 > "$temporario/status"
    conferir_metricas "$quantidade" "$quantidade" "$quantidade"
    cmp "$entrada" "$temporario/saida.csv"
    echo "$leitor: dataset completo ($quantidade registros), CSV idêntico: OK"
done

# Uma consulta válida sem correspondências ainda deve exportar o cabeçalho.
printf 'FILTER|year|<|0\n' > "$temporario/vazia.query"
leitor=nasa
entrada="$temporario/nasa.csv"
consulta="$temporario/vazia.query"
base=fila
resultado=fila
executar 0
conferir_metricas 65 0 0
head -n 1 "$entrada" > "$temporario/cabecalho"
cmp "$temporario/cabecalho" "$temporario/saida.csv"

# Falhas precisam retornar código diferente de zero e não publicar STATUS=OK.
rejeitar() {
    if "$programa" "$@" > "$temporario/status" 2> "$temporario/erro"; then
        falhar "entrada inválida aceita: $*"
    fi
    test ! -s "$temporario/status" || falhar "resumo de sucesso em falha"
    test -s "$temporario/erro" || falhar "falha sem diagnóstico"
    # Um erro de memória não pode ser confundido com a rejeição esperada.
    if grep -Eq 'AddressSanitizer|LeakSanitizer|UndefinedBehaviorSanitizer|runtime error:' "$temporario/erro"; then
        cat "$temporario/erro" >&2
        falhar "sanitizador detectou erro"
    fi
}
rejeitar --limit abc
rejeitar --limit -1
rejeitar --limit 9999999999999999999999999
rejeitar --limit 1 --limit 2
rejeitar --reader
rejeitar --desconhecida x
rejeitar --reader nasa --input "$temporario/inexistente" --base fila --result fila --output "$temporario/saida.csv"
printf 'SIM|year|2026|0|1\n' > "$temporario/invalida.query"
rejeitar --reader nasa --input "$entrada" --base fila --result fila --query "$temporario/invalida.query" --output "$temporario/saida.csv"
rejeitar --reader nasa --input "$entrada" --base fila --result lista_gen --output "$temporario/saida.csv"
rejeitar --reader nasa --input "$entrada" --base fila --result lista_cruz --output "$temporario/saida.csv"
head -n 1 "$entrada" > "$temporario/malformado.csv"
printf 'registro,incompleto\n' >> "$temporario/malformado.csv"
rejeitar --reader nasa --input "$temporario/malformado.csv" --base fila --result fila --output "$temporario/saida.csv"
if [ -c /dev/full ]; then
    rejeitar --reader nasa --input "$entrada" --base fila --result fila --output /dev/full --limit 1
fi
echo 'test_integracao: OK'
