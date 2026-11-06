#ifndef MINIPY_VALOR_H
#define MINIPY_VALOR_H

#include "Semantica/Ast/ast.h"

typedef enum {
    VAL_NONE, VAL_INT, VAL_REAL, VAL_BOOL, VAL_STR, VAL_LISTA, VAL_FUNCAO
} TipoValor;

typedef struct Lista Lista;

typedef struct Valor {
    TipoValor tipo;
    union {
        long long inteiro;   /* VAL_INT */
        double real;         /* VAL_REAL */
        int booleano;        /* VAL_BOOL: 0 ou 1 */
        char *texto;         /* VAL_STR: copia propria (malloc), nunca aponta para a AST */
        Lista *lista;        /* VAL_LISTA: compartilhada, com contador de referencias */
        NoAST *funcao;       /* VAL_FUNCAO: no AST_FUNCAO (pertence a AST, nunca liberar) */
    } u;
} Valor;

/* Lista do MiniPy: vetor dinamico. Como no Python, "b = a" faz as duas
 * variaveis verem a MESMA lista: por isso o contador de referencias. */
struct Lista {
    int tamanho;
    int capacidade;
    int referencias;
    Valor *itens;
};

/* ----- [P1] valor.c ----- */
Valor valor_none(void);
Valor valor_int(long long v);
Valor valor_real(double v);
Valor valor_bool(int v);
Valor valor_str(const char *texto);          /* copia o texto */
Valor valor_funcao(NoAST *def);
Valor valor_copiar(Valor v);                 /* str: duplica; lista: referencias++ */
void  valor_liberar(Valor v);                /* str: free; lista: referencias--, libera no 0 */
void  valor_imprimir(Valor v, int dentro_de_lista);  /* formato da secao 2.4, sem \n */
const char *valor_nome_tipo(Valor v);        /* "int", "float", "bool", "str", "list", "NoneType", "function" */

/* ----- [P4] lista.c ----- */
Lista *lista_criar(void);                    /* referencias = 1, vazia */
void   lista_adicionar(Lista *l, Valor v);   /* a lista passa a ser dona de v */
Valor  valor_lista(Lista *l);                /* embrulha a lista; assume a referencia que voce tinha */
void   lista_liberar(Lista *l);              /* chamado por valor_liberar quando referencias chega a 0 */

#endif
