#include "query.h"
#include "lista_cruz.h"
#include "lista_gen.h"
#include "lista_ord.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static const COLUNA colunas[] = {
    {"is_future_event", CAMPO_BOOLEANO}, {"distance_au", CAMPO_REAL},
    {"velocity_km_s", CAMPO_REAL}, {"risk_score", CAMPO_REAL},
    {"threat_category", CAMPO_TEXTO}, {"year", CAMPO_INTEIRO}
};

static void set_real(ITEM *item, size_t i, double v) {
    CAMPO c = {.tipo=CAMPO_REAL, .ausente=false, .valor.real=v};
    assert(item_definir_campo(item,i,&c));
}
static void coletar(ITEM *item,void *ctx){ITEM **p=ctx;while(*p)p++;*p=item;}

int main(void) {
    ESQUEMA *e=esquema_criar(colunas,sizeof(colunas)/sizeof(colunas[0]));
    assert(e);
    FILE *f=tmpfile();assert(f);
    const char *q="FILTER|is_future_event|==|true\n"
        "FILTER|distance_au|<=|0.08\n"
        "SIM|velocity_km_s|15.0|10.0|1.0\n"
        "SIM|risk_score|0.70|0.30|2.0\n"
        "ACCEPT|0.60\nORDER|similarity|DESC\n"
        "GROUP|threat_category|year\nCROSS|threat_category|year\n";
    assert(fwrite(q,1,strlen(q),f)==strlen(q));rewind(f);
    QUERY *query=NULL;assert(query_ler(f,e,&query));
    size_t n=0;const INSTRUCAO *in=query_instrucoes(query,&n);assert(n==8);
    assert(in[5].ordenar_por_similaridade&&!in[5].ascendente);
    assert(in[6].quantidade_campos==2&&in[6].indices_campos[0]==4&&in[6].indices_campos[1]==5);
    assert(in[7].quantidade_campos==2&&in[7].indices_campos[0]==4&&in[7].indices_campos[1]==5);
    ITEM *a=item_criar(e),*b=item_criar(e);assert(a&&b);
    CAMPO yes={.tipo=CAMPO_BOOLEANO,.ausente=false,.valor.booleano=true};
    assert(item_definir_campo(a,0,&yes)&&item_definir_campo(b,0,&yes));
    set_real(a,1,0.07);set_real(a,2,15.0);set_real(a,3,0.70);
    set_real(b,1,0.07);set_real(b,2,20.0);set_real(b,3,1.00);
    CAMPO texto_a={.tipo=CAMPO_TEXTO,.ausente=false,.valor.texto="SAFE"};
    CAMPO texto_b={.tipo=CAMPO_TEXTO,.ausente=false,.valor.texto="RISK"};
    CAMPO ano={.tipo=CAMPO_INTEIRO,.ausente=false,.valor.inteiro=2026};
    assert(item_definir_campo(a,4,&texto_a)&&item_definir_campo(b,4,&texto_b));
    assert(item_definir_campo(a,5,&ano)&&item_definir_campo(b,5,&ano));
    assert(fabs(query_item_similaridade(query,a)-1.0)<1e-12);
    assert(fabs(query_item_similaridade(query,b)-(0.5/3.0))<1e-12);
    assert(query_item_aceito(query,a)&&!query_item_aceito(query,b));
    assert(query_item_comparar(query,a,b)<0);

    LISTA_ORD *ord=lista_ord_criar();assert(ord);
    assert(lista_ord_ordenar_comparador(ord,query_comparador_ordenacao,query));
    assert(lista_ord_inserir(ord,b)&&lista_ord_inserir(ord,a));
    ITEM *ordem[3]={0};lista_ord_percorrer(ord,coletar,ordem);assert(ordem[0]==a&&ordem[1]==b);
    lista_ord_apagar(&ord);
    LISTA_GEN *gen=lista_gen_criar();assert(gen);
    assert(lista_gen_configurar_grupos(gen,in[6].indices_campos,in[6].quantidade_campos));
    assert(lista_gen_inserir_atomo(gen,a)&&lista_gen_inserir_atomo(gen,b));
    ITEM *agrupados[3]={0};lista_gen_percorrer(gen,coletar,agrupados);assert(agrupados[0]&&agrupados[1]);
    lista_gen_apagar(&gen);
    LISTA_CRUZ *cruz=lista_cruz_criar(in[7].indices_campos[0],in[7].indices_campos[1]);assert(cruz);
    assert(lista_cruz_inserir(cruz,a)&&lista_cruz_inserir(cruz,b));
    ITEM *linhas[3]={0},*colunas_visitadas[3]={0};
    lista_cruz_percorrer(cruz,CRUZ_POR_LINHA,coletar,linhas);
    lista_cruz_percorrer(cruz,CRUZ_POR_COLUNA,coletar,colunas_visitadas);
    assert(linhas[0]&&linhas[1]&&colunas_visitadas[0]&&colunas_visitadas[1]);lista_cruz_apagar(&cruz);
    item_apagar(&a);item_apagar(&b);query_liberar(&query);esquema_apagar(&e);fclose(f);

    e=esquema_criar(colunas,sizeof(colunas)/sizeof(colunas[0]));assert(e);
    f=tmpfile();assert(f);assert(fputs("SIM|distance_au|1|0|1\n",f)>=0);rewind(f);
    assert(!query_ler(f,e,&query));
    fclose(f);esquema_apagar(&e);
    puts("test_query: OK");return 0;
}
