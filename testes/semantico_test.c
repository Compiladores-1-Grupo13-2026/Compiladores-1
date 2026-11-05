/* ===== [P4] Testes de atribuicao, listas e indexacao =====
 * Testa os arquivos compartilhados com dados pre-montados e dubles de dependencia.
 * O callback fornece metadados de teste; nao analisa expressoes da P1. */
#include "dubles_ast.h"
#include "Semantica/Ast/ast.h"
#include "Semantica/Tabela/tabela.h"
#include "Semantica/semantico.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static int erros;
static int ultima_linha;
static char mensagens[8192];

static void erro_teste(int linha, const char *formato, ...) {
    size_t usados = strlen(mensagens);
    va_list args;
    va_start(args, formato);
    vsnprintf(mensagens + usados, sizeof mensagens - usados, formato, args);
    va_end(args);
    usados = strlen(mensagens);
    if (usados + 1 < sizeof mensagens) {
        mensagens[usados] = '\n';
        mensagens[usados + 1] = '\0';
    }
    ultima_linha = linha;
    erros++;
}

static const char *visitar(NoAST *no) {
    const char *tipo = semantico_p4_visitar(no, visitar, erro_teste);
    if (tipo) return tipo;
    switch (no->tipo) {
        case AST_NUMERO: return "int";
        case AST_STRING: return "str";
        case AST_BOOL: return "bool";
        case AST_NONE: return "none";
        default: return "?"; /* Interface de uma expressao ainda sem tipo. */
    }
}

static NoAST *id(const char *nome, int linha) {
    return teste_texto(AST_IDENTIFICADOR, nome, linha);
}

static NoAST *numero(int valor, int linha) {
    return teste_numero(valor, linha);
}

static NoAST *texto(const char *valor, int linha) {
    return teste_texto(AST_STRING, valor, linha);
}

static NoAST *atribuir(TipoNo operador, const char *nome, NoAST *valor, int linha) {
    return ast_p4_atribuicao(operador, id(nome, linha), valor, linha);
}

static void verificar(NoAST *no, const char *tipo, int novos_erros) {
    int anteriores = erros;
    const char *obtido = visitar(no);
    assert(obtido && strcmp(obtido, tipo) == 0);
    assert(erros == anteriores + novos_erros);
}

static void tipo_simbolo(const char *nome, const char *tipo, int linha) {
    Simbolo *simbolo = tabela_buscar(nome);
    assert(simbolo && strcmp(simbolo->tipo, tipo) == 0);
    assert(simbolo->linha == linha);
}

static void reiniciar(void) {
    tabela_liberar();
    teste_reiniciar();
    erros = 0;
    ultima_linha = 0;
    mensagens[0] = '\0';
}

static void testar_ast(void) {
    reiniciar();
    const TipoNo operadores[] = {AST_ATRIBUICAO, AST_ATRIB_SOMA, AST_ATRIB_SUB,
                                 AST_ATRIB_MULT, AST_ATRIB_DIV};
    const char *nomes[] = {"ATRIBUICAO", "ATRIB_SOMA", "ATRIB_SUB",
                          "ATRIB_MULT", "ATRIB_DIV"};
    for (size_t i = 0; i < sizeof operadores / sizeof operadores[0]; i++) {
        NoAST *alvo = id("a", 2), *valor = numero(3, 3);
        NoAST *no = ast_p4_atribuicao(operadores[i], alvo, valor, 1);
        assert(no->tipo == operadores[i] && no->linha == 1);
        assert(no->a == alvo && no->b == valor && !no->c && !no->prox);
        assert(strcmp(ast_p4_nome_tipo(no->tipo), nomes[i]) == 0);
    }
    NoAST *primeiro = numero(1, 4), *segundo = texto("dois", 5);
    primeiro->prox = segundo;
    NoAST *lista = ast_p4_lista(primeiro, 4);
    assert(lista->tipo == AST_LISTA && lista->linha == 4 && lista->a == primeiro);
    assert(primeiro->prox == segundo && !lista->b && !lista->c);
    assert(strcmp(ast_p4_nome_tipo(AST_LISTA), "LISTA") == 0);
    NoAST *indice = numero(0, 6);
    NoAST *indexacao = ast_p4_indexacao(lista, indice, 6);
    assert(indexacao->tipo == AST_INDEXACAO && indexacao->linha == 6);
    assert(indexacao->a == lista && indexacao->b == indice && !indexacao->c);
    assert(strcmp(ast_p4_nome_tipo(AST_INDEXACAO), "INDEXACAO") == 0);
    assert(!ast_p4_nome_tipo(AST_NUMERO) && !ast_p4_imprimir(NULL, 0));
    assert(!ast_p4_imprimir(indice, 0));

    /* O impressor P4 delega apenas a/b/c uma vez e deixa prox para o dono
     * da sequencia. O spy nao conhece o conteudo de nenhuma outra fatia. */
    assert(teste_impressao_total() == 0);
    indexacao->prox = teste_no(AST_NONE, 7);
    assert(ast_p4_imprimir(indexacao, 2) == 1);
    assert(teste_impressao_total() == 3);
    assert(teste_impressao_chamada(0)->no == lista);
    assert(teste_impressao_chamada(1)->no == indice);
    assert(teste_impressao_chamada(2)->no == NULL);
    for (size_t i = 0; i < 3; i++) {
        assert(teste_impressao_chamada(i)->nivel == 3);
        assert(teste_impressao_chamada(i)->no != indexacao->prox);
    }
}

