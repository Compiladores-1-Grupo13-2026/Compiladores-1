#ifndef MINIPY_SEMANTICO_H
#define MINIPY_SEMANTICO_H

#include "Ast/ast.h"

/* ===== [P1] API de registro e analise de expressoes ===== */
/* O parser tambem encaminha expressoes de outras regras para esta analise. */
void semantico_registrar_expressao(NoAST *raiz, int imprimir);
NoAST *semantico_ast(void);
int semantico_analisar(NoAST *raiz);
const char *semantico_tipo_expr(const NoAST *no);
void semantico_liberar(NoAST *raiz_impressa);
/* ===== FIM [P1] API de registro e analise de expressoes ===== */

#endif
