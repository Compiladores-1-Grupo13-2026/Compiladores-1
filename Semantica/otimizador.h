/*
 * FGA0003 - Compiladores 1
 * Curso de Engenharia de Software
 * Universidade de Brasília (UnB)
 * Equipe 13 - MiniPy
 *
 * Arquivo: Semantica/otimizador.h
 * Descrição: interface do dobramento de constantes, que recebe
 *            a raiz da AST e devolve a raiz resultante.
 */

#ifndef MINIPY_OTIMIZADOR_H
#define MINIPY_OTIMIZADOR_H

#include "Ast/ast.h"

/* Recebe a raiz e devolve a raiz otimizada, que pode ser um novo no. */
NoAST *otimizar_ast(NoAST *raiz);

#endif
