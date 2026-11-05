/* ===== [P4] Testes da tabela de simbolos ===== */
#include "Semantica/Tabela/tabela.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* [P4] Contrato de escopos e propriedade da memoria, sem parser ou outras fatias. */
int main(void) {
    tabela_liberar();
    tabela_liberar();
    Simbolo *builtin = tabela_buscar("print");
    assert(builtin && builtin->categoria == CAT_BUILTIN);
    assert(builtin->escopo == 0 && builtin->linha == 0);
    assert(builtin->num_params == -1 && strcmp(builtin->tipo, "funcao") == 0);
    assert(tabela_buscar("range") && !tabela_buscar("ausente"));
    assert(tabela_inserir("print", CAT_VARIAVEL, "int", 1) == -1);

    /* Colisao conhecida na funcao da Semana 7: 16*'a'+'a' == 16*'b'+'Q'.
     * A busca precisa comparar o nome e atravessar a cadeia inteira. */
    assert(tabela_inserir("aa", CAT_VARIAVEL, "int", 1) == 0);
    assert(tabela_inserir("bQ", CAT_VARIAVEL, "str", 2) == 0);
    Simbolo *aa_global = tabela_buscar("aa");
    Simbolo *bq_global = tabela_buscar("bQ");
    assert(aa_global && bq_global && aa_global != bq_global);
    assert(strcmp(aa_global->tipo, "int") == 0);
    assert(strcmp(bq_global->tipo, "str") == 0);
    assert(aa_global->prox == bq_global); /* Impressao segue ordem de insercao. */
    assert(tabela_inserir("aa", CAT_VARIAVEL, "none", 3) == -1);
    assert(tabela_buscar("aa") == aa_global && aa_global->linha == 1);

    char nome[] = "x";
    char tipo[] = "int";
    assert(tabela_inserir(nome, CAT_VARIAVEL, tipo, 2) == 0);
    nome[0] = 'z';
    tipo[0] = 'X';
    Simbolo *global = tabela_buscar("x");
    assert(global && strcmp(global->nome, "x") == 0 && strcmp(global->tipo, "int") == 0);
    assert(global->linha == 2 && global->escopo == 0);
    assert(tabela_inserir("x", CAT_VARIAVEL, "str", 3) == -1);
    assert(tabela_buscar("x") == global && global->linha == 2);

    tabela_entrar_escopo("f");
    assert(!tabela_buscar_local("aa") && tabela_buscar("aa") == aa_global);
    assert(tabela_inserir("aa", CAT_PARAMETRO, "bool", 4) == 0);
    Simbolo *aa_local = tabela_buscar("aa");
    assert(aa_local != aa_global && strcmp(aa_local->tipo, "bool") == 0);
    assert(tabela_buscar("bQ") == bq_global); /* Bucket global continua visivel. */
    assert(tabela_inserir("bQ", CAT_VARIAVEL, "lista", 5) == 0);
    assert(tabela_buscar("bQ") != bq_global);
    assert(tabela_buscar("aa") == aa_local);
    assert(tabela_inserir("aa", CAT_VARIAVEL, "str", 6) == -1);
    assert(!tabela_buscar_local("x") && tabela_buscar("x") == global);
    assert(tabela_inserir("x", CAT_PARAMETRO, "?", 4) == 0);
    Simbolo *local = tabela_buscar("x");
    int primeiro_id = local->escopo;
    assert(local != global && primeiro_id > 0);
    assert(tabela_inserir("x", CAT_PARAMETRO, "?", 5) == -1);
    assert(tabela_inserir("so_local", CAT_VARIAVEL, "lista", 6) == 0);

    tabela_entrar_escopo("interna");
    assert(!tabela_buscar_local("x") && tabela_buscar("x") == local);
    assert(tabela_inserir("x", CAT_VARIAVEL, "bool", 7) == 0);
    assert(tabela_buscar("x")->escopo > primeiro_id);
    assert(tabela_buscar("print") == builtin);
    tabela_sair_escopo();
    assert(tabela_buscar("x") == local);
    tabela_sair_escopo();
    assert(tabela_buscar("x") == global && !tabela_buscar("so_local"));
    assert(tabela_buscar("aa") == aa_global && tabela_buscar("bQ") == bq_global);
    tabela_sair_escopo();
    assert(tabela_buscar("x") == global);

    tabela_entrar_escopo("outra");
    assert(tabela_inserir("y", CAT_VARIAVEL, "none", 8) == 0);
    assert(tabela_buscar("y")->escopo > primeiro_id + 1);
    tabela_sair_escopo();
    assert(!tabela_buscar("y"));

    assert(tabela_inserir("longo", CAT_VARIAVEL, "12345678901234567890", 9) == 0);
    assert(strlen(tabela_buscar("longo")->tipo) == 15);
    assert(tabela_inserir("desconhecido", (Categoria)999, NULL, 10) == 0);
    assert(strcmp(tabela_buscar("desconhecido")->tipo, "?") == 0);
    assert(tabela_inserir(NULL, CAT_VARIAVEL, "int", 1) == -1);
    assert(!tabela_buscar(NULL) && !tabela_buscar_local(NULL));

    tabela_entrar_escopo("liberar_com_pilha_ativa");
    assert(tabela_inserir("temporario", CAT_VARIAVEL, "int", 11) == 0);
    tabela_liberar();
    assert(!tabela_buscar("x") && tabela_buscar("range"));
    assert(!tabela_buscar("aa") && !tabela_buscar("bQ"));
    tabela_entrar_escopo("reiniciada");
    assert(tabela_inserir("novo", CAT_VARIAVEL, "int", 12) == 0);
    assert(tabela_buscar("novo")->escopo == 1);
    tabela_liberar();

    /* Exerce a inicializacao por impressao e a ordem dos builtins. */
    tabela_imprimir();
    tabela_liberar();
    puts("[OK] tabela P4: escopos, busca, duplicados, builtins e reuso");
    return 0;
}