static void testar_variaveis(void) {
    reiniciar();
    verificar(atribuir(AST_ATRIBUICAO, "y", id("y", 23), 23), P4_TIPO_ERRO, 1);
    assert(!tabela_buscar("y") && ultima_linha == 23 && strstr(mensagens, "'y'"));

    verificar(atribuir(AST_ATRIBUICAO, "x", numero(1, 3), 3), "none", 0);
    Simbolo *original = tabela_buscar("x");
    assert(original->categoria == CAT_VARIAVEL);
    tipo_simbolo("x", "int", 3);
    verificar(atribuir(AST_ATRIBUICAO, "x", texto("texto", 4), 4), "none", 0);
    assert(tabela_buscar("x") == original);
    tipo_simbolo("x", "str", 3);
    verificar(atribuir(AST_ATRIBUICAO, "x", id("x", 5), 5), "none", 0);
    tipo_simbolo("x", "str", 3); /* Tipo pode apontar para o proprio simbolo. */
    verificar(atribuir(AST_ATRIBUICAO, "x", id("ausente", 6), 6), P4_TIPO_ERRO, 1);
    assert(tabela_buscar("x") == original);
    tipo_simbolo("x", "str", 3);

    assert(tabela_inserir("f", CAT_FUNCAO, "funcao", 7) == 0);
    verificar(atribuir(AST_ATRIBUICAO, "f", numero(1, 8), 8), P4_TIPO_ERRO, 1);
    assert(tabela_buscar("f")->categoria == CAT_FUNCAO);
    tipo_simbolo("f", "funcao", 7);
    verificar(atribuir(AST_ATRIBUICAO, "print", numero(1, 9), 9), P4_TIPO_ERRO, 1);
    assert(tabela_buscar("print")->categoria == CAT_BUILTIN);
    verificar(atribuir(AST_ATRIB_SOMA, "nao_declarada", numero(1, 10), 10),
              P4_TIPO_ERRO, 1);
    assert(!tabela_buscar("nao_declarada"));

    assert(tabela_inserir("incerto", CAT_VARIAVEL, "?", 11) == 0);
    verificar(atribuir(AST_ATRIBUICAO, "copia", id("incerto", 12), 12), "none", 0);
    tipo_simbolo("copia", "?", 12);
    assert(tabela_inserir("ja_falhou", CAT_VARIAVEL, P4_TIPO_ERRO, 13) == 0);
    verificar(atribuir(AST_ATRIBUICAO, "nao_inserir", id("ja_falhou", 14), 14),
              P4_TIPO_ERRO, 0); /* Erro do filho ja foi diagnosticado pelo dono. */
    assert(!tabela_buscar("nao_inserir"));
}

