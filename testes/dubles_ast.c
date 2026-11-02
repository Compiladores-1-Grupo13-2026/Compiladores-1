/* ===== [P4] Dubles de teste das dependencias da AST ===== */
/* Dubles exclusivos dos testes P4. Sem alocacao dinamica, interpretacao,
 * percurso, liberacao ou impressao geral da AST das outras fatias. */
#include <assert.h>
#include <string.h>
#include "dubles_ast.h"

enum { CAPACIDADE_NOS = 1024, CAPACIDADE_TEXTO = 128, CAPACIDADE_CHAMADAS = 16 };
static NoAST nos[CAPACIDADE_NOS];
static char textos[CAPACIDADE_NOS][CAPACIDADE_TEXTO];
static size_t usados;
static TesteImpressao chamadas[CAPACIDADE_CHAMADAS];
static size_t total_chamadas;

void teste_impressao_reiniciar(void) {
    memset(chamadas, 0, sizeof chamadas);
    total_chamadas = 0;
}

void teste_reiniciar(void) {
    memset(nos, 0, sizeof nos);
    memset(textos, 0, sizeof textos);
    usados = 0;
    teste_impressao_reiniciar();
}

NoAST *teste_no(TipoNo tipo, int linha) {
    assert(usados < CAPACIDADE_NOS);
    NoAST *no = &nos[usados++];
    no->tipo = tipo;
    no->linha = linha;
    return no;
}

NoAST *teste_numero(int valor, int linha) {
    NoAST *no = teste_no(AST_NUMERO, linha);
    no->valor = valor;
    return no;
}

NoAST *teste_texto(TipoNo tipo, const char *texto, int linha) {
    assert(texto && strlen(texto) < CAPACIDADE_TEXTO);
    NoAST *no = teste_no(tipo, linha);
    no->texto = textos[usados - 1];
    memcpy(no->texto, texto, strlen(texto) + 1);
    return no;
}

/* Unico construtor comum necessario para ligar os wrappers P4: devolve
 * registros da arena de teste, sem implementar o gerenciamento da P1. */
NoAST *ast_criar(TipoNo tipo, NoAST *a, NoAST *b, NoAST *c, int linha) {
    NoAST *no = teste_no(tipo, linha);
    no->a = a;
    no->b = b;
    no->c = c;
    return no;
}

/* Spy: registra a delegacao da P4 sem percorrer nem imprimir qualquer filho. */
void ast_imprimir(NoAST *no, int nivel) {
    assert(total_chamadas < CAPACIDADE_CHAMADAS);
    chamadas[total_chamadas].no = no;
    chamadas[total_chamadas].nivel = nivel;
    total_chamadas++;
}

size_t teste_impressao_total(void) {
    return total_chamadas;
}

const TesteImpressao *teste_impressao_chamada(size_t indice) {
    assert(indice < total_chamadas);
    return &chamadas[indice];
}
