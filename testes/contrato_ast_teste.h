/* ===== [P4] Contrato de teste das dependencias da AST ===== */
#ifndef MINIPY_P4_CONTRATO_TESTE_H
#define MINIPY_P4_CONTRATO_TESTE_H

/* SOMENTE TESTE: espelho declarativo da secao 3.1 do plano da Sprint 3.
 * Nao e o header de producao da P1 e nao deve entrar no interpretador.
 * O runner injeta esta interface para testar a P4 com a base ainda pendente.
 * Os nomes de todas as fatias preservam os valores do enum combinado. */
typedef enum {
    AST_PROGRAMA, AST_BLOCO,
    /* [P1] */
    AST_NUMERO, AST_STRING, AST_BOOL, AST_NONE, AST_IDENTIFICADOR,
    AST_SOMA, AST_SUBTRACAO, AST_MULTIPLICACAO, AST_DIVISAO,
    AST_DIVINT, AST_MODULO, AST_POTENCIA, AST_NEGATIVO,
    /* [P2] */
    AST_IGUAL, AST_DIFERENTE, AST_MENOR, AST_MAIOR,
    AST_MENOR_IGUAL, AST_MAIOR_IGUAL, AST_E, AST_OU, AST_NAO, AST_IF,
    /* [P3] */
    AST_WHILE, AST_FOR, AST_BREAK, AST_CONTINUE,
    /* [P4] */
    AST_ATRIBUICAO, AST_ATRIB_SOMA, AST_ATRIB_SUB, AST_ATRIB_MULT,
    AST_ATRIB_DIV, AST_LISTA, AST_INDEXACAO,
    /* [P5] */
    AST_FUNCAO, AST_PARAMETRO, AST_RETURN, AST_CHAMADA
} TipoNo;

typedef struct NoAST {
    TipoNo tipo;
    int linha;
    int valor;
    char *texto;
    struct NoAST *a, *b, *c;
    struct NoAST *prox;
} NoAST;

NoAST *ast_criar(TipoNo tipo, NoAST *a, NoAST *b, NoAST *c, int linha);
void ast_imprimir(NoAST *raiz, int nivel);

#endif
