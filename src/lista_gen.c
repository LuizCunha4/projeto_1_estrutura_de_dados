#include "lista_gen.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;
enum tipo_no { NO_ATOMO, NO_GRUPO };
struct no_ {
    enum tipo_no tipo;
    ITEM *item;                 /* átomo ou representante da chave do grupo */
    NO *proximo;
    LISTA_GEN *sublista;
};
struct lista_gen_ {
    NO *inicio, *fim;
    size_t quantidade, profundidade;
    size_t *campos;
    size_t n_campos;
    bool dono_campos;
};

LISTA_GEN *lista_gen_criar(void) { return calloc(1, sizeof(LISTA_GEN)); }

static bool vazia(const LISTA_GEN *l) { return l == NULL || l->inicio == NULL; }

void lista_gen_apagar(LISTA_GEN **lista) {
    if (lista == NULL || *lista == NULL) return;
    NO *n=(*lista)->inicio;
    while(n) { NO *prox=n->proximo; if(n->tipo==NO_GRUPO) lista_gen_apagar(&n->sublista); free(n); n=prox; }
    if((*lista)->dono_campos) free((*lista)->campos);
    free(*lista); *lista=NULL;
}

bool lista_gen_configurar_grupos(LISTA_GEN *l,const size_t *campos,size_t n) {
    if(l==NULL||campos==NULL||n<2||l->quantidade!=0||n>SIZE_MAX/sizeof(*campos)) return false;
    size_t *c=malloc(n*sizeof(*c)); if(c==NULL)return false;
    for(size_t i=0;i<n;i++)c[i]=campos[i];
    l->campos=c;l->n_campos=n;l->dono_campos=true;return true;
}

static LISTA_GEN *filha_criar(const LISTA_GEN *pai) {
    LISTA_GEN *f=lista_gen_criar(); if(f==NULL)return NULL;
    f->campos=pai->campos; f->n_campos=pai->n_campos;
    f->profundidade=pai->profundidade+1; return f;
}

static NO *novo_no(enum tipo_no tipo,ITEM *item) {
    NO *n=calloc(1,sizeof(*n));if(n){n->tipo=tipo;n->item=item;}return n;
}
static void anexar(LISTA_GEN*l,NO*n){if(l->fim)l->fim->proximo=n;else l->inicio=n;l->fim=n;++l->quantidade;}

bool lista_gen_inserir_atomo(LISTA_GEN *l,ITEM *item) {
    if(l==NULL||item==NULL)return false;
    if(l->profundidade<l->n_campos){
        size_t campo=l->campos[l->profundidade];
        for(NO*n=l->inicio;n;n=n->proximo) if(n->tipo==NO_GRUPO&&n->item!=NULL&&item_comparar_campo(n->item,item,campo)==0)
            return lista_gen_inserir_atomo(n->sublista,item);
        LISTA_GEN *sub=filha_criar(l);if(sub==NULL)return false;
        NO*n=novo_no(NO_GRUPO,item);if(n==NULL){lista_gen_apagar(&sub);return false;}
        n->sublista=sub;
        if(!lista_gen_inserir_atomo(sub,item)){free(n);lista_gen_apagar(&sub);return false;}
        anexar(l,n);return true;
    }
    NO*n=novo_no(NO_ATOMO,item);if(n==NULL)return false;anexar(l,n);return true;
}

LISTA_GEN *lista_gen_inserir_sublista(LISTA_GEN *l) {
    if(l==NULL)return NULL;
    LISTA_GEN*sub=filha_criar(l);if(sub==NULL)return NULL;
    NO*n=novo_no(NO_GRUPO,NULL);if(n==NULL){lista_gen_apagar(&sub);return NULL;}
    n->sublista=sub;anexar(l,n);return sub;
}

static const ITEM *primeiro_item(const LISTA_GEN*l){if(!l)return NULL;for(NO*n=l->inicio;n;n=n->proximo){if(n->tipo==NO_ATOMO)return n->item;const ITEM*i=primeiro_item(n->sublista);if(i)return i;}return NULL;}

bool lista_gen_contem_item(const LISTA_GEN*l,const ITEM*item){if(!l||!item)return false;for(NO*n=l->inicio;n;n=n->proximo){if(n->tipo==NO_ATOMO&&n->item==item)return true;if(n->tipo==NO_GRUPO&&lista_gen_contem_item(n->sublista,item))return true;}return false;}

static bool remover_rec(LISTA_GEN*l,ITEM*item){NO**elo=&l->inicio;while(*elo){NO*n=*elo;if(n->tipo==NO_ATOMO&&n->item==item){*elo=n->proximo;if(l->fim==n){l->fim=NULL;for(NO*p=l->inicio;p;p=p->proximo)l->fim=p;}free(n);--l->quantidade;return true;}if(n->tipo==NO_GRUPO&&remover_rec(n->sublista,item)){if(vazia(n->sublista)){*elo=n->proximo;if(l->fim==n){l->fim=NULL;for(NO*p=l->inicio;p;p=p->proximo)l->fim=p;}lista_gen_apagar(&n->sublista);free(n);--l->quantidade;}else if(n->item==item)n->item=(ITEM*)primeiro_item(n->sublista);return true;}elo=&n->proximo;}return false;}

ITEM *lista_gen_remover_item(LISTA_GEN*l,ITEM*item){return l&&item&&remover_rec(l,item)?item:NULL;}
int lista_gen_profundidade(const LISTA_GEN*l){if(!l)return 0;int m=0;for(NO*n=l->inicio;n;n=n->proximo)if(n->tipo==NO_GRUPO){int d=lista_gen_profundidade(n->sublista);if(d>m)m=d;}return 1+m;}

void lista_gen_percorrer(const LISTA_GEN*l,TAD_VISITANTE f,void*c){if(!l||!f)return;for(NO*n=l->inicio;n;n=n->proximo)if(n->tipo==NO_ATOMO)f(n->item,c);else lista_gen_percorrer(n->sublista,f,c);}
void lista_gen_imprimir(const LISTA_GEN*l){if(!l)return;printf("(");for(NO*n=l->inicio;n;n=n->proximo){if(n->tipo==NO_ATOMO){const char*s=item_original(n->item);printf("%s",s?s:"<item>");}else lista_gen_imprimir(n->sublista);if(n->proximo)printf(", ");}printf(")");}