static void testar_escopos(void) {
    reiniciar();
    verificar(atribuir(AST_ATRIBUICAO, "global", numero(1, 1), 1), "none", 0);
    Simbolo *global = tabela_buscar("global");
    tabela_entrar_escopo("escopo_fixture");
    verificar(id("global", 2), "int", 0);
    verificar(atribuir(AST_ATRIB_SOMA, "global", numero(2, 3), 3), P4_TIPO_ERRO, 1);
    assert(!tabela_buscar_local("global") && tabela_buscar("global") == global);
    tipo_simbolo("global", "int", 1);
    verificar(atribuir(AST_ATRIBUICAO, "global", texto("local", 4), 4), "none", 0);
    assert(tabela_buscar_local("global") && tabela_buscar("global") != global);
    tipo_simbolo("global", "str", 4);
    verificar(atribuir(AST_ATRIB_SOMA, "global", texto("mais", 5), 5), "none", 0);
    tipo_simbolo("global", "str", 4);
    assert(tabela_inserir("p", CAT_PARAMETRO, "int", 6) == 0);
    verificar(atribuir(AST_ATRIB_SOMA, "p", numero(1, 7), 7), "none", 0);
    assert(tabela_buscar("p")->categoria == CAT_PARAMETRO);
    tipo_simbolo("p", "int", 6);
    tabela_sair_escopo();
    assert(tabela_buscar("global") == global && !tabela_buscar("p"));
    tipo_simbolo("global", "int", 1);
}

static void testar_listas_e_indices(void) {
    reiniciar();
    NoAST *elementos = numero(1, 1);
    elementos->prox = texto("dois", 2);
    elementos->prox->prox = teste_no(AST_BOOL, 3);
    elementos->prox->prox->prox = teste_no(AST_NONE, 4);
    elementos->prox->prox->prox->prox = ast_p4_lista(NULL, 5);
    verificar(atribuir(AST_ATRIBUICAO, "l", ast_p4_lista(elementos, 1), 1), "none", 0);
    tipo_simbolo("l", "lista", 1);
    verificar(ast_p4_indexacao(id("l", 6), numero(0, 6), 6), "?", 0);
    verificar(ast_p4_atribuicao(AST_ATRIBUICAO,
                  ast_p4_indexacao(id("l", 7), numero(0, 7), 7), texto("novo", 7), 7),
              "none", 0);
    tipo_simbolo("l", "lista", 1);

    verificar(ast_p4_indexacao(id("l", 8),
                  teste_no(AST_BOOL, 9), 8), P4_TIPO_ERRO, 1);
    assert(ultima_linha == 9 && strstr(mensagens, "bool"));
    verificar(ast_p4_indexacao(numero(5, 10), numero(0, 10), 10), P4_TIPO_ERRO, 1);
    verificar(ast_p4_atribuicao(AST_ATRIBUICAO, numero(3, 11), numero(2, 11), 11),
              P4_TIPO_ERRO, 1);

    /* Uma falha nao impede visitar os demais elementos da mesma lista. */
    elementos = id("primeira_ausente", 12);
    elementos->prox = id("segunda_ausente", 13);
    verificar(atribuir(AST_ATRIBUICAO, "lista_invalida", ast_p4_lista(elementos, 12), 12),
              P4_TIPO_ERRO, 2);
    assert(strstr(mensagens, "primeira_ausente") && strstr(mensagens, "segunda_ausente"));
    assert(!tabela_buscar("lista_invalida") && ultima_linha == 13);
    assert(tabela_inserir("lista_incerta", CAT_VARIAVEL, "?", 14) == 0);
    verificar(ast_p4_indexacao(id("lista_incerta", 15), numero(0, 15), 15), "?", 0);
}

