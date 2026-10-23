#include "estrutura.h"

#include "fila.h"
#include "lista_cabeca.h"
#include "lista_cruz.h"
#include "lista_enc.h"
#include "lista_gen.h"
#include "lista_ord.h"
#include "lista_seq.h"
#include "pilha.h"

#include <stdlib.h>
#include <string.h>

typedef enum { T_PILHA, T_FILA, T_SEQ, T_ENC, T_CABECA, T_ORD, T_GEN, T_CRUZ } TIPO_TAD;
struct estrutura {
    TIPO_TAD tipo;
    ESTRUTURA_PAPEL papel;
    const ESQUEMA *esquema;
    const ESQUEMA *esquema_query; /* Emprestado do contexto/dataset. */
    union {
        PILHA *pilha; FILA *fila; LISTA_SEQ *seq; LISTA_ENC *enc;
        LISTA_CABECA *cabeca; LISTA_ORD *ord; LISTA_GEN *gen; LISTA_CRUZ *cruz;
    } dado;
};

static const INSTRUCAO *instrucoes_tipo(const QUERY *q,TIPO_INSTRUCAO tipo) {
    size_t n=0;const INSTRUCAO *v=query_instrucoes(q,&n);
    for(size_t i=0;i<n;i++)if(v[i].tipo==tipo)return &v[i];
    return NULL;
}

ESTRUTURA *estrutura_criar(const char *nome,ESTRUTURA_PAPEL papel,const QUERY *q) {
    if(nome==NULL||(papel!=ESTRUTURA_BASE&&papel!=ESTRUTURA_RESULTADOS))return NULL;
    ESTRUTURA *e=calloc(1,sizeof(*e));if(e==NULL)return NULL;e->papel=papel;e->esquema_query=query_esquema(q);
    if(strcmp(nome,"pilha")==0){e->tipo=T_PILHA;e->dado.pilha=pilha_criar();}
    else if(strcmp(nome,"fila")==0){e->tipo=T_FILA;e->dado.fila=fila_criar();}
    else if(strcmp(nome,"lista_seq")==0){e->tipo=T_SEQ;e->dado.seq=lista_seq_criar();}
    else if(strcmp(nome,"lista_enc")==0){e->tipo=T_ENC;e->dado.enc=lista_enc_criar(false);}
    else if(strcmp(nome,"lista_cabeca")==0){e->tipo=T_CABECA;e->dado.cabeca=lista_cabeca_criar();}
    else if(strcmp(nome,"lista_ord")==0){
        e->tipo=T_ORD;e->dado.ord=lista_ord_criar();
        if(e->dado.ord!=NULL&&instrucoes_tipo(q,ORDER)!=NULL)
            if(!lista_ord_ordenar_comparador(e->dado.ord,query_comparador_ordenacao,(void*)q))
                lista_ord_apagar(&e->dado.ord);
    } else if(strcmp(nome,"lista_gen")==0){
        e->tipo=T_GEN;e->dado.gen=lista_gen_criar();
        const INSTRUCAO *i=instrucoes_tipo(q,GROUP);
        const size_t padrao[]={0,1};
        const size_t *campos=i!=NULL?i->indices_campos:padrao;
        size_t n_campos=i!=NULL?i->quantidade_campos:2;
        if(e->dado.gen!=NULL&&
           !lista_gen_configurar_grupos(e->dado.gen,campos,n_campos))
            lista_gen_apagar(&e->dado.gen);
        if(papel==ESTRUTURA_RESULTADOS&&i==NULL)lista_gen_apagar(&e->dado.gen);
    } else if(strcmp(nome,"lista_cruz")==0){
        e->tipo=T_CRUZ;const INSTRUCAO *i=instrucoes_tipo(q,CROSS);
        if(i!=NULL&&i->quantidade_campos==2)
            e->dado.cruz=lista_cruz_criar(i->indices_campos[0],i->indices_campos[1]);
        else if(papel==ESTRUTURA_BASE)e->dado.cruz=lista_cruz_criar(0,1);
    } else { free(e);return NULL; }
    bool criada=false;
    switch(e->tipo){
    case T_PILHA:criada=e->dado.pilha!=NULL;break;case T_FILA:criada=e->dado.fila!=NULL;break;
    case T_SEQ:criada=e->dado.seq!=NULL;break;case T_ENC:criada=e->dado.enc!=NULL;break;
    case T_CABECA:criada=e->dado.cabeca!=NULL;break;case T_ORD:criada=e->dado.ord!=NULL;break;
    case T_GEN:criada=e->dado.gen!=NULL;break;case T_CRUZ:criada=e->dado.cruz!=NULL;break;
    }
    if(!criada){free(e);return NULL;}return e;
}

