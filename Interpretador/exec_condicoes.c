#include "Interpretador/interpretador.h"

#include <string.h>

int valor_verdadeiro(Valor v) {
    switch (v.tipo) {
        case VAL_NONE: return 0;
        case VAL_INT: return v.u.inteiro != 0;
        case VAL_REAL: return v.u.real != 0.0;
        case VAL_BOOL: return v.u.booleano != 0;
        case VAL_STR: return v.u.texto[0] != '\0';
        case VAL_LISTA: return v.u.lista->tamanho != 0;
        case VAL_FUNCAO: return 1;
    }
    return 0;
}

static int numerico(Valor v) {
    return v.tipo == VAL_INT || v.tipo == VAL_REAL || v.tipo == VAL_BOOL;
}

static long double numero(Valor v) {
    if (v.tipo == VAL_REAL) return v.u.real;
    if (v.tipo == VAL_BOOL) return v.u.booleano;
    return v.u.inteiro;
}

static int iguais(Valor a, Valor b, int linha) {
    if (numerico(a) && numerico(b)) return numero(a) == numero(b);
    if (a.tipo != b.tipo) return 0;
    switch (a.tipo) {
        case VAL_NONE: return 1;
        case VAL_STR: return strcmp(a.u.texto, b.u.texto) == 0;
        case VAL_FUNCAO: return a.u.funcao == b.u.funcao;
        case VAL_LISTA:
            if (a.u.lista == b.u.lista) return 1;
            if (a.u.lista->tamanho != b.u.lista->tamanho) return 0;
            for (int i = 0; i < a.u.lista->tamanho; i++)
                if (!iguais(a.u.lista->itens[i], b.u.lista->itens[i], linha)) return 0;
            return 1;
        default: return 0;
    }
}

Valor comparar_valores(TipoNo op, Valor a, Valor b, int linha) {
    if (op == AST_IGUAL || op == AST_DIFERENTE) {
        int resultado = iguais(a, b, linha);
        return valor_bool(op == AST_IGUAL ? resultado : !resultado);
    }
    if (numerico(a) && numerico(b)) {
        long double x = numero(a), y = numero(b);
        switch (op) {
            case AST_MENOR: return valor_bool(x < y);
            case AST_MAIOR: return valor_bool(x > y);
            case AST_MENOR_IGUAL: return valor_bool(x <= y);
            case AST_MAIOR_IGUAL: return valor_bool(x >= y);
            default: break;
        }
    } else if (a.tipo == VAL_STR && b.tipo == VAL_STR) {
        int ordem = strcmp(a.u.texto, b.u.texto);
        switch (op) {
            case AST_MENOR: return valor_bool(ordem < 0);
            case AST_MAIOR: return valor_bool(ordem > 0);
            case AST_MENOR_IGUAL: return valor_bool(ordem <= 0);
            case AST_MAIOR_IGUAL: return valor_bool(ordem >= 0);
            default: break;
        }
    }
    const char *simbolo = op == AST_MENOR ? "<" : op == AST_MAIOR ? ">" :
                          op == AST_MENOR_IGUAL ? "<=" : ">=";
    erro_execucao(linha, "comparacao '%s' entre tipos incompativeis ('%s' e '%s')",
                  simbolo, valor_nome_tipo(a), valor_nome_tipo(b));
    return valor_none();
}

Valor executar_condicao(NoAST *no) {
    Valor a = interpretar_no(no->a);
    if (no->tipo == AST_E || no->tipo == AST_OU) {
        int verdade = valor_verdadeiro(a);
        if ((no->tipo == AST_E && !verdade) || (no->tipo == AST_OU && verdade))
            return a;
        valor_liberar(a);
        return interpretar_no(no->b);
    }
    if (no->tipo == AST_NAO || no->tipo == AST_IF) {
        int verdade = valor_verdadeiro(a);
        valor_liberar(a);
        if (no->tipo == AST_NAO) return valor_bool(!verdade);
        valor_liberar(interpretar_no(verdade ? no->b : no->c));
        return valor_none();
    }
    Valor b = interpretar_no(no->b);
    Valor resultado = comparar_valores(no->tipo, a, b, no->linha);
    valor_liberar(a);
    valor_liberar(b);
    return resultado;
}
