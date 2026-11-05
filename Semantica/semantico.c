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
