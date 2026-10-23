#include "lista_cruz.h"

#include <stdint.h>
#include <stdlib.h>

typedef struct grupo GRUPO;
typedef struct celula CELULA;
typedef struct item_no ITEM_NO;
struct item_no { ITEM *item; ITEM_NO *proximo, *hash_proximo; };
struct celula { GRUPO *linha, *coluna; CELULA *prox_linha, *prox_coluna; ITEM_NO *itens; };
struct grupo { ITEM *representante; CELULA *celulas; GRUPO *proximo; size_t quantidade; };
struct lista_cruz_ { size_t campo_linha,campo_coluna,tamanho,cap_hash; const ESQUEMA *esquema; GRUPO *linhas,*colunas; ITEM_NO **hash; };

static size_t hash_item(const LISTA_CRUZ*l,const ITEM*i){return (((uintptr_t)i>>4)^((uintptr_t)i>>13))%l->cap_hash;}
static ITEM_NO *achar_hash(const LISTA_CRUZ*l,const ITEM*i){if(!l||!i)return NULL;for(ITEM_NO*n=l->hash[hash_item(l,i)];n;n=n->hash_proximo)if(n->item==i)return n;return NULL;}
static bool ampliar_hash(LISTA_CRUZ*l){if(l->tamanho+1<=l->cap_hash-l->cap_hash/4)return true;if(l->cap_hash>SIZE_MAX/2/sizeof(*l->hash))return false;size_t cap=l->cap_hash*2;ITEM_NO**h=calloc(cap,sizeof(*h));if(!h)return false;size_t anterior=l->cap_hash;l->cap_hash=cap;for(GRUPO*r=l->linhas;r;r=r->proximo)for(CELULA*c=r->celulas;c;c=c->prox_linha)for(ITEM_NO*n=c->itens;n;n=n->proximo){size_t k=hash_item(l,n->item);n->hash_proximo=h[k];h[k]=n;}free(l->hash);l->hash=h;(void)anterior;return true;}
static void remover_hash(LISTA_CRUZ*l,ITEM*i){size_t k=hash_item(l,i);ITEM_NO**p=&l->hash[k];while(*p&&(*p)->item!=i)p=&(*p)->hash_proximo;if(*p){ITEM_NO*n=*p;*p=n->hash_proximo;n->hash_proximo=NULL;}}

LISTA_CRUZ *lista_cruz_criar(size_t linha,size_t coluna) {
    if(linha==coluna)return NULL;
    LISTA_CRUZ*l=calloc(1,sizeof(*l));if(l){l->campo_linha=linha;l->campo_coluna=coluna;l->cap_hash=32;l->hash=calloc(l->cap_hash,sizeof(*l->hash));if(!l->hash){free(l);return NULL;}}return l;
}

static GRUPO *achar_grupo(GRUPO *lista,const ITEM *item,size_t campo) {
    for(GRUPO*g=lista;g;g=g->proximo)
        if(item_comparar_campo(g->representante,item,campo)==0)return g;
    return NULL;
}
static CELULA *achar_celula(GRUPO *linha,GRUPO *coluna) {
    for(CELULA*c=linha->celulas;c;c=c->prox_linha)if(c->coluna==coluna)return c;
    return NULL;
}

ITEM *lista_cruz_buscar(const LISTA_CRUZ*l,const ITEM*item) {
    ITEM_NO*n=achar_hash(l,item);return n?n->item:NULL;
}

