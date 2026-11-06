#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ===== [P1] Registro auxiliar para valores numericos reais ===== */
typedef struct NumeroRealAST {
    const NoAST *no;
    double valor;
    struct NumeroRealAST *prox;
} NumeroRealAST;

static NumeroRealAST *numeros_reais;
/* ===== FIM [P1] Registro auxiliar para valores numericos reais ===== */

static char *duplicar_texto(const char *texto) {
    if (!texto) return NULL;
    size_t tamanho = strlen(texto) + 1;
    char *copia = malloc(tamanho);
    if (copia) memcpy(copia, texto, tamanho);
    return copia;
}

NoAST *ast_criar(TipoNo tipo, NoAST *a, NoAST *b, NoAST *c, int linha) {
    NoAST *no = calloc(1, sizeof *no);
    if (!no) return NULL;
    no->tipo = tipo;
    no->linha = linha;
    no->a = a;
    no->b = b;
    no->c = c;
    return no;
}

/* ===== [P1] Construcao e leitura de literais numericos ===== */
NoAST *ast_criar_numero(int valor, int linha) {
    NoAST *no = ast_criar(AST_NUMERO, NULL, NULL, NULL, linha);
    if (no) no->valor = valor;
    return no;
}

NoAST *ast_criar_numero_real(double valor, int linha) {
    NoAST *no = ast_criar(AST_NUMERO, NULL, NULL, NULL, linha);
    NumeroRealAST *registro = malloc(sizeof *registro);
    if (!no || !registro) {
        free(no);
        free(registro);
        return NULL;
    }
    registro->no = no;
    registro->valor = valor;
    registro->prox = numeros_reais;
    numeros_reais = registro;
    return no;
}

int ast_numero_valor(const NoAST *no, double *valor) {
    if (!no || no->tipo != AST_NUMERO || !valor) return 0;
    for (NumeroRealAST *item = numeros_reais; item; item = item->prox) {
        if (item->no == no) {
            *valor = item->valor;
            return 1;
        }
    }
    *valor = (double) no->valor;
    return 1;
}
/* ===== FIM [P1] Construcao e leitura de literais numericos ===== */

NoAST *ast_criar_texto(TipoNo tipo, const char *texto, int linha) {
    NoAST *no = ast_criar(tipo, NULL, NULL, NULL, linha);
    if (!no) return NULL;
    no->texto = duplicar_texto(texto ? texto : "");
    if (!no->texto) {
        free(no);
        return NULL;
    }
    return no;
}

NoAST *ast_anexar(NoAST *lista, NoAST *novo) {
    if (!lista) return novo;
    if (!novo) return lista;
    NoAST *ultimo = lista;
    while (ultimo->prox) ultimo = ultimo->prox;
    ultimo->prox = novo;
    return lista;
}

const char *ast_nome_tipo(TipoNo tipo) {
    switch (tipo) {
        case AST_PROGRAMA: return "PROGRAMA";
        case AST_BLOCO: return "BLOCO";
        /* ===== [P1] Nomes dos nos de expressoes e literais ===== */
        case AST_NUMERO: return "NUMERO";
        case AST_STRING: return "STRING";
        case AST_BOOL: return "BOOL";
        case AST_NONE: return "NONE";
        case AST_IDENTIFICADOR: return "IDENTIFICADOR";
        case AST_SOMA: return "SOMA";
        case AST_SUBTRACAO: return "SUBTRACAO";
        case AST_MULTIPLICACAO: return "MULTIPLICACAO";
        case AST_DIVISAO: return "DIVISAO";
        case AST_DIVINT: return "DIVINT";
        case AST_MODULO: return "MODULO";
        case AST_POTENCIA: return "POTENCIA";
        case AST_NEGATIVO: return "NEGATIVO";
        /* ===== FIM [P1] Nomes dos nos de expressoes e literais ===== */
        case AST_IGUAL: return "IGUAL";
        case AST_DIFERENTE: return "DIFERENTE";
        case AST_MENOR: return "MENOR";
        case AST_MAIOR: return "MAIOR";
        case AST_MENOR_IGUAL: return "MENOR_IGUAL";
        case AST_MAIOR_IGUAL: return "MAIOR_IGUAL";
        case AST_E: return "E";
        case AST_OU: return "OU";
        case AST_NAO: return "NAO";
        case AST_IF: return "IF";
        case AST_WHILE: return "WHILE";
        case AST_FOR: return "FOR";
        case AST_BREAK: return "BREAK";
        case AST_CONTINUE: return "CONTINUE";

        case AST_ATRIBUICAO: return "ATRIBUICAO";
        case AST_ATRIB_SOMA: return "ATRIB_SOMA";
        case AST_ATRIB_SUB: return "ATRIB_SUB";
        case AST_ATRIB_MULT: return "ATRIB_MULT";
        case AST_ATRIB_DIV: return "ATRIB_DIV";
        case AST_LISTA: return "LISTA";
        case AST_INDEXACAO: return "INDEXACAO";


        /* ===== [P5] Nomes dos nos de funcoes ===== */
        case AST_FUNCAO: return "FUNCAO";
        case AST_PARAMETRO: return "PARAMETRO";
        case AST_RETURN: return "RETURN";
        case AST_CHAMADA: return "CHAMADA";
        /* ===== FIM [P5] Nomes dos nos de funcoes ===== */
    }
    return "DESCONHECIDO";
}

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

