#include "Interpretador/interpretador.h"
#include "Interpretador/ambiente.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>

Sinal sinal_atual = SINAL_NORMAL;
Valor valor_retorno;

static jmp_buf ponto_de_erro;   /* para onde erro_execucao volta */

void erro_execucao(int linha, const char *formato, ...) {
    va_list args;
    fflush(stdout);   /* o que ja foi impresso aparece antes do erro */
    fprintf(stderr, "[ERRO EXECUCAO] Linha %d: ", linha);
    va_start(args, formato);
    vfprintf(stderr, formato, args);
    va_end(args);
    fputc('\n', stderr);
    longjmp(ponto_de_erro, 1);
}

void executar_bloco(NoAST *primeiro) {
    for (NoAST *c = primeiro; c && sinal_atual == SINAL_NORMAL; c = c->prox)
        valor_liberar(interpretar_no(c));
}

Valor interpretar_no(NoAST *no) {
    if (!no) return valor_none();
    switch (no->tipo) {
        case AST_PROGRAMA: case AST_BLOCO:
            executar_bloco(no->a);
            return valor_none();
        /* [P1] */
        case AST_NUMERO: case AST_STRING: case AST_BOOL: case AST_NONE:
        case AST_SOMA: case AST_SUBTRACAO: case AST_MULTIPLICACAO: case AST_DIVISAO:
        case AST_DIVINT: case AST_MODULO: case AST_POTENCIA: case AST_NEGATIVO:
            return executar_expressao(no);
        /* [P2] */
        case AST_IGUAL: case AST_DIFERENTE: case AST_MENOR: case AST_MAIOR:
        case AST_MENOR_IGUAL: case AST_MAIOR_IGUAL: case AST_E: case AST_OU:
        case AST_NAO: case AST_IF:
            return executar_condicao(no);
        /* [P3] */
        case AST_WHILE: case AST_FOR: case AST_BREAK: case AST_CONTINUE:
            return executar_laco(no);
        /* [P4] */
        case AST_IDENTIFICADOR: case AST_ATRIBUICAO: case AST_ATRIB_SOMA:
        case AST_ATRIB_SUB: case AST_ATRIB_MULT: case AST_ATRIB_DIV:
        case AST_LISTA: case AST_INDEXACAO:
            return executar_atribuicao(no);
        /* [P5] */
        case AST_FUNCAO: case AST_PARAMETRO: case AST_RETURN: case AST_CHAMADA:
            return executar_funcao(no);
    }
    erro_execucao(no->linha, "no da AST desconhecido");
    return valor_none();
}

int interpretar_programa(NoAST *raiz) {
    ambiente_iniciar();
    sinal_atual = SINAL_NORMAL;
    valor_retorno = valor_none();
    if (setjmp(ponto_de_erro)) {   /* chegou aqui vindo de erro_execucao */
        ambiente_liberar();
        return 1;
    }
    interpretar_no(raiz);
    fflush(stdout);
    ambiente_liberar();
    return 0;
}
