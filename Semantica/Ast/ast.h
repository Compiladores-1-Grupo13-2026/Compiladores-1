#ifndef MINIPY_AST_H
#define MINIPY_AST_H

/* Enum comum da Sprint 3. Cada fatia usa somente os seus tipos de nó. */
typedef enum {
    AST_PROGRAMA, AST_BLOCO,

    /* [P1] Expressoes e literais */
    AST_NUMERO, AST_STRING, AST_BOOL, AST_NONE, AST_IDENTIFICADOR,
    AST_SOMA, AST_SUBTRACAO, AST_MULTIPLICACAO, AST_DIVISAO,
    AST_DIVINT, AST_MODULO, AST_POTENCIA, AST_NEGATIVO,
    /* FIM [P1] Expressoes e literais */

    /* [P2] Condicoes e logica */
    AST_IGUAL, AST_DIFERENTE, AST_MENOR, AST_MAIOR,
    AST_MENOR_IGUAL, AST_MAIOR_IGUAL, AST_E, AST_OU, AST_NAO, AST_IF,

    /* [P3] Lacos */
    AST_WHILE, AST_FOR, AST_BREAK, AST_CONTINUE,

    /* [P4] Atribuicao e listas */
    AST_ATRIBUICAO, AST_ATRIB_SOMA, AST_ATRIB_SUB, AST_ATRIB_MULT,
    AST_ATRIB_DIV, AST_LISTA, AST_INDEXACAO,

    /* [P5] Funcoes */
    AST_FUNCAO, AST_PARAMETRO, AST_RETURN, AST_CHAMADA
} TipoNo;

typedef struct NoAST {
    TipoNo tipo;
    int linha;
    int valor;              /* NUMERO inteiro e BOOL */
    char *texto;            /* ID, STRING e nomes; o no e dono da copia */
    struct NoAST *a, *b, *c;
    struct NoAST *prox;     /* proximo item de uma lista */
} NoAST;

NoAST *ast_criar(TipoNo tipo, NoAST *a, NoAST *b, NoAST *c, int linha);
/* ===== [P1] API de valores numericos dos literais ===== */
NoAST *ast_criar_numero(int valor, int linha);
/* Preserva literais reais sem alterar a struct compartilhada. */
NoAST *ast_criar_numero_real(double valor, int linha);
int ast_numero_valor(const NoAST *no, double *valor);
/* ===== FIM [P1] API de valores numericos dos literais ===== */
NoAST *ast_criar_texto(TipoNo tipo, const char *texto, int linha);
NoAST *ast_anexar(NoAST *lista, NoAST *novo);
void ast_imprimir(NoAST *raiz, int nivel);
void ast_liberar(NoAST *raiz);
const char *ast_nome_tipo(TipoNo tipo);

#endif