/* ===== [P5] Construtores de funcoes e chamadas ===== */

/* def nome(parametros) { corpo }: o nome fica em texto, os parametros
 * (ja encadeados por prox) no filho a e o BLOCO do corpo no filho b. */
NoAST *ast_p5_funcao(const char *nome, NoAST *parametros, NoAST *corpo, int linha) {
    NoAST *no = ast_criar_texto(AST_FUNCAO, nome, linha);
    if (!no) {
        ast_liberar(parametros);
        ast_liberar(corpo);
        return NULL;
    }
    no->a = parametros;
    no->b = corpo;
    return no;
}

/* nome(argumentos): os argumentos sao expressoes encadeadas por prox. */
NoAST *ast_p5_chamada(const char *nome, NoAST *argumentos, int linha) {
    NoAST *no = ast_criar_texto(AST_CHAMADA, nome, linha);
    if (!no) {
        ast_liberar(argumentos);
        return NULL;
    }
    no->a = argumentos;
    return no;
}

int ast_p5_contar(const NoAST *lista) {
    int total = 0;
    for (; lista; lista = lista->prox) total++;
    return total;
}

/* ===== FIM [P5] Construtores de funcoes e chamadas ===== */

static void imprimir_texto(const char *texto) {
    putchar('\'');
    for (const unsigned char *p = (const unsigned char *) texto; p && *p; p++) {
        if (*p == '\\' || *p == '\'') putchar('\\');
        if (*p == '\n') fputs("\\n", stdout);
        else if (*p == '\r') fputs("\\r", stdout);
        else if (*p == '\t') fputs("\\t", stdout);
        else putchar(*p);
    }
    putchar('\'');
}

void ast_imprimir(NoAST *raiz, int nivel) {
    if (!raiz) return;
    for (int i = 0; i < nivel * 2; i++) putchar(' ');
    fputs(ast_nome_tipo(raiz->tipo), stdout);

    switch (raiz->tipo) {
        /* ===== [P1] Valores impressos nos nos de literais ===== */
        case AST_NUMERO: {
            double valor;
            (void) ast_numero_valor(raiz, &valor);
            printf(" %.15g", valor);
            break;
        }
        case AST_BOOL:
            printf(" %s", raiz->valor ? "True" : "False");
            break;
        /* ===== FIM [P1] Valores numericos e booleanos dos literais ===== */
        /* ===== [P1] Casos STRING e IDENTIFICADOR ===== */
        case AST_STRING:
        case AST_IDENTIFICADOR:
        /* ===== FIM [P1] Casos STRING e IDENTIFICADOR ===== */
        /* Estes casos compartilham a impressao textual com os nos P5 abaixo. */
        case AST_FUNCAO:
        case AST_PARAMETRO:
        case AST_CHAMADA:
        case AST_FOR:
            if (raiz->texto) {
                putchar(' ');
                imprimir_texto(raiz->texto);
            }
            break;
        default:
            break;
    }
    printf(" [linha %d]\n", raiz->linha);

    ast_imprimir(raiz->a, nivel + 1);
    ast_imprimir(raiz->b, nivel + 1);
    ast_imprimir(raiz->c, nivel + 1);
    ast_imprimir(raiz->prox, nivel);
}

void ast_liberar(NoAST *raiz) {
    if (!raiz) return;
    /* ===== [P1] Libera o registro auxiliar do literal numerico real ===== */
    NumeroRealAST **cursor = &numeros_reais;
    while (*cursor) {
        if ((*cursor)->no == raiz) {
            NumeroRealAST *removido = *cursor;
            *cursor = removido->prox;
            free(removido);
            break;
        }
        cursor = &(*cursor)->prox;
    }
    /* ===== FIM [P1] Libera o registro auxiliar do literal numerico real ===== */
    ast_liberar(raiz->a);
    ast_liberar(raiz->b);
    ast_liberar(raiz->c);
    ast_liberar(raiz->prox);
    free(raiz->texto);
    free(raiz);
}
