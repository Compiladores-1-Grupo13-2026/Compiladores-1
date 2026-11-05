
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "semantico.h"
#include "Tabela/tabela.h"

/* ===== [P4] Atribuição, listas e indexação ===== */

static int p4_mesmo_tipo(const char *a, const char *b) {
    return a && strcmp(a, b) == 0;
}

static const char *p4_ler(NoAST *no, P4Visitar visitar) {
    const char *tipo = visitar(no);
    return tipo ? tipo : "?";
}

static const char *p4_identificador(NoAST *no, P4Erro erro) {
    Simbolo *simbolo = tabela_buscar(no->texto);
    if (!simbolo) {
        erro(no->linha, "variável '%s' usada antes de ser atribuída", no->texto);
        return P4_TIPO_ERRO;
    }
    return simbolo->tipo;
}

static const char *p4_indexacao(NoAST *no, P4Visitar visitar, P4Erro erro) {
    const char *base = p4_ler(no->a, visitar);
    const char *indice = p4_ler(no->b, visitar);
    int invalido = p4_mesmo_tipo(base, P4_TIPO_ERRO) || p4_mesmo_tipo(indice, P4_TIPO_ERRO);
    if (!p4_mesmo_tipo(base, P4_TIPO_ERRO) && !p4_mesmo_tipo(base, "lista") &&
        !p4_mesmo_tipo(base, "?")) {
        erro(no->linha, "indexação exige lista; recebido '%s'", base);
        invalido = 1;
    }
    if (!p4_mesmo_tipo(indice, P4_TIPO_ERRO) && !p4_mesmo_tipo(indice, "int") &&
        !p4_mesmo_tipo(indice, "?")) {
        erro(no->b->linha, "índice de lista deve ser int; recebido '%s'", indice);
        invalido = 1;
    }
    /* A P1 preserva reais fora de NoAST.valor e ainda os classifica como int.
     * Consultar a API numerica permite recusar indices literais fracionarios.
     * Valores de expressoes e limites da lista ficam para a execucao. */
    double valor_indice;
    if (p4_mesmo_tipo(indice, "int") && ast_numero_valor(no->b, &valor_indice) &&
        (!isfinite(valor_indice) || trunc(valor_indice) != valor_indice)) {
        erro(no->b->linha, "índice de lista deve ser inteiro finito");
        invalido = 1;
    }
    /* O contrato nao guarda tipo por elemento: uma leitura l[i] tem tipo ?. */
    return invalido ? P4_TIPO_ERRO : "?";
}

/* Verifica somente operadores compostos P4. Operadores de expressoes comuns
 * continuam sob responsabilidade da P1; nao ha execucao nem teste de valores. */
static const char *p4_tipo_composto(TipoNo op, const char *alvo, const char *valor) {
    if (p4_mesmo_tipo(alvo, "?") || p4_mesmo_tipo(valor, "?")) return "?";
    if (p4_mesmo_tipo(alvo, "int") && p4_mesmo_tipo(valor, "int")) {
        /* O contrato int nao representa o resultado geral de /=. */
        return op == AST_ATRIB_DIV ? "?" : "int";
    }
    if (op == AST_ATRIB_SOMA && p4_mesmo_tipo(alvo, valor) &&
        (p4_mesmo_tipo(alvo, "str") || p4_mesmo_tipo(alvo, "lista"))) return alvo;
    if (op == AST_ATRIB_MULT && p4_mesmo_tipo(valor, "int") &&
        (p4_mesmo_tipo(alvo, "str") || p4_mesmo_tipo(alvo, "lista"))) return alvo;
    return P4_TIPO_ERRO;
}

