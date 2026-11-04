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