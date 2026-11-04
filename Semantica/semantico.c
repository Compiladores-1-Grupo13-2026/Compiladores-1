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

/* Verifica se o no representa um operador aritmetico da P1. */
static int eh_operador_p1(TipoNo tipo) {
    return tipo == AST_SOMA || tipo == AST_SUBTRACAO ||
           tipo == AST_MULTIPLICACAO || tipo == AST_DIVISAO ||
           tipo == AST_DIVINT || tipo == AST_MODULO ||
           tipo == AST_POTENCIA || tipo == AST_NEGATIVO;
}

/* Verifica se o tipo informado e int. */
static int tipos_numericos(const char *tipo) {
    return tipo && tipo[0] == 'i' && tipo[1] == 'n' &&
           tipo[2] == 't' && !tipo[3];
}

/* Verifica se dois tipos sao iguais. */
static int tipos_iguais(const char *a, const char *b) {
    if (!a || !b) return 0;

    if (a[0] != b[0]) return 0;

    if (a[0] == 'i')
        return tipos_numericos(a) && tipos_numericos(b);

    if (a[0] == 's')
        return a[1] == 't' && a[2] == 'r' && !a[3] &&
               b[1] == 't' && b[2] == 'r' && !b[3];

    if (a[0] == 'b')
        return a[1] == 'o' && a[2] == 'o' && a[3] == 'l' && !a[4] &&
               b[1] == 'o' && b[2] == 'o' && b[3] == 'l' && !b[4];

    if (a[0] == 'n')
        return a[1] == 'o' && a[2] == 'n' && a[3] == 'e' && !a[4] &&
               b[1] == 'o' && b[2] == 'n' && b[3] == 'e' && !b[4];

    return 0;
}

/* ===== [P2] Identificacao de operadores de comparacao e logica ===== */

static int eh_operador_p2(TipoNo tipo) {
    return tipo == AST_IGUAL || tipo == AST_DIFERENTE ||
           tipo == AST_MENOR || tipo == AST_MAIOR ||
           tipo == AST_MENOR_IGUAL || tipo == AST_MAIOR_IGUAL ||
           tipo == AST_E || tipo == AST_OU || tipo == AST_NAO;
}

static int eh_comparacao_igualdade(TipoNo tipo) {
    return tipo == AST_IGUAL || tipo == AST_DIFERENTE;
}

static int eh_comparacao_ordem(TipoNo tipo) {
    return tipo == AST_MENOR || tipo == AST_MAIOR ||
           tipo == AST_MENOR_IGUAL || tipo == AST_MAIOR_IGUAL;
}

/* ===== FIM [P2] Identificacao de operadores de comparacao e logica ===== */

/*
 * Retorna o tipo resultante de uma expressao.
 *
 * P1:
 *   Operacoes aritmeticas retornam int quando os operandos sao numericos.
 *
 * P2:
 *   Comparacoes e operadores logicos retornam bool.
 */
