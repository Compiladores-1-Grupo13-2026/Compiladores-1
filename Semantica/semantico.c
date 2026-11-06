#include "semantico.h"
#include "Tabela/tabela.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Contador comum de erros semanticos (P1, P2, P3, P4 e P5). */
static int erros;

/* ===== [P4] Atribuição, listas e indexação ===== */

static int p4_mesmo_tipo(const char *a, const char *b) {
    return a && strcmp(a, b) == 0;
}

static const char *p4_ler(NoAST *no, P4Visitar visitar) {
    const char *tipo = visitar(no);
    /* A P1 ainda classifica NUMERO real como int. Atribuicoes/indices P4
     * consultam a API real sem mudar o visitante de outra fatia. */
    double numero;
    if (no && ast_numero_valor(no, &numero) &&
        isfinite(numero) && trunc(numero) != numero) return "float";
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
static int p4_numerico(const char *tipo) {
    return p4_mesmo_tipo(tipo, "int") || p4_mesmo_tipo(tipo, "float") ||
           p4_mesmo_tipo(tipo, "bool");
}

static const char *p4_tipo_composto(TipoNo op, const char *alvo, const char *valor) {
    if (p4_mesmo_tipo(alvo, "?") || p4_mesmo_tipo(valor, "?")) return "?";
    if (p4_numerico(alvo) && p4_numerico(valor)) {
        if (op == AST_ATRIB_DIV || p4_mesmo_tipo(alvo, "float") ||
            p4_mesmo_tipo(valor, "float")) return "float";
        return "int";
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

/* ===== [P1] Analise semantica de expressoes e literais ===== */

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
        case AST_NAO:
            return "bool";

        case AST_E:
        case AST_OU: {
            const char *a = semantico_tipo_expr(no->a);
            const char *b = semantico_tipo_expr(no->b);
            return tipos_iguais(a, b) ? a : "?";
        }

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
 * Igualdade e diferenca aceitam quaisquer tipos, conforme o contrato runtime.
 *
 * Comparacoes de ordem:
 *   - aceitam numeros (inclusive bool/float) ou dois textos;
 *   - consultam os identificadores na tabela e ignoram tipos desconhecidos.
 *
 * Operadores logicos:
 *   - aceitam operandos de qualquer tipo, seguindo a ideia de
 *     avaliacao por verdadeiro/falso.
 */
static const char *p2_tipo_operando(const NoAST *no) {
    if (no && no->tipo == AST_IDENTIFICADOR) {
        Simbolo *simbolo = tabela_buscar(no->texto);
        return simbolo ? simbolo->tipo : "?";
    }
    return semantico_tipo_expr(no);
}

static int p2_numerico(const char *tipo) {
    return tipos_numericos(tipo) || strcmp(tipo, "float") == 0 ||
           strcmp(tipo, "bool") == 0;
}

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

    a = p2_tipo_operando(no->a);
    b = p2_tipo_operando(no->b);

    /* Nao verifica expressoes que contenham identificadores desconhecidos. */
    if (a[0] == '?' || b[0] == '?')
        return 0;

    if (eh_comparacao_igualdade(no->tipo)) return 0;

    if (eh_comparacao_ordem(no->tipo))
        return !((p2_numerico(a) && p2_numerico(b)) ||
                 (tipos_iguais(a, "str") && tipos_iguais(b, "str")));

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
                 p2_tipo_operando(no->a),
                 p2_tipo_operando(no->b));
    }

    erro_semantico(no->linha, mensagem);
}

/* ===== FIM [P2] Analise semantica de comparacoes e logica ===== */

/*
 * Verifica UM no de expressao (literais, aritmetica e comparacoes).
 * [P5] Antes este trecho tambem descia nos filhos; agora o visitante geral
 * (secao P5) percorre a arvore e chama esta funcao em pos-ordem.
 */
