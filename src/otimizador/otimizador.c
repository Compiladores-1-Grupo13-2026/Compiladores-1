/*
 * FGA0003 - Compiladores 1
 * Curso de Engenharia de Software
 * Universidade de Brasília (UnB)
 * Equipe 13 - MiniPy
 *
 * Arquivo: src/otimizador/otimizador.c
 * Descrição: percurso da AST para dobrar somas, subtrações e
 *            multiplicações de literais inteiros cujo resultado cabe em int.
 */

#include "otimizador.h"

#include <limits.h>
#include <stddef.h>

static int dobrar_operacao(TipoNo tipo, int a, int b, int *resultado) {
    switch (tipo) {
        case AST_SOMA:
            if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
                return 0;
            *resultado = a + b;
            return 1;
        case AST_SUBTRACAO:
            if ((b > 0 && a < INT_MIN + b) || (b < 0 && a > INT_MAX + b))
                return 0;
            *resultado = a - b;
            return 1;
        case AST_MULTIPLICACAO:
            if (a > 0 && (b > INT_MAX / a || b < INT_MIN / a))
                return 0;
            if (a < 0 && ((b > 0 && a < INT_MIN / b) ||
                          (b < 0 && a < INT_MAX / b)))
                return 0;
            *resultado = a * b;
            return 1;
        default:
            return 0;
    }
}

NoAST *otimizar_ast(NoAST *raiz) {
    for (NoAST **elo = &raiz; *elo; elo = &(*elo)->prox) {
        NoAST *no = *elo;
        no->a = otimizar_ast(no->a);
        no->b = otimizar_ast(no->b);
        no->c = otimizar_ast(no->c);

        int a, b, resultado;
        if (!ast_numero_inteiro(no->a, &a) || !ast_numero_inteiro(no->b, &b) ||
            !dobrar_operacao(no->tipo, a, b, &resultado))
            continue;

        NoAST *dobrado = ast_criar_numero(resultado, no->linha);
        if (!dobrado) continue;
        dobrado->prox = no->prox;
        no->prox = NULL;
        *elo = dobrado;
        ast_liberar(no);
    }
    return raiz;
}
