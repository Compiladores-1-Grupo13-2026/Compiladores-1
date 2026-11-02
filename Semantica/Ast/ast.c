#include <stdio.h>
#include "ast.h"

/* ===== [P4] Atribuição, listas e indexação ===== */

NoAST *ast_p4_atribuicao(TipoNo operador, NoAST *alvo, NoAST *valor, int linha) {
    return ast_criar(operador, alvo, valor, NULL, linha);
}

NoAST *ast_p4_lista(NoAST *elementos, int linha) {
    return ast_criar(AST_LISTA, elementos, NULL, NULL, linha);
}

NoAST *ast_p4_indexacao(NoAST *base, NoAST *indice, int linha) {
    return ast_criar(AST_INDEXACAO, base, indice, NULL, linha);
}

const char *ast_p4_nome_tipo(TipoNo tipo) {
    switch (tipo) {
        case AST_ATRIBUICAO: return "ATRIBUICAO";
        case AST_ATRIB_SOMA: return "ATRIB_SOMA";
        case AST_ATRIB_SUB: return "ATRIB_SUB";
        case AST_ATRIB_MULT: return "ATRIB_MULT";
        case AST_ATRIB_DIV: return "ATRIB_DIV";
        case AST_LISTA: return "LISTA";
        case AST_INDEXACAO: return "INDEXACAO";
        default: return NULL;
    }
}

int ast_p4_imprimir(NoAST *no, int nivel) {
    const char *nome = no ? ast_p4_nome_tipo(no->tipo) : NULL;
    if (!nome) return 0;
    printf("%*s%s [linha %d]\n", nivel * 2, "", nome, no->linha);
    ast_imprimir(no->a, nivel + 1);
    ast_imprimir(no->b, nivel + 1);
    ast_imprimir(no->c, nivel + 1);
    return 1;
}

/* ===== FIM [P4] Atribuição, listas e indexação ===== */
