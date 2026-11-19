#ifndef MINIPY_OTIMIZADOR_H
#define MINIPY_OTIMIZADOR_H

#include "Ast/ast.h"

/* Recebe a raiz e devolve a raiz otimizada, que pode ser um novo no. */
NoAST *otimizar_ast(NoAST *raiz);

#endif
