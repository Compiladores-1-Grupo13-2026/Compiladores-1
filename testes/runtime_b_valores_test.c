/* Limites numericos e posse de valores usando os modulos reais da dupla. */
#include "Interpretador/interpretador.h"

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void esperar_int(TipoNo op, long long a, long long b, long long esperado) {
    Valor resultado = operar_aritmetica(op, valor_int(a), valor_int(b), 1);
    assert(resultado.tipo == VAL_INT && resultado.u.inteiro == esperado);
    valor_liberar(resultado);
}

static void testar_limites(void) {
    esperar_int(AST_SOMA, 9007199254740993LL, 2, 9007199254740995LL);
    esperar_int(AST_SUBTRACAO, LLONG_MIN + 1, 1, LLONG_MIN);
    esperar_int(AST_MULTIPLICACAO, LLONG_MIN / 2, 2, LLONG_MIN);
    esperar_int(AST_POTENCIA, -2, 63, LLONG_MIN);
    esperar_int(AST_POTENCIA, -1, LLONG_MAX, -1);
    esperar_int(AST_DIVINT, LLONG_MIN, 3, -3074457345618258603LL);
    esperar_int(AST_DIVINT, LLONG_MAX, 3, 3074457345618258602LL);
    esperar_int(AST_MODULO, LLONG_MIN, 3, 1);
    esperar_int(AST_MODULO, LLONG_MIN, -1, 0);
    esperar_int(AST_MODULO, 7, -3, -2);
    Valor resultado = operar_aritmetica(AST_DIVISAO, valor_int(4), valor_int(2), 1);
    assert(resultado.tipo == VAL_REAL && resultado.u.real == 2.0);
    resultado = operar_aritmetica(AST_DIVINT, valor_real(-7.5), valor_int(2), 1);
    assert(resultado.tipo == VAL_REAL && resultado.u.real == -4.0);
    resultado = operar_aritmetica(AST_POTENCIA, valor_int(2), valor_int(-3), 1);
    assert(resultado.tipo == VAL_REAL && resultado.u.real == 0.125);
    resultado = operar_aritmetica(AST_MODULO, valor_real(4), valor_real(-2), 1);
    assert(resultado.tipo == VAL_REAL && resultado.u.real == 0 && signbit(resultado.u.real));

    /* O limite superior de long long nao pode ser convertido de 2^63/double. */
    NoAST *numero = ast_criar_numero_real(-(double)LLONG_MIN, 1);
    resultado = executar_expressao(numero);
    assert(resultado.tipo == VAL_REAL);
    ast_liberar(numero);
    numero = ast_criar_numero_real((double)LLONG_MIN, 1);
    resultado = executar_expressao(numero);
    assert(resultado.tipo == VAL_INT && resultado.u.inteiro == LLONG_MIN);
    ast_liberar(numero);
}

static void testar_copias(void) {
    char original[] = "abc";
    Valor a = valor_str(original), b = valor_copiar(a);
    original[0] = 'X';
    a.u.texto[1] = 'Y';
    assert(a.u.texto != b.u.texto && strcmp(b.u.texto, "abc") == 0);
    valor_liberar(a);
    assert(strcmp(b.u.texto, "abc") == 0);
    valor_liberar(b);

    Lista *lista = lista_criar();
    lista_adicionar(lista, valor_str("compartilhado"));
    a = valor_lista(lista);
    b = valor_copiar(a);
    assert(a.u.lista == b.u.lista && lista->referencias == 2);
    valor_liberar(a);
    assert(lista->referencias == 1);
    assert(strcmp(b.u.lista->itens[0].u.texto, "compartilhado") == 0);
    valor_liberar(b);

    NoAST *def = ast_criar_texto(AST_FUNCAO, "f", 1);
    a = valor_funcao(def);
    b = valor_copiar(a);
    valor_liberar(a);
    valor_liberar(b);
    assert(strcmp(def->texto, "f") == 0);
    ast_liberar(def);
}

static NoAST *potencia(int base, int expoente) {
    return ast_criar(AST_POTENCIA, ast_criar_numero(base, 7),
                     ast_criar_numero(expoente, 7), NULL, 7);
}

static void rejeitar_overflow(NoAST *expressao) {
    NoAST *raiz = ast_criar(AST_PROGRAMA, expressao, NULL, NULL, 1);
    assert(interpretar_programa(raiz) == 1);
    ast_liberar(raiz);
}

int main(void) {
    testar_limites();
    testar_copias();
    rejeitar_overflow(ast_criar(AST_MULTIPLICACAO, potencia(2, 62),
                               ast_criar_numero(2, 7), NULL, 7));
    rejeitar_overflow(potencia(2, 63));
    rejeitar_overflow(ast_criar(AST_NEGATIVO, potencia(-2, 63), NULL, NULL, 7));
    puts("Pessoa B/P1: precisao inteira, limites e posse de valores passaram.");
    return 0;
}