bool estrutura_inserir(ESTRUTURA*e,ITEM*i){
    if(!e||!i)return false;
    const ESQUEMA *esquema=item_esquema(i);
    if((e->esquema&&e->esquema!=esquema)||
       (e->esquema_query&&e->esquema_query!=esquema))return false;
    /* Os índices padrão de GROUP/CROSS também precisam existir quando não
       há instrução explícita na consulta. */
    if((e->tipo==T_GEN||e->tipo==T_CRUZ)&&esquema_quantidade(esquema)<2)
        return false;
    if(e->papel==ESTRUTURA_BASE&&!item_reservar_base(i,e))return false;
    bool ok=false;switch(e->tipo){
case T_PILHA:ok=pilha_empilhar(e->dado.pilha,i);break;case T_FILA:ok=fila_inserir(e->dado.fila,i);break;
case T_SEQ:ok=lista_seq_inserir(e->dado.seq,i);break;case T_ENC:ok=lista_enc_inserir(e->dado.enc,i);break;
case T_CABECA:ok=lista_cabeca_inserir(e->dado.cabeca,i);break;case T_ORD:ok=lista_ord_inserir(e->dado.ord,i);break;
case T_GEN:ok=lista_gen_inserir_atomo(e->dado.gen,i);break;case T_CRUZ:ok=lista_cruz_inserir(e->dado.cruz,i);break;}
    if(e->papel==ESTRUTURA_BASE&&!ok)item_liberar_base(i,e);
    if (ok && e->esquema == NULL)
        e->esquema = item_esquema(i);
    return ok;
}

void estrutura_percorrer(const ESTRUTURA*e,TAD_VISITANTE f,void*c){if(!e||!f)return;switch(e->tipo){
case T_PILHA:pilha_percorrer(e->dado.pilha,f,c);break;case T_FILA:fila_percorrer(e->dado.fila,f,c);break;
case T_SEQ:lista_seq_percorrer(e->dado.seq,f,c);break;case T_ENC:lista_enc_percorrer(e->dado.enc,f,c);break;
case T_CABECA:lista_cabeca_percorrer(e->dado.cabeca,f,c);break;case T_ORD:lista_ord_percorrer(e->dado.ord,f,c);break;
case T_GEN:lista_gen_percorrer(e->dado.gen,f,c);break;case T_CRUZ:lista_cruz_percorrer(e->dado.cruz,CRUZ_POR_LINHA,f,c);break;}}

void estrutura_percorrer_eixo(const ESTRUTURA*e,LISTA_CRUZ_EIXO eixo,TAD_VISITANTE f,void*c){if(!e||!f)return;if(e->tipo==T_CRUZ)lista_cruz_percorrer(e->dado.cruz,eixo,f,c);else estrutura_percorrer(e,f,c);}

ITEM *estrutura_remover(ESTRUTURA*e,ITEM*i){if(!e||!i)return NULL;ITEM *retorno=NULL;switch(e->tipo){
case T_PILHA:retorno=pilha_remover_item(e->dado.pilha,i);break;case T_FILA:retorno=fila_remover_item(e->dado.fila,i);break;
case T_SEQ:retorno=lista_seq_remover_item(e->dado.seq,i);break;case T_ENC:retorno=lista_enc_remover(e->dado.enc,i);break;
case T_CABECA:retorno=lista_cabeca_remover(e->dado.cabeca,i);break;case T_ORD:retorno=lista_ord_remover_item(e->dado.ord,i);break;
case T_GEN:retorno=lista_gen_remover_item(e->dado.gen,i);break;case T_CRUZ:retorno=lista_cruz_remover(e->dado.cruz,i);break;}
    if(retorno&&e->papel==ESTRUTURA_BASE)item_liberar_base(retorno,e);
    return retorno;}

static void contar(ITEM*i,void*c){(void)i;++*(size_t*)c;}
size_t estrutura_tamanho(const ESTRUTURA*e){size_t n=0;estrutura_percorrer(e,contar,&n);return n;}
static void liberar_item(ITEM*i,void*c){(void)c;item_apagar(&i);}

void estrutura_apagar(ESTRUTURA**estrutura){if(!estrutura||!*estrutura)return;ESTRUTURA*e=*estrutura;
    if(e->papel==ESTRUTURA_BASE)estrutura_percorrer(e,liberar_item,NULL);
    switch(e->tipo){case T_PILHA:pilha_apagar(&e->dado.pilha);break;case T_FILA:fila_apagar(&e->dado.fila);break;
    case T_SEQ:lista_seq_apagar(&e->dado.seq);break;case T_ENC:lista_enc_apagar(&e->dado.enc);break;
    case T_CABECA:lista_cabeca_apagar(&e->dado.cabeca);break;case T_ORD:lista_ord_apagar(&e->dado.ord);break;
    case T_GEN:lista_gen_apagar(&e->dado.gen);break;case T_CRUZ:lista_cruz_apagar(&e->dado.cruz);break;}
    free(e);*estrutura=NULL;}