static const char *p4_atribuicao(NoAST *no, P4Visitar visitar, P4Erro erro) {
    NoAST *alvo = no->a;
    int simples = no->tipo == AST_ATRIBUICAO;
    int invalido = 0;
    Simbolo *local = NULL;
    const char *tipo_alvo = "?";
    if (alvo->tipo == AST_IDENTIFICADOR) {
        Simbolo *visivel = tabela_buscar(alvo->texto);
        local = tabela_buscar_local(alvo->texto);
        if (visivel && (visivel->categoria == CAT_FUNCAO ||
                        visivel->categoria == CAT_BUILTIN)) {
            erro(alvo->linha, "não é permitido atribuir ao nome de função ou builtin '%s'",
                 alvo->texto);
            invalido = 1;
        } else if (!simples && !local) {
            erro(alvo->linha, "atribuição composta exige variável '%s' já atribuída no escopo atual",
                 alvo->texto);
            invalido = 1;
        } else if (local) {
            tipo_alvo = local->tipo;
        }
    } else if (alvo->tipo == AST_INDEXACAO) {
        tipo_alvo = p4_indexacao(alvo, visitar, erro);
        invalido = p4_mesmo_tipo(tipo_alvo, P4_TIPO_ERRO);
    } else {
        erro(alvo->linha, "alvo inválido de atribuição: esperado identificador ou indexação");
        invalido = 1;
    }

    /* Visitar RHS antes de inserir impede y = y + 1 de declarar y cedo demais.
     * Mesmo com alvo invalido, continua para relatar erros independentes. */
    const char *tipo_valor = p4_ler(no->b, visitar);
    if (p4_mesmo_tipo(tipo_valor, P4_TIPO_ERRO)) invalido = 1;
    /* ast_criar_numero_real pode deixar valor zerado mesmo para 2 ou 0.5. */
    double divisor;
    if (no->tipo == AST_ATRIB_DIV && ast_numero_valor(no->b, &divisor) && divisor == 0.0) {
        erro(no->b->linha, "divisão por zero na atribuição composta '/='");
        invalido = 1;
    }
    if (invalido) return P4_TIPO_ERRO;
    const char *tipo_resultado = simples ? tipo_valor :
                                p4_tipo_composto(no->tipo, tipo_alvo, tipo_valor);
    if (p4_mesmo_tipo(tipo_resultado, P4_TIPO_ERRO)) {
        erro(no->linha, "tipos incompatíveis na atribuição composta: '%s' e '%s'",
             tipo_alvo, tipo_valor);
        return P4_TIPO_ERRO;
    }

    if (alvo->tipo == AST_IDENTIFICADOR) {
        if (!local) {
            tabela_inserir(alvo->texto, CAT_VARIAVEL, tipo_resultado, no->linha);
        } else {
            /* tipo_resultado pode apontar para local->tipo: copia intermediaria
             * evita sobreposicao indefinida no snprintf da atualizacao. */
            char tipo_copia[sizeof local->tipo] = {0};
            snprintf(tipo_copia, sizeof tipo_copia, "%s", tipo_resultado);
            memcpy(local->tipo, tipo_copia, sizeof tipo_copia);
        }
    }
    return "none";
}

const char *semantico_p4_visitar(NoAST *no, P4Visitar visitar, P4Erro erro) {
    if (!no) return "none";
    switch (no->tipo) {
        case AST_IDENTIFICADOR:
            return p4_identificador(no, erro);
        case AST_ATRIBUICAO:
        case AST_ATRIB_SOMA:
        case AST_ATRIB_SUB:
        case AST_ATRIB_MULT:
        case AST_ATRIB_DIV:
            return p4_atribuicao(no, visitar, erro);
        case AST_LISTA: {
            int invalido = 0;
            for (NoAST *item = no->a; item; item = item->prox) {
                if (p4_mesmo_tipo(p4_ler(item, visitar), P4_TIPO_ERRO)) invalido = 1;
            }
            return invalido ? P4_TIPO_ERRO : "lista";
        }
        case AST_INDEXACAO:
            return p4_indexacao(no, visitar, erro);
        default:
            return NULL;
    }
}

/* ===== FIM [P4] Atribuição, listas e indexação ===== */
=======
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

