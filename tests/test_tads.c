#include "fila.h"
#include "estrutura.h"
#include "lista_cabeca.h"
#include "lista_cruz.h"
#include "lista_enc.h"
#include "lista_gen.h"
#include "lista_ord.h"
#include "lista_seq.h"
#include "pilha.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct { ITEM *itens[64]; size_t n; } COLETA;
static void coletar(ITEM *item, void *ctx) {
    COLETA *c=ctx;assert(c->n<64);c->itens[c->n++]=item;
}
static ITEM *criar_item(const ESQUEMA *e,const char *grupo,long long ano) {
    ITEM *i=item_criar(e);assert(i);
    CAMPO g={.tipo=CAMPO_TEXTO,.ausente=false,.valor.texto=grupo};
    CAMPO a={.tipo=CAMPO_INTEIRO,.ausente=false,.valor.inteiro=ano};
    assert(item_definir_campo(i,0,&g)&&item_definir_campo(i,1,&a));
    return i;
}
static void conferir(COLETA *c,ITEM **esperado,size_t n) {
    assert(c->n==n);
    for(size_t i=0;i<n;i++)assert(c->itens[i]==esperado[i]);
}

int main(void) {
    COLUNA colunas[]={{"grupo",CAMPO_TEXTO},{"ano",CAMPO_INTEIRO}};
    ESQUEMA *e=esquema_criar(colunas,2);assert(e);
    ITEM *itens[12];
    for(size_t i=0;i<12;i++)itens[i]=criar_item(e,i%2?"B":"A",2000+(long long)(i%3));
    COLETA c={0};

    PILHA *p=pilha_criar();assert(p);for(size_t i=0;i<12;i++)assert(pilha_empilhar(p,itens[i]));
    pilha_percorrer(p,coletar,&c);assert(c.n==12&&c.itens[0]==itens[11]);
    assert(pilha_desempilhar(p)==itens[11]);pilha_apagar(&p);
    assert(item_campo(itens[0],1)->valor.inteiro==2000);c=(COLETA){0};

    FILA *f=fila_criar();assert(f);for(size_t i=0;i<12;i++)assert(fila_inserir(f,itens[i]));
    fila_percorrer(f,coletar,&c);assert(c.n==12&&c.itens[0]==itens[0]);
    assert(fila_remover(f)==itens[0]);fila_apagar(&f);c=(COLETA){0};

    LISTA_SEQ *s=lista_seq_criar();assert(s);for(size_t i=0;i<12;i++)assert(lista_seq_inserir(s,itens[i]));
    assert(lista_seq_tamanho(s)==12);lista_seq_percorrer(s,coletar,&c);conferir(&c,itens,12);
    assert(lista_seq_remover(s,4)==itens[4]);lista_seq_apagar(&s);c=(COLETA){0};

    LISTA_ENC *le=lista_enc_criar(false);assert(le&&lista_enc_ordenar_por(le,1,true));
    for(size_t i=0;i<12;i++)assert(lista_enc_inserir(le,itens[i]));
    lista_enc_percorrer(le,coletar,&c);assert(c.n==12);
    for(size_t i=1;i<c.n;i++)assert(item_comparar_campo(c.itens[i-1],c.itens[i],1)<=0);
    assert(lista_enc_remover(le,itens[3])==itens[3]);lista_enc_apagar(&le);c=(COLETA){0};

    LISTA_ORD *lo=lista_ord_criar();assert(lo&&lista_ord_ordenar_por(lo,1,false));
    for(size_t i=0;i<12;i++)assert(lista_ord_inserir(lo,itens[i]));
    lista_ord_percorrer(lo,coletar,&c);assert(c.n==12);
    for(size_t i=1;i<c.n;i++)assert(item_comparar_campo(c.itens[i-1],c.itens[i],1)>=0);
    assert(lista_ord_remover_item(lo,itens[0])==itens[0]);lista_ord_apagar(&lo);c=(COLETA){0};

    LISTA_CABECA *lc=lista_cabeca_criar();assert(lc);
    for(size_t i=0;i<12;i++)assert(lista_cabeca_inserir(lc,itens[i]));
    lista_cabeca_percorrer(lc,coletar,&c);conferir(&c,itens,12);
    assert(lista_cabeca_remover(lc,itens[0])==itens[0]);lista_cabeca_apagar(&lc);c=(COLETA){0};

    size_t grupos[]={0,1};LISTA_GEN *lg=lista_gen_criar();assert(lg);
    assert(lista_gen_configurar_grupos(lg,grupos,2));
    for(size_t i=0;i<12;i++)assert(lista_gen_inserir_atomo(lg,itens[i]));
    assert(lista_gen_profundidade(lg)==3);
    lista_gen_percorrer(lg,coletar,&c);assert(c.n==12);
    assert(lista_gen_contem_item(lg,itens[8]));assert(lista_gen_remover_item(lg,itens[8])==itens[8]);
    lista_gen_apagar(&lg);c=(COLETA){0};

    LISTA_CRUZ *lx=lista_cruz_criar(0,1);assert(lx);
    for(size_t i=0;i<12;i++)assert(lista_cruz_inserir(lx,itens[i]));
    assert(!lista_cruz_inserir(lx,itens[0]));
    assert(lista_cruz_grupos(lx,CRUZ_POR_LINHA)==2);
    assert(lista_cruz_grupos(lx,CRUZ_POR_COLUNA)==3);
    lista_cruz_percorrer(lx,CRUZ_POR_LINHA,coletar,&c);assert(c.n==12);
    c=(COLETA){0};lista_cruz_percorrer(lx,CRUZ_POR_COLUNA,coletar,&c);assert(c.n==12);
    assert(lista_cruz_buscar(lx,itens[3])==itens[3]);
    assert(lista_cruz_remover(lx,itens[3])==itens[3]);
    for(size_t i=0;i<12;i++)if(i!=3)assert(lista_cruz_remover(lx,itens[i])==itens[i]);
    assert(lista_cruz_grupos(lx,CRUZ_POR_LINHA)==0&&lista_cruz_grupos(lx,CRUZ_POR_COLUNA)==0);
    assert(lista_cruz_inserir(lx,itens[3])&&lista_cruz_buscar(lx,itens[3])==itens[3]);
    lista_cruz_apagar(&lx);

    FILE *qf=tmpfile();assert(qf);
    assert(fputs("ORDER|similarity|DESC\nGROUP|grupo|ano\nCROSS|grupo|ano\n",qf)>=0);rewind(qf);
    QUERY *query=NULL;assert(query_ler(qf,e,&query));
    const char *nomes[]={"pilha","fila","lista_seq","lista_enc","lista_cabeca","lista_ord","lista_gen","lista_cruz"};
    for(size_t i=0;i<sizeof(nomes)/sizeof(nomes[0]);i++){
        ESTRUTURA *resultado=estrutura_criar(nomes[i],ESTRUTURA_RESULTADOS,query);assert(resultado);
        assert(estrutura_inserir(resultado,itens[0])&&estrutura_tamanho(resultado)==1);
        assert(estrutura_remover(resultado,itens[0])==itens[0]);estrutura_apagar(&resultado);
        assert(item_campo(itens[0],1)->valor.inteiro==2000);
    }
    ESTRUTURA *base=estrutura_criar("lista_cruz",ESTRUTURA_BASE,query);assert(base);
    ITEM *dono_a=criar_item(e,"D",2040),*dono_b=criar_item(e,"E",2041);
    assert(estrutura_inserir(base,dono_a)&&estrutura_inserir(base,dono_b));
    assert(estrutura_tamanho(base)==2);
    ESTRUTURA *resultado=estrutura_criar("lista_gen",ESTRUTURA_RESULTADOS,query);assert(resultado);
    assert(estrutura_inserir(resultado,dono_a)&&estrutura_inserir(resultado,dono_b));
    estrutura_apagar(&resultado);
    assert(item_campo(dono_a,1)->valor.inteiro==2040); /* resultado só referencia */
    estrutura_apagar(&base); /* base libera cada registro exatamente uma vez */
    for(size_t i=0;i<sizeof(nomes)/sizeof(nomes[0]);i++){
        ESTRUTURA *base_tad=estrutura_criar(nomes[i],ESTRUTURA_BASE,query);assert(base_tad);
        ITEM *registro=criar_item(e,"BASE",2100+(long long)i);
        assert(estrutura_inserir(base_tad,registro)&&estrutura_tamanho(base_tad)==1);
        estrutura_apagar(&base_tad);
    }
    query_liberar(&query);fclose(qf);

    /* Destrutores liberam os nós, nunca os ITEMs emprestados. */
    for(size_t i=0;i<12;i++)item_apagar(&itens[i]);
    esquema_apagar(&e);puts("test_tads: OK");return 0;
}
