#include "semantico.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* ===== [P1] Analise semantica de expressoes e literais ===== */

struct ExpressaoRegistrada {
    NoAST *raiz;
    int imprimir;
    struct ExpressaoRegistrada *prox;
};

static struct ExpressaoRegistrada *expressoes;
static struct ExpressaoRegistrada **fim_expressoes = &expressoes;
static NoAST *raiz_ast;
static int erros;

/* [P1] Ponte de registro usada pelo parser nas regras de expressao. */
static struct ExpressaoRegistrada *buscar_registro(NoAST *raiz) {
    for (struct ExpressaoRegistrada *item = expressoes; item; item = item->prox)
        if (item->raiz == raiz) return item;
    return NULL;
}

void semantico_registrar_expressao(NoAST *raiz, int imprimir) {
    if (!raiz) return;

    struct ExpressaoRegistrada *item = buscar_registro(raiz);
    if (!item) {
        item = malloc(sizeof *item);
        if (!item) return;
        item->raiz = raiz;
        item->imprimir = 0;
        item->prox = NULL;
        *fim_expressoes = item;
        fim_expressoes = &item->prox;
    }

    if (imprimir && !item->imprimir) {
        if (!raiz_ast)
            raiz_ast = ast_criar(AST_PROGRAMA, NULL, NULL, NULL, raiz->linha);
        if (raiz_ast) {
            raiz_ast->a = ast_anexar(raiz_ast->a, raiz);
            item->imprimir = 1;
        }
    }
}

NoAST *semantico_ast(void) {
    return raiz_ast;
}

static int eh_operador_p1(TipoNo tipo) {
    return tipo == AST_SOMA || tipo == AST_SUBTRACAO ||
           tipo == AST_MULTIPLICACAO || tipo == AST_DIVISAO ||
           tipo == AST_DIVINT || tipo == AST_MODULO ||
           tipo == AST_POTENCIA || tipo == AST_NEGATIVO;
}

static int tipos_numericos(const char *tipo) {
    return tipo && tipo[0] == 'i' && tipo[1] == 'n' && tipo[2] == 't' && !tipo[3];
}

const char *semantico_tipo_expr(const NoAST *no) {
    if (!no) return "?";
    switch (no->tipo) {
        case AST_NUMERO: return "int";
        case AST_STRING: return "str";
        case AST_BOOL: return "bool";
        case AST_NONE: return "none";
        case AST_IDENTIFICADOR: return "?";
        case AST_NEGATIVO:
            return tipos_numericos(semantico_tipo_expr(no->a)) ? "int" : "?";
        case AST_SOMA: {
            const char *a = semantico_tipo_expr(no->a);
            const char *b = semantico_tipo_expr(no->b);
            if (tipos_numericos(a) && tipos_numericos(b)) return "int";
            if (a[0] == 's' && a[1] == 't' && a[2] == 'r' && !a[3] &&
                b[0] == 's' && b[1] == 't' && b[2] == 'r' && !b[3]) return "str";
            return "?";
        }
        case AST_SUBTRACAO:
        case AST_MULTIPLICACAO:
        case AST_DIVISAO:
        case AST_DIVINT:
        case AST_MODULO:
        case AST_POTENCIA:
            return tipos_numericos(semantico_tipo_expr(no->a)) &&
                   tipos_numericos(semantico_tipo_expr(no->b)) ? "int" : "?";
        default:
            return "?";
    }
}

static void erro_semantico(int linha, const char *mensagem) {
    fprintf(stderr, "[ERRO SEMANTICO] Linha %d: %s\n", linha, mensagem);
    erros++;
}

static int numero_literal(const NoAST *no, double *valor) {
    return ast_numero_valor(no, valor);
}

static int valor_numerico(const NoAST *no, double *valor) {
    double a, b;
    if (!no) return 0;
    if (no->tipo == AST_NUMERO) {
        if (!numero_literal(no, valor) || !isfinite(*valor)) return 0;
        return 1;
    }
    if (!eh_operador_p1(no->tipo) || !tipos_numericos(semantico_tipo_expr(no)))
        return 0;
    if (!valor_numerico(no->a, &a)) return 0;
    if (no->tipo == AST_NEGATIVO) {
        *valor = -a;
        return isfinite(*valor);
    }
    if (!valor_numerico(no->b, &b)) return 0;

    switch (no->tipo) {
        case AST_SOMA: *valor = a + b; break;
        case AST_SUBTRACAO: *valor = a - b; break;
        case AST_MULTIPLICACAO: *valor = a * b; break;
        case AST_DIVISAO:
            if (b == 0.0) return 0;
            *valor = a / b;
            break;
        case AST_DIVINT:
            if (b == 0.0) return 0;
            *valor = floor(a / b);
            break;
        case AST_MODULO:
            if (b == 0.0) return 0;
            *valor = fmod(a, b);
            if (*valor == 0.0) *valor = copysign(0.0, b);
            else if ((*valor < 0.0) != (b < 0.0)) *valor += b;
            break;
        case AST_POTENCIA:
            if (a == 0.0 && b < 0.0) return 0;
            *valor = pow(a, b);
            break;
        default: return 0;
    }
    return isfinite(*valor);
}

static int zero_literal(const NoAST *no) {
    double valor;
    return numero_literal(no, &valor) && valor == 0.0;
}