const char *semantico_tipo_expr(const NoAST *no) {
    if (!no) return "?";

    switch (no->tipo) {
        case AST_NUMERO:
            return "int";

        case AST_STRING:
            return "str";

        case AST_BOOL:
            return "bool";

        case AST_NONE:
            return "none";

        case AST_IDENTIFICADOR:
            return "?";

        case AST_NEGATIVO:
            return tipos_numericos(semantico_tipo_expr(no->a)) ? "int" : "?";

        case AST_SOMA: {
            const char *a = semantico_tipo_expr(no->a);
            const char *b = semantico_tipo_expr(no->b);

            if (tipos_numericos(a) && tipos_numericos(b))
                return "int";

            if (tipos_iguais(a, "str") && tipos_iguais(b, "str"))
                return "str";

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

        /* ===== [P2] Tipo resultante de comparacoes e logica ===== */

        case AST_IGUAL:
        case AST_DIFERENTE:
        case AST_MENOR:
        case AST_MAIOR:
        case AST_MENOR_IGUAL:
        case AST_MAIOR_IGUAL:
        case AST_E:
        case AST_OU:
        case AST_NAO:
            return "bool";

        /* ===== FIM [P2] Tipo resultante de comparacoes e logica ===== */

        default:
            return "?";
    }
}

static void erro_semantico(int linha, const char *mensagem) {
    fprintf(stderr, "[ERRO SEMANTICO] Linha %d: %s\n", linha, mensagem);
    erros++;
}

/* Recupera o valor numerico armazenado em um literal. */
static int numero_literal(const NoAST *no, double *valor) {
    return ast_numero_valor(no, valor);
}

/* Calcula o valor de uma expressao numerica da P1. */
static int valor_numerico(const NoAST *no, double *valor) {
    double a, b;

    if (!no) return 0;

    if (no->tipo == AST_NUMERO) {
        if (!numero_literal(no, valor) || !isfinite(*valor))
            return 0;

        return 1;
    }

    if (!eh_operador_p1(no->tipo) ||
        !tipos_numericos(semantico_tipo_expr(no)))
        return 0;

    if (!valor_numerico(no->a, &a))
        return 0;

    if (no->tipo == AST_NEGATIVO) {
        *valor = -a;
        return isfinite(*valor);
    }

    if (!valor_numerico(no->b, &b))
        return 0;

    switch (no->tipo) {
        case AST_SOMA:
            *valor = a + b;
            break;

        case AST_SUBTRACAO:
            *valor = a - b;
            break;

        case AST_MULTIPLICACAO:
            *valor = a * b;
            break;

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

            if (*valor == 0.0)
                *valor = copysign(0.0, b);
            else if ((*valor < 0.0) != (b < 0.0))
                *valor += b;

            break;

        case AST_POTENCIA:
            if (a == 0.0 && b < 0.0) return 0;
            *valor = pow(a, b);
            break;

        default:
            return 0;
    }

    return isfinite(*valor);
}

/* Verifica se um no representa o literal numerico zero. */
static int zero_literal(const NoAST *no) {
    double valor;

    return numero_literal(no, &valor) && valor == 0.0;
}

/* Verifica incompatibilidades entre operandos da P1. */
static int tipos_incompativeis(const NoAST *no) {
    const char *a, *b;

    if (no->tipo == AST_NEGATIVO) {
        const char *tipo = semantico_tipo_expr(no->a);

        return tipo[0] != '?' && !tipos_numericos(tipo);
    }

    a = semantico_tipo_expr(no->a);
    b = semantico_tipo_expr(no->b);

    /* Identificadores ainda nao possuem verificacao de tipo. */
    if (a[0] == '?' || b[0] == '?')
        return 0;

    /* Soma de duas strings e permitida. */
    if (no->tipo == AST_SOMA &&
        tipos_iguais(a, "str") && tipos_iguais(b, "str"))
        return 0;

    return !tipos_numericos(a) || !tipos_numericos(b);
}

/* ===== [P2] Verificacao de compatibilidade de tipos ===== */

/*
 * Verifica se os operandos de uma comparacao possuem tipos compativeis.
 *
 * Igualdade e diferenca:
 *   - permitem comparar valores do mesmo tipo;
 *   - permitem comparar valores numericos;
 *   - nao verificam operandos cujo tipo ainda e desconhecido.
 *
 * Comparacoes de ordem:
 *   - neste momento, aceitam apenas operandos numericos;
 *   - ignoram identificadores cujo tipo ainda e desconhecido.
 *
 * Operadores logicos:
 *   - aceitam operandos de qualquer tipo, seguindo a ideia de
 *     avaliacao por verdadeiro/falso.
 */
static int tipos_incompativeis_p2(const NoAST *no) {
    const char *a;
    const char *b;

    if (no->tipo == AST_NAO) {
        a = semantico_tipo_expr(no->a);

        /* Tipo desconhecido: nao e possivel validar ainda. */
        return a[0] != '?' && a[0] == '\0';
    }

    if (no->tipo == AST_E || no->tipo == AST_OU)
        return 0;

    a = semantico_tipo_expr(no->a);
    b = semantico_tipo_expr(no->b);

    /* Nao verifica expressoes que contenham identificadores desconhecidos. */
    if (a[0] == '?' || b[0] == '?')
        return 0;

    if (eh_comparacao_igualdade(no->tipo)) {
        if (tipos_numericos(a) && tipos_numericos(b))
            return 0;

        return !tipos_iguais(a, b);
    }

    if (eh_comparacao_ordem(no->tipo))
        return !tipos_numericos(a) || !tipos_numericos(b);

    return 0;
}

/* ===== FIM [P2] Verificacao de compatibilidade de tipos ===== */

/* Mensagens especificas para erros de operacoes aritmeticas. */
static const char *mensagem_overflow(TipoNo tipo) {
    switch (tipo) {
        case AST_SOMA:
            return "resultado de soma fora do intervalo suportado";

        case AST_SUBTRACAO:
            return "resultado de subtracao fora do intervalo suportado";

        case AST_MULTIPLICACAO:
            return "resultado de multiplicacao fora do intervalo suportado";

        case AST_DIVISAO:
            return "resultado de divisao fora do intervalo suportado";

        case AST_DIVINT:
            return "resultado de divisao inteira fora do intervalo suportado";

        case AST_MODULO:
            return "resultado de modulo fora do intervalo suportado";

        case AST_POTENCIA:
            return "resultado de potencia fora do intervalo suportado";

        default:
            return "resultado numerico fora do intervalo suportado";
    }
}

/* Calcula o resultado de uma operacao aritmetica da P1. */
static int resultado_operacao(NoAST *no, double *resultado) {
    double a, b;

    if (!tipos_numericos(semantico_tipo_expr(no)) ||
        !valor_numerico(no->a, &a))
        return 0;

    if (no->tipo == AST_NEGATIVO) {
        *resultado = -a;
        return 1;
    }

    if (!valor_numerico(no->b, &b))
        return 0;

    switch (no->tipo) {
        case AST_SOMA:
            *resultado = a + b;
            return 1;

        case AST_SUBTRACAO:
            *resultado = a - b;
            return 1;

        case AST_MULTIPLICACAO:
            *resultado = a * b;
            return 1;

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

            if (*resultado == 0.0)
                *resultado = copysign(0.0, b);
            else if ((*resultado < 0.0) != (b < 0.0))
                *resultado += b;

            return 1;

        case AST_POTENCIA:
            *resultado = pow(a, b);
            return 1;

        default:
            return 0;
    }
}

/* ===== [P2] Analise semantica de comparacoes e logica ===== */

static void analisar_operador_p2(NoAST *no) {
    int incompatibilidade = tipos_incompativeis_p2(no);

    if (!incompatibilidade)
        return;

    char mensagem[160];

    if (no->tipo == AST_NAO) {
        snprintf(mensagem, sizeof mensagem,
                 "operacao logica com tipo incompativel ('%s')",
                 semantico_tipo_expr(no->a));
    } else {
        snprintf(mensagem, sizeof mensagem,
                 "comparacao entre tipos incompativeis ('%s' e '%s')",
                 semantico_tipo_expr(no->a),
                 semantico_tipo_expr(no->b));
    }

    erro_semantico(no->linha, mensagem);
}

/* ===== FIM [P2] Analise semantica de comparacoes e logica ===== */

/*
 * Percorre a AST e realiza as verificacoes semanticas.
 * A analise dos filhos ocorre antes da analise do proprio no.
 */
static void analisar_no(NoAST *no) {
    if (!no) return;

    analisar_no(no->a);
    analisar_no(no->b);
    analisar_no(no->c);

    /* Verifica literais numericos fora do intervalo suportado. */
    if (no->tipo == AST_NUMERO) {
        double valor;

        if (numero_literal(no, &valor) && !isfinite(valor))
            erro_semantico(no->linha,
                           "literal numerico fora do intervalo suportado");

        return;
    }

    /* ===== [P2] Verificacao dos operadores logicos e comparativos ===== */

    if (eh_operador_p2(no->tipo)) {
        analisar_operador_p2(no);
        return;
    }

    /* ===== FIM [P2] Verificacao dos operadores logicos e comparativos ===== */

    /* Os demais nos que nao sao operadores aritmeticos nao precisam
       das verificacoes numericas da P1. */
    if (!eh_operador_p1(no->tipo))
        return;

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
                     semantico_tipo_expr(no->a),
                     semantico_tipo_expr(no->b));
        }

        erro_semantico(no->linha, mensagem);
    }

    /* Verifica divisao por zero. */
    double divisor;

    if ((no->tipo == AST_DIVISAO ||
         no->tipo == AST_DIVINT ||
         no->tipo == AST_MODULO) &&
        valor_numerico(no->b, &divisor) && divisor == 0.0) {

        erro_semantico(no->linha, "divisao por zero");
        return;
    }

    /* Verifica potencia de zero com expoente negativo. */
    if (no->tipo == AST_POTENCIA && zero_literal(no->a)) {
        double expoente;

        if (valor_numerico(no->b, &expoente) && expoente < 0.0) {
            erro_semantico(no->linha,
                           "divisao por zero em potencia com expoente negativo");
            return;
        }
    }

    if (incompatibilidade)
        return;

    /* Verifica overflow e resultados numericos invalidos. */
    double resultado;

    if (!resultado_operacao(no, &resultado))
        return;

    if (isnan(resultado) && no->tipo == AST_POTENCIA) {
        erro_semantico(no->linha, "potencia sem resultado real");
    } else if (!isfinite(resultado)) {
        erro_semantico(no->linha, mensagem_overflow(no->tipo));
    }
}

/* Executa a analise semantica de todas as expressoes registradas. */
int semantico_analisar(NoAST *raiz) {
    erros = 0;

    for (struct ExpressaoRegistrada *item = expressoes;
         item;
         item = item->prox) {

        analisar_no(item->raiz);
    }

    if (!expressoes && raiz)
        analisar_no(raiz);

    return erros;
}

/* Verifica se uma raiz ja esta anexada a AST utilizada para impressao. */
static int raiz_esta_impressa(NoAST *raiz_impressa, NoAST *raiz) {
    if (!raiz_impressa)
        return 0;

    for (NoAST *item = raiz_impressa->a; item; item = item->prox) {
        if (item == raiz)
            return 1;
    }

    return 0;
}

/* Libera as expressoes registradas e reinicia o estado do analisador. */
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
