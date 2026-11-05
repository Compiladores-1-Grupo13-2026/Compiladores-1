#ifndef MINIPY_AST_H
#define MINIPY_AST_H


/* ===== [P4] Atribuição, listas e indexação ===== */

/* NoAST, TipoNo e API comum serao acrescentados pela base do grupo. */

/* Os construtores recebem a propriedade dos filhos; cada filho deve ter um
 * unico dono. A lista de elementos ja vem encadeada por ast_anexar/prox. */
NoAST *ast_p4_atribuicao(TipoNo operador, NoAST *alvo, NoAST *valor, int linha);
NoAST *ast_p4_lista(NoAST *elementos, int linha);
NoAST *ast_p4_indexacao(NoAST *base, NoAST *indice, int linha);

/* Retorna NULL para os tipos das outras fatias. O impressor comum pode usar
 * esse nome e percorrer a/b/c/prox no formato acordado (2 espacos por nivel). */
const char *ast_p4_nome_tipo(TipoNo tipo);
/* Imprime um no P4 e seus filhos via ast_imprimir; nao percorre no->prox,
 * responsabilidade do impressor comum. Retorna 0 quando nao for um no P4. */
int ast_p4_imprimir(NoAST *no, int nivel);

/* ===== FIM [P4] Atribuição, listas e indexação ===== */
=======
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