static int tipos_incompativeis(const NoAST *no) {
    const char *a, *b;
    if (no->tipo == AST_NEGATIVO)
        return semantico_tipo_expr(no->a)[0] != '?' &&
               !tipos_numericos(semantico_tipo_expr(no->a));
    a = semantico_tipo_expr(no->a);
    b = semantico_tipo_expr(no->b);
    if (a[0] == '?' || b[0] == '?') return 0;
    if (no->tipo == AST_SOMA && a[0] == 's' && b[0] == 's') return 0;
    return !tipos_numericos(a) || !tipos_numericos(b);
}

static const char *mensagem_overflow(TipoNo tipo) {
    switch (tipo) {
        case AST_SOMA: return "resultado de soma fora do intervalo suportado";
        case AST_SUBTRACAO: return "resultado de subtracao fora do intervalo suportado";
        case AST_MULTIPLICACAO: return "resultado de multiplicacao fora do intervalo suportado";
        case AST_DIVISAO: return "resultado de divisao fora do intervalo suportado";
        case AST_DIVINT: return "resultado de divisao inteira fora do intervalo suportado";
        case AST_MODULO: return "resultado de modulo fora do intervalo suportado";
        case AST_POTENCIA: return "resultado de potencia fora do intervalo suportado";
        default: return "resultado numerico fora do intervalo suportado";
    }
}

static int resultado_operacao(NoAST *no, double *resultado) {
    double a, b;
    if (!tipos_numericos(semantico_tipo_expr(no)) ||
        !valor_numerico(no->a, &a)) return 0;
    if (no->tipo == AST_NEGATIVO) {
        *resultado = -a;
        return 1;
    }
    if (!valor_numerico(no->b, &b)) return 0;
    switch (no->tipo) {
        case AST_SOMA: *resultado = a + b; return 1;
        case AST_SUBTRACAO: *resultado = a - b; return 1;
        case AST_MULTIPLICACAO: *resultado = a * b; return 1;
        case AST_DIVISAO:
            if (b == 0.0) return 0;
            *resultado = a / b;
            return 1;
        case AST_DIVINT:
            if (b == 0.0) return 0;
            *resultado = floor(a / b);
            return 1;
        case AST_MODULO:
            if (b == 0.0) return 0;
            *resultado = fmod(a, b);
            if (*resultado == 0.0) *resultado = copysign(0.0, b);
            else if ((*resultado < 0.0) != (b < 0.0)) *resultado += b;
            return 1;
        case AST_POTENCIA:
            *resultado = pow(a, b);
            return 1;
        default: return 0;
    }
}

static void analisar_no(NoAST *no) {
    if (!no) return;
    analisar_no(no->a);
    analisar_no(no->b);
    analisar_no(no->c);

    if (no->tipo == AST_NUMERO) {
        double valor;
        if (numero_literal(no, &valor) && !isfinite(valor))
            erro_semantico(no->linha, "literal numerico fora do intervalo suportado");
        return;
    }
    if (!eh_operador_p1(no->tipo)) return;

    int incompatibilidade = tipos_incompativeis(no);
    if (incompatibilidade) {
        char mensagem[160];
        if (no->tipo == AST_NEGATIVO) {
            snprintf(mensagem, sizeof mensagem,
                     "operacao entre tipos incompativeis ('%s')",
                     semantico_tipo_expr(no->a));
        } else {
            snprintf(mensagem, sizeof mensagem,
                     "operacao entre tipos incompativeis ('%s' e '%s')",
                     semantico_tipo_expr(no->a), semantico_tipo_expr(no->b));
        }
        erro_semantico(no->linha, mensagem);
    }

    double divisor;
    if ((no->tipo == AST_DIVISAO || no->tipo == AST_DIVINT ||
         no->tipo == AST_MODULO) && valor_numerico(no->b, &divisor) && divisor == 0.0) {
        erro_semantico(no->linha, "divisao por zero");
        return;
    }
    if (no->tipo == AST_POTENCIA && zero_literal(no->a)) {
        double expoente;
        if (valor_numerico(no->b, &expoente) && expoente < 0.0) {
            erro_semantico(no->linha,
                           "divisao por zero em potencia com expoente negativo");
            return;
        }
    }

    if (incompatibilidade) return;

    double resultado;
    if (!resultado_operacao(no, &resultado)) return;
    if (isnan(resultado) && no->tipo == AST_POTENCIA) {
        erro_semantico(no->linha, "potencia sem resultado real");
    } else if (!isfinite(resultado)) {
        erro_semantico(no->linha, mensagem_overflow(no->tipo));
    }
}

int semantico_analisar(NoAST *raiz) {
    erros = 0;
    for (struct ExpressaoRegistrada *item = expressoes; item; item = item->prox)
        analisar_no(item->raiz);
    if (!expressoes && raiz) analisar_no(raiz);
    return erros;
}

static int raiz_esta_impressa(NoAST *raiz_impressa, NoAST *raiz) {
    if (!raiz_impressa) return 0;
    for (NoAST *item = raiz_impressa->a; item; item = item->prox)
        if (item == raiz) return 1;
    return 0;
}

void semantico_liberar(NoAST *raiz_impressa) {
    struct ExpressaoRegistrada *item = expressoes;
    while (item) {
        struct ExpressaoRegistrada *proximo = item->prox;
        if (!raiz_esta_impressa(raiz_impressa, item->raiz))
            ast_liberar(item->raiz);
        free(item);
        item = proximo;
    }
    expressoes = NULL;
    fim_expressoes = &expressoes;
    raiz_ast = NULL;
    erros = 0;
}

/* ===== FIM [P1] Analise semantica de expressoes e literais ===== */