/* ============================================================================
 * P3 (Gabriel) - LACOS - trecho de Semantica/semantico.c
 *
 * Sao 2 pedacos para colar no semantico.c (esqueleto do P2).
 *
 * Suposicoes (contrato do plano, secoes 3.4 a 3.6):
 *   - visitar(NoAST *no) visita UM no e aceita NULL; o case de BLOCO/PROGRAMA e
 *     quem percorre a lista pelo prox;
 *   - erro_semantico(int linha, const char *formato, ...) conta o erro e segue;
 *   - tabela.h (tabela_buscar, tabela_buscar_local, tabela_inserir, Categoria)
 *     e ast.h ja estao incluidos.
 *
 * COMBINADO COM O P5 (case AST_FUNCAO): o corpo de um def NAO herda o laco que
 * o envolve (break dentro de def dentro de while e erro). O P5 precisa de 3
 * linhas em volta da visita do corpo da funcao:
 *     int laco_salvo = profundidade_laco;
 *     profundidade_laco = 0;
 *     ... visita parametros e corpo ...
 *     profundidade_laco = laco_salvo;
 * O teste invalidos/p3_break_em_def_dentro_de_laco cobra isso.
 * ========================================================================== */


/* ---------- PEDACO 1: topo do semantico.c, antes de visitar() -------------- */
#include <string.h>

/* ===== [P3] LACOS: estado e auxiliares ===== */
/* Quantos while/for envolvem o no visitado, dentro da funcao atual.
 * break e continue so sao validos quando for maior que zero. */
static int profundidade_laco = 0;

/* Tipo guardado na tabela para a variavel de controle do for. */
static const char *tipo_variavel_for(const NoAST *iteravel) {
    if (!iteravel) return "?";
    if (iteravel->tipo == AST_CHAMADA && iteravel->texto &&
        strcmp(iteravel->texto, "range") == 0) return "int";
    if (iteravel->tipo == AST_STRING) return "str";
    return "?";                         /* lista, identificador, expressao... */
}

/* Literais que nunca sao iteraveis (for i in 5). NULL = nao sabemos / pode ser. */
static const char *literal_nao_iteravel(const NoAST *no) {
    if (!no) return NULL;
    switch (no->tipo) {
    case AST_NUMERO: return "um numero";
    case AST_BOOL:   return "um booleano";
    case AST_NONE:   return "None";
    default:         return NULL;
    }
}


/* ---------- PEDACO 2: dentro do switch (no->tipo) de visitar() ------------- */
    /* ===== [P3] LAÇOS ===== */

case AST_WHILE: {
    profundidade_laco++;
    semantico_visitar(no->a); /* Condição */
    semantico_visitar(no->b); /* Corpo */
    profundidade_laco--;
    break;
}

case AST_FOR: {
    /* 1. Registra a variável do 'for' na Tabela de Símbolos como CAT_VARIAVEL */
    if (no->texto) {
        tabela_inserir(no->texto, CAT_VARIAVEL, "int", no->linha);
    }

    /* 2. Verifica se o iterável não é um literal numérico isolado */
    if (no->a && no->a->tipo == AST_NUMERO) {
        erro_semantico(no->linha, "iteravel do 'for' nao pode ser um numero literal");
    }

    profundidade_laco++;
    semantico_visitar(no->a); /* Iterável */
    semantico_visitar(no->b); /* Corpo */
    profundidade_laco--;
    break;
}

case AST_BREAK: {
    if (profundidade_laco <= 0) {
        erro_semantico(no->linha, "'break' fora de um laco");
    }
    break;
}

case AST_CONTINUE: {
    if (profundidade_laco <= 0) {
        erro_semantico(no->linha, "'continue' fora de um laco");
    }
    break;
}

case AST_FUNCAO: {
    /* ATENÇÃO: Definição de função zera a profundidade de laço interna */
    int profundidade_anterior = profundidade_laco;
    profundidade_laco = 0;
    
    /* Visita parâmetros e corpo da função */
    semantico_visitar(no->a);
    semantico_visitar(no->b);
    
    profundidade_laco = profundidade_anterior;
    break;
}