static void verificar_no(NoAST *no) {
    if (!no) return;

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

/* ===== FIM [P1] Analise semantica de expressoes e literais ===== */

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
/* ===== FIM [P3] LACOS: estado e auxiliares ===== */

/* ===== [P5] Funcoes, chamadas, return e percurso da AST ===== */

/* Quantas definicoes de funcao envolvem o no visitado (0 = nivel global).
 * return so e valido quando for maior que zero. */
static int profundidade_funcao = 0;

/* Diagnostico no formato comum, aceitando argumentos como o printf.
 * Tem a mesma assinatura do callback P4Erro usado pela secao P4. */
static void erro_formatado(int linha, const char *formato, ...) {
    va_list argumentos;
    fprintf(stderr, "[ERRO SEMANTICO] Linha %d: ", linha);
    va_start(argumentos, formato);
    vfprintf(stderr, formato, argumentos);
    va_end(argumentos);
    fputc('\n', stderr);
    erros++;
}

static const char *visitar(NoAST *no);

/* Coloca o nome da funcao no escopo atual, com a quantidade de parametros.
 * Um nome repetido no MESMO escopo e erro (redeclaracao). */
static void registrar_funcao(NoAST *funcao) {
    /* Parametro repetido: compara cada parametro com os anteriores. */
    for (NoAST *parametro = funcao->a; parametro; parametro = parametro->prox) {
        for (NoAST *anterior = funcao->a; anterior != parametro; anterior = anterior->prox) {
            if (strcmp(anterior->texto, parametro->texto) == 0) {
                erro_formatado(parametro->linha, "parametro '%s' repetido na funcao '%s'",
                               parametro->texto, funcao->texto);
                break;
            }
        }
    }

    Simbolo *existente = tabela_buscar_local(funcao->texto);
    if (existente) {
        if (existente->categoria == CAT_BUILTIN)
            erro_formatado(funcao->linha,
                           "nao e permitido redefinir a funcao embutida '%s'",
                           funcao->texto);
        else if (existente->categoria == CAT_FUNCAO)
            erro_formatado(funcao->linha, "funcao '%s' ja definida na linha %d",
                           funcao->texto, existente->linha);
        else
            erro_formatado(funcao->linha, "nome '%s' ja usado na linha %d",
                           funcao->texto, existente->linha);
        return;
    }
    tabela_inserir(funcao->texto, CAT_FUNCAO, "funcao", funcao->linha);
    Simbolo *simbolo = tabela_buscar_local(funcao->texto);
    if (simbolo) simbolo->num_params = ast_p5_contar(funcao->a);
}

/* Visita uma sequencia de comandos (corpo do PROGRAMA ou de um BLOCO).
 * 1a passada: registra as funcoes definidas na sequencia, para permitir
 * a recursao e que um corpo chame uma funcao definida mais abaixo.
 * 2a passada: visita cada comando na ordem do codigo-fonte. */
static void visitar_lista(NoAST *primeiro) {
    for (NoAST *comando = primeiro; comando; comando = comando->prox)
        if (comando->tipo == AST_FUNCAO) registrar_funcao(comando);
    for (NoAST *comando = primeiro; comando; comando = comando->prox)
        visitar(comando);
}

/* def nome(p1, p2) { corpo }: cada funcao ganha o seu proprio escopo.
 * Parametros e variaveis atribuidas no corpo morrem ao sair dele. */
static void visitar_funcao(NoAST *funcao) {
    /* Combinado com a P3: o corpo do def nao herda o laco que o envolve. */
    int laco_salvo = profundidade_laco;
    profundidade_laco = 0;
    profundidade_funcao++;

    tabela_entrar_escopo(funcao->texto);
    /* Um repetido ja foi relatado em registrar_funcao; aqui so falha a insercao. */
    for (NoAST *parametro = funcao->a; parametro; parametro = parametro->prox)
        tabela_inserir(parametro->texto, CAT_PARAMETRO, "?", parametro->linha);
    visitar(funcao->b);
    tabela_sair_escopo();

    profundidade_funcao--;
    profundidade_laco = laco_salvo;
}

/* nome(a1, a2): confere se o nome existe, se e chamavel e a aridade. */
static const char *visitar_chamada(NoAST *chamada) {
    int recebidos = 0;
    for (NoAST *argumento = chamada->a; argumento; argumento = argumento->prox) {
        visitar(argumento);
        recebidos++;
    }

    Simbolo *simbolo = tabela_buscar(chamada->texto);
    if (!simbolo) {
        erro_formatado(chamada->linha, "funcao '%s' nao definida", chamada->texto);
    } else if (simbolo->categoria == CAT_BUILTIN) {
        /* print aceita qualquer quantidade; range aceita de 1 a 3. */
        if (strcmp(chamada->texto, "range") == 0 && (recebidos < 1 || recebidos > 3))
            erro_formatado(chamada->linha,
                           "funcao 'range' espera de 1 a 3 argumentos, recebeu %d",
                           recebidos);
    } else if (simbolo->categoria == CAT_FUNCAO) {
        /* Fora de funcoes o codigo roda de cima para baixo: o def precisa
         * vir antes. Dentro de um corpo, a chamada so acontece quando a
         * funcao for executada, entao um def mais abaixo e aceito. */
        if (profundidade_funcao == 0 && simbolo->linha > chamada->linha)
            erro_formatado(chamada->linha,
                           "funcao '%s' chamada antes de ser definida (def na linha %d)",
                           chamada->texto, simbolo->linha);
        else if (simbolo->num_params != recebidos)
            erro_formatado(chamada->linha,
                           "funcao '%s' espera %d argumento(s), recebeu %d",
                           chamada->texto, simbolo->num_params, recebidos);
    } else if (strcmp(simbolo->tipo, "?") != 0) {
        /* Parametros e valores de tipo desconhecido podem guardar uma funcao;
         * um int, str, bool ou lista conhecido nao pode ser chamado. */
        erro_formatado(chamada->linha, "'%s' nao e uma funcao (tipo '%s')",
                       chamada->texto, simbolo->tipo);
    }
    return "?";   /* O tipo de retorno so sera conhecido na execucao. */
}

/* return [expr]: so pode aparecer dentro do corpo de uma funcao. */
static void visitar_return(NoAST *retorno) {
    if (profundidade_funcao == 0)
        erro_formatado(retorno->linha, "'return' fora de uma funcao");
    visitar(retorno->a);
}

/* [P3] Lacos: regras de Gabriel, ligadas ao percurso comum. */
static void visitar_laco(NoAST *no) {
    switch (no->tipo) {
        case AST_WHILE:
            profundidade_laco++;
            visitar(no->a); /* Condicao */
            visitar(no->b); /* Corpo */
            profundidade_laco--;
            break;
        case AST_FOR:
            /* Registra a variavel do 'for' na tabela como CAT_VARIAVEL. */
            if (no->texto)
                tabela_inserir(no->texto, CAT_VARIAVEL, tipo_variavel_for(no->a), no->linha);
            /* O iteravel nao pode ser um literal numerico isolado. */
            if (no->a && no->a->tipo == AST_NUMERO)
                erro_semantico(no->linha, "iteravel do 'for' nao pode ser um numero literal");
            profundidade_laco++;
            visitar(no->a); /* Iteravel */
            visitar(no->b); /* Corpo */
            profundidade_laco--;
            break;
        case AST_BREAK:
            if (profundidade_laco <= 0) erro_semantico(no->linha, "'break' fora de um laco");
            break;
        case AST_CONTINUE:
            if (profundidade_laco <= 0) erro_semantico(no->linha, "'continue' fora de um laco");
            break;
        default:
            break;
    }
}

/* Visitante geral: visita UM no e devolve o tipo da expressao ("none" para
 * comandos). Cada fatia trata os seus tipos; quem e dono de uma lista
 * (PROGRAMA, BLOCO, chamada, LISTA) e quem percorre o prox. */
static const char *visitar(NoAST *no) {
    if (!no) return "none";

    switch (no->tipo) {
        case AST_PROGRAMA:
        case AST_BLOCO:
            visitar_lista(no->a);
            return "none";
        case AST_FUNCAO:
            visitar_funcao(no);
            return "none";
        case AST_PARAMETRO:
            return "none";
        case AST_RETURN:
            visitar_return(no);
            return "none";
        case AST_CHAMADA:
            return visitar_chamada(no);
        case AST_IF:            /* [P2] a = condicao, b = bloco, c = elif/else */
            visitar(no->a);
            visitar(no->b);
            visitar(no->c);
            return "none";
        case AST_WHILE:
        case AST_FOR:
        case AST_BREAK:
        case AST_CONTINUE:
            visitar_laco(no);
            return "none";
        default:
            break;
    }

    /* [P4] Atribuicoes, listas, indexacao e leitura de identificadores. */
    const char *tipo_p4 = semantico_p4_visitar(no, visitar, erro_formatado);
    if (tipo_p4) return tipo_p4;

    /* [P1]/[P2] Expressoes: filhos primeiro (pos-ordem), depois o operador. */
    visitar(no->a);
    visitar(no->b);
    visitar(no->c);
    verificar_no(no);
    return semantico_tipo_expr(no);
}

int semantico_analisar(NoAST *raiz) {
    erros = 0;
    profundidade_laco = 0;
    profundidade_funcao = 0;
    visitar(raiz);
    tabela_liberar();
    return erros;
}

/* ===== FIM [P5] Funcoes, chamadas, return e percurso da AST ===== */