bool lista_cruz_inserir(LISTA_CRUZ*l,ITEM*item) {
    if(!l||!item||item_campo(item,l->campo_linha)==NULL||
       item_campo(item,l->campo_coluna)==NULL||
       (l->esquema&&l->esquema!=item_esquema(item))||
       lista_cruz_buscar(l,item)||!ampliar_hash(l))return false;
    GRUPO*linha=achar_grupo(l->linhas,item,l->campo_linha);
    GRUPO*coluna=achar_grupo(l->colunas,item,l->campo_coluna);
    GRUPO*nlinha=linha?NULL:calloc(1,sizeof(*nlinha));
    GRUPO*ncoluna=coluna?NULL:calloc(1,sizeof(*ncoluna));
    if((!linha&&!nlinha)||(!coluna&&!ncoluna)){free(nlinha);free(ncoluna);return false;}
    if(!linha){linha=nlinha;linha->representante=item;}
    if(!coluna){coluna=ncoluna;coluna->representante=item;}
    CELULA*c=achar_celula(linha,coluna);
    CELULA*nc=c?NULL:calloc(1,sizeof(*nc));
    ITEM_NO*ni=malloc(sizeof(*ni));
    if(ni==NULL||(!c&&nc==NULL)){free(ni);free(nc);free(nlinha);free(ncoluna);return false;}
    if(nlinha){nlinha->proximo=l->linhas;l->linhas=nlinha;}
    if(ncoluna){ncoluna->proximo=l->colunas;l->colunas=ncoluna;}
    if(nc){nc->linha=linha;nc->coluna=coluna;nc->prox_linha=linha->celulas;linha->celulas=nc;
        nc->prox_coluna=coluna->celulas;coluna->celulas=nc;c=nc;}
    ni->item=item;ni->proximo=c->itens;c->itens=ni;size_t k=hash_item(l,item);ni->hash_proximo=l->hash[k];l->hash[k]=ni;++linha->quantidade;++coluna->quantidade;++l->tamanho;if(!l->esquema)l->esquema=item_esquema(item);
    return true;
}

static void remover_celula(CELULA **head,CELULA *c,bool linha) {
    CELULA **p=head;
    while(*p&&*p!=c)p=linha?&(*p)->prox_linha:&(*p)->prox_coluna;
    if(*p)*p=linha?c->prox_linha:c->prox_coluna;
}
static void apagar_grupo(GRUPO **lista,GRUPO*g){GRUPO**p=lista;while(*p&&*p!=g)p=&(*p)->proximo;if(*p)*p=g->proximo;free(g);}

ITEM *lista_cruz_remover(LISTA_CRUZ*l,ITEM*item) {
    if(!l||!item)return NULL;
    for(GRUPO*r=l->linhas;r;r=r->proximo){CELULA**pc=&r->celulas;while(*pc){CELULA*c=*pc;ITEM_NO**pn=&c->itens;while(*pn&&(*pn)->item!=item)pn=&(*pn)->proximo;if(*pn){
        GRUPO*col=c->coluna;ITEM_NO*rm=*pn;*pn=rm->proximo;remover_hash(l,item);free(rm);--r->quantidade;--col->quantidade;--l->tamanho;
        if(c->itens==NULL){remover_celula(&r->celulas,c,true);remover_celula(&col->celulas,c,false);free(c);}
        if(r->quantidade==0)apagar_grupo(&l->linhas,r);
        else if(r->representante==item)r->representante=r->celulas->itens->item;
        if(col->quantidade==0)apagar_grupo(&l->colunas,col);
        else if(col->representante==item)col->representante=col->celulas->itens->item;
        return item;}pc=&c->prox_linha;}}
    return NULL;
}

void lista_cruz_apagar(LISTA_CRUZ**lista){if(!lista||!*lista)return;LISTA_CRUZ*l=*lista;GRUPO*r=l->linhas;while(r){GRUPO*rn=r->proximo;CELULA*c=r->celulas;while(c){CELULA*cn=c->prox_linha;ITEM_NO*n=c->itens;while(n){ITEM_NO*nn=n->proximo;free(n);n=nn;}free(c);c=cn;}free(r);r=rn;}GRUPO*g=l->colunas;while(g){GRUPO*n=g->proximo;free(g);g=n;}free(l->hash);free(l);*lista=NULL;}

void lista_cruz_percorrer(const LISTA_CRUZ*l,LISTA_CRUZ_EIXO eixo,TAD_VISITANTE f,void*ctx){if(!l||!f)return;GRUPO*g=eixo==CRUZ_POR_LINHA?l->linhas:l->colunas;for(;g;g=g->proximo)for(CELULA*c=g->celulas;c;c=eixo==CRUZ_POR_LINHA?c->prox_linha:c->prox_coluna)for(ITEM_NO*n=c->itens;n;n=n->proximo)f(n->item,ctx);}
size_t lista_cruz_grupos(const LISTA_CRUZ*l,LISTA_CRUZ_EIXO eixo){if(!l)return 0;size_t n=0;for(GRUPO*g=eixo==CRUZ_POR_LINHA?l->linhas:l->colunas;g;g=g->proximo)++n;return n;}
