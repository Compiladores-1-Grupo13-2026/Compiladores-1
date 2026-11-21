#ifndef MINIPY_INTERPRETADOR_H
#define MINIPY_INTERPRETADOR_H

#include "Semantica/Ast/ast.h"
#include "Interpretador/valor.h"

/* break, continue e return interrompem a execucao no meio de um bloco. */
typedef enum { SINAL_NORMAL, SINAL_BREAK, SINAL_CONTINUE, SINAL_RETURN } Sinal;
extern Sinal sinal_atual;
extern Valor valor_retorno;          /* preenchido por RETURN (P5) */

/* ----- base: interpretador.c ----- */
int   interpretar_programa(NoAST *raiz);   /* 0 = ok, 1 = erro de execucao */
Valor interpretar_no(NoAST *no);           /* o chamador fica dono do Valor devolvido */
void  executar_bloco(NoAST *primeiro);     /* executa a lista prox ate o fim ou sinal != NORMAL */
void  erro_execucao(int linha, const char *formato, ...);  /* imprime e NAO retorna (longjmp) */

/* ----- uma funcao de entrada por fatia (o switch da base chama) ----- */
Valor executar_expressao(NoAST *no);   /* [P1] NUMERO STRING BOOL NONE, aritmetica, NEGATIVO */
Valor executar_condicao(NoAST *no);    /* [P2] comparacoes, E OU NAO, IF */
Valor executar_laco(NoAST *no);        /* [P3] WHILE FOR BREAK CONTINUE */
Valor executar_atribuicao(NoAST *no);  /* [P4] IDENTIFICADOR, ATRIB*, LISTA, INDEXACAO */
Valor executar_funcao(NoAST *no);      /* [P5] FUNCAO PARAMETRO RETURN CHAMADA */

/* ----- ajudantes compartilhados ----- */
Valor operar_aritmetica(TipoNo op, Valor a, Valor b, int linha);  /* [P1] usado pela P4 em += -= *= /= */
Valor comparar_valores(TipoNo op, Valor a, Valor b, int linha);   /* [P2] */
int   valor_verdadeiro(Valor v);       /* [P2] 0, 0.0, "", [], None e False sao falsos */

#endif