static void testar_compostos(void) {
    reiniciar();
    verificar(atribuir(AST_ATRIBUICAO, "n", numero(8, 1), 1), "none", 0);
    verificar(atribuir(AST_ATRIB_SOMA, "n", numero(2, 2), 2), "none", 0);
    verificar(atribuir(AST_ATRIB_SUB, "n", numero(2, 3), 3), "none", 0);
    verificar(atribuir(AST_ATRIB_MULT, "n", numero(2, 4), 4), "none", 0);
    tipo_simbolo("n", "int", 1);
    verificar(atribuir(AST_ATRIB_DIV, "n", numero(0, 5), 5), P4_TIPO_ERRO, 1);
    assert(ultima_linha == 5 && strstr(mensagens, "zero"));
    tipo_simbolo("n", "int", 1);
    verificar(atribuir(AST_ATRIB_DIV, "n", numero(2, 6), 6), "none", 0);
    tipo_simbolo("n", "?", 1);

    verificar(atribuir(AST_ATRIBUICAO, "s", texto("a", 7), 7), "none", 0);
    verificar(atribuir(AST_ATRIB_SOMA, "s", texto("b", 8), 8), "none", 0);
    verificar(atribuir(AST_ATRIB_MULT, "s", numero(3, 9), 9), "none", 0);
    tipo_simbolo("s", "str", 7);
    verificar(atribuir(AST_ATRIB_SUB, "s", numero(1, 10), 10), P4_TIPO_ERRO, 1);
    verificar(atribuir(AST_ATRIB_SOMA, "s", numero(1, 11), 11), P4_TIPO_ERRO, 1);
    tipo_simbolo("s", "str", 7);

    verificar(atribuir(AST_ATRIBUICAO, "l", ast_p4_lista(NULL, 12), 12), "none", 0);
    verificar(atribuir(AST_ATRIB_SOMA, "l", ast_p4_lista(numero(1, 13), 13), 13), "none", 0);
    verificar(atribuir(AST_ATRIB_MULT, "l", numero(2, 14), 14), "none", 0);
    verificar(atribuir(AST_ATRIB_DIV, "l", numero(2, 15), 15), P4_TIPO_ERRO, 1);
    tipo_simbolo("l", "lista", 12);
    verificar(ast_p4_atribuicao(AST_ATRIB_SOMA,
                  ast_p4_indexacao(id("l", 16), numero(0, 16), 16), numero(1, 16), 16),
              "none", 0);
    tipo_simbolo("l", "lista", 12);
}

/* Cinco cenarios novos; os testes existentes ficam preservados. */
static void testar_regressoes_p4(void) {
    reiniciar();
    verificar(atribuir(AST_ATRIBUICAO, "n", numero(8, 1), 1), "none", 0);

    /* 1. Literal real nao nulo com valor == 0 nao e divisao por zero. */
    NoAST *divisor = teste_numero_real(0.5, 2);
    assert(divisor->valor == 0);
    verificar(atribuir(AST_ATRIB_DIV, "n", divisor, 2), "none", 0);
    tipo_simbolo("n", "?", 1);

    /* 2. Zero real deve gerar erro e conservar o tipo anterior. */
    verificar(atribuir(AST_ATRIBUICAO, "n", numero(8, 3), 3), "none", 0);
    verificar(atribuir(AST_ATRIB_DIV, "n", teste_numero_real(0.0, 4), 4),
              P4_TIPO_ERRO, 1);
    tipo_simbolo("n", "int", 1);
    assert(ultima_linha == 4 && strstr(mensagens, "zero"));

    /* 3. Indice fracionario impede a insercao da variavel de destino. */
    verificar(atribuir(AST_ATRIBUICAO, "l", ast_p4_lista(NULL, 5), 5), "none", 0);
    verificar(atribuir(AST_ATRIBUICAO, "destino",
                  ast_p4_indexacao(id("l", 6), teste_numero_real(1.5, 6), 6), 6),
              P4_TIPO_ERRO, 1);
    assert(!tabela_buscar("destino") && ultima_linha == 6);
    tipo_simbolo("l", "lista", 5);

    /* 4. Base e indice invalidos produzem os dois erros, sem atualizar n. */
    verificar(atribuir(AST_ATRIBUICAO, "n",
                  ast_p4_indexacao(numero(5, 7), texto("indice", 8), 7), 7),
              P4_TIPO_ERRO, 2);
    tipo_simbolo("n", "int", 1);
    assert(ultima_linha == 8);

    /* 5. Alvo invalido nao impede diagnosticar um nome ausente no RHS. */
    verificar(ast_p4_atribuicao(AST_ATRIBUICAO, numero(3, 9),
                  id("ausente_rhs", 10), 9), P4_TIPO_ERRO, 2);
    assert(!tabela_buscar("ausente_rhs") && ultima_linha == 10);
}

int main(void) {
    testar_ast();
    testar_variaveis();
    testar_escopos();
    testar_listas_e_indices();
    testar_compostos();
    testar_regressoes_p4();
    tabela_liberar();
    puts("[OK] semantica e AST P4: alvos, tipos, listas, indices e escopos");
    return 0;
}
