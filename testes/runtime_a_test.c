/* Contrato de execucao da Pessoa A. Liga as implementacoes reais;
 * nao substitui valores, expressoes, lacos nem funcoes da Pessoa B. */
#include "Interpretador/ambiente.h"
#include "Interpretador/interpretador.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void verdade(Valor valor, int esperado) {
    assert(valor_verdadeiro(valor) == esperado);
    valor_liberar(valor);
}

static void testar_verdade_e_comparacoes(void) {
    verdade(valor_none(), 0);
    verdade(valor_bool(0), 0);
    verdade(valor_int(0), 0);
    verdade(valor_real(0), 0);
    verdade(valor_str(""), 0);
    verdade(valor_lista(lista_criar()), 0);
    verdade(valor_bool(1), 1);
    verdade(valor_int(-1), 1);
    verdade(valor_real(0.5), 1);
    verdade(valor_str("x"), 1);
    NoAST *funcao = ast_criar_texto(AST_FUNCAO, "f", 1);
    verdade(valor_funcao(funcao), 1);
    Valor a = valor_funcao(funcao), b = valor_funcao(funcao);
    assert(comparar_valores(AST_IGUAL, a, b, 1).u.booleano);
    ast_liberar(funcao);
    assert(comparar_valores(AST_IGUAL, valor_int(1), valor_real(1), 1).u.booleano);
    assert(comparar_valores(AST_IGUAL, valor_int(1), valor_bool(1), 1).u.booleano);
    assert(comparar_valores(AST_DIFERENTE, valor_int(1), valor_none(), 1).u.booleano);
    assert(comparar_valores(AST_IGUAL, valor_none(), valor_none(), 1).u.booleano);
    assert(!comparar_valores(AST_MENOR_IGUAL, valor_real(NAN), valor_int(1), 1).u.booleano);
    a = valor_str("abc");
    b = valor_str("abd");
    assert(comparar_valores(AST_MENOR, a, b, 1).u.booleano);
    assert(comparar_valores(AST_MAIOR_IGUAL, b, a, 1).u.booleano);
    valor_liberar(a);
    valor_liberar(b);

    Lista *l1 = lista_criar(), *l2 = lista_criar();
    lista_adicionar(l1, valor_str("item"));
    lista_adicionar(l2, valor_str("item"));
    a = valor_lista(l1);
    b = valor_lista(l2);
    assert(comparar_valores(AST_IGUAL, a, b, 1).u.booleano);
    lista_adicionar(l2, valor_int(2));
    assert(comparar_valores(AST_DIFERENTE, a, b, 1).u.booleano);
    valor_liberar(a);
    valor_liberar(b);
}

static void testar_ambiente_e_memoria(void) {
    ambiente_iniciar();
    assert(ambiente_profundidade() == 0);
    char nome[] = "global";
    ambiente_definir(nome, valor_str("original"));
    nome[0] = 'X';
    assert(strcmp(ambiente_buscar("global")->u.texto, "original") == 0);
    ambiente_entrar();
    ambiente_definir("chamador", valor_int(42));
    ambiente_definir("global", valor_str("local"));
    ambiente_entrar();
    assert(ambiente_profundidade() == 2);
    assert(!ambiente_buscar("chamador"));
    assert(strcmp(ambiente_buscar("global")->u.texto, "original") == 0);
    ambiente_sair();
    assert(ambiente_buscar("chamador")->u.inteiro == 42);
    assert(strcmp(ambiente_buscar("global")->u.texto, "local") == 0);
    ambiente_sair();
    ambiente_sair(); /* Nao remove o global. */
    assert(ambiente_profundidade() == 0);

    Lista *lista = lista_criar();
    assert(lista->referencias == 1 && lista->tamanho == 0);
    for (int i = 0; i < 1000; i++) lista_adicionar(lista, valor_int(i));
    assert(lista->tamanho == 1000 && lista->capacidade >= 1000);
    Valor original = valor_lista(lista);
    Valor copia = valor_copiar(original);
    assert(copia.u.lista == lista && lista->referencias == 2);
    ambiente_definir("a", original);
    ambiente_definir("b", copia);
    ambiente_definir("a", valor_none());
    assert(lista->referencias == 1 && lista->itens[999].u.inteiro == 999);

    Valor texto = valor_str("proprio"), duplicado = valor_copiar(texto);
    assert(texto.u.texto != duplicado.u.texto);
    texto.u.texto[0] = 'X';
    assert(strcmp(duplicado.u.texto, "proprio") == 0);
    valor_liberar(texto);
    valor_liberar(duplicado);
    ambiente_liberar();
    ambiente_liberar();
    assert(!ambiente_buscar("b") && ambiente_profundidade() == 0);
}

static NoAST *booleano(int valor) {
    NoAST *no = ast_criar(AST_BOOL, NULL, NULL, NULL, 1);
    no->valor = valor;
    return no;
}

static void testar_curto_circuito(void) {
    /* O identificador ausente provocaria erro se o RHS fosse avaliado. */
    NoAST *no = ast_criar(AST_E, booleano(0),
                         ast_criar_texto(AST_IDENTIFICADOR, "ausente", 1), NULL, 1);
    Valor v = interpretar_no(no);
    assert(v.tipo == VAL_BOOL && !v.u.booleano);
    valor_liberar(v);
    ast_liberar(no);
    no = ast_criar(AST_OU, ast_criar_texto(AST_STRING, "operando", 1),
                   ast_criar_texto(AST_IDENTIFICADOR, "ausente", 1), NULL, 1);
    v = interpretar_no(no);
    assert(v.tipo == VAL_STR && strcmp(v.u.texto, "operando") == 0);
    valor_liberar(v);
    ast_liberar(no);
    no = ast_criar(AST_OU, ast_criar_numero(0, 1), ast_criar_numero(5, 1), NULL, 1);
    v = interpretar_no(no);
    assert(v.tipo == VAL_INT && v.u.inteiro == 5);
    valor_liberar(v);
    ast_liberar(no);
}

static void testar_if_preserva_sinal(void) {
    const TipoNo comandos[] = {AST_BREAK, AST_CONTINUE, AST_RETURN};
    const Sinal sinais[] = {SINAL_BREAK, SINAL_CONTINUE, SINAL_RETURN};
    for (int i = 0; i < 3; i++) {
        NoAST *comando = ast_criar(comandos[i], NULL, NULL, NULL, 1);
        NoAST *bloco = ast_criar(AST_BLOCO, comando, NULL, NULL, 1);
        NoAST *no = ast_criar(AST_IF, booleano(1), bloco, NULL, 1);
        sinal_atual = SINAL_NORMAL;
        valor_liberar(interpretar_no(no));
        assert(sinal_atual == sinais[i]);
        sinal_atual = SINAL_NORMAL;
        valor_liberar(valor_retorno);
        valor_retorno = valor_none();
        ast_liberar(no);
    }
}

int main(void) {
    testar_verdade_e_comparacoes();
    testar_ambiente_e_memoria();
    testar_curto_circuito();
    testar_if_preserva_sinal();
    puts("[OK] Pessoa A: verdade, comparacoes, escopos, referencias e sinais");
    return 0;
}
