/*
 * FGA0003 - Compiladores 1
 * Curso de Engenharia de Software
 * Universidade de Brasília (UnB)
 * Equipe 13 - MiniPy
 *
 * Arquivo: Interpretador/exec_atribuicao.c
 * Descrição: execução de atribuições simples e compostas, leitura
 *            de variáveis, criação de listas e indexação de listas e textos.
 */

#include "Interpretador/interpretador.h"
#include "Interpretador/ambiente.h"

#include <string.h>

/* [P4] Posicao real de um indice em uma colecao de "tamanho" itens.
 * Comum a listas e textos: negativo conta do fim, fora do tamanho e erro.
 * "colecao" so muda a mensagem ("lista" ou "texto"). */
static int posicao_indice(Valor indice, long long tamanho, const char *colecao, int linha) {
    if (indice.tipo != VAL_INT)
        erro_execucao(linha, "indice de %s deve ser int, recebido '%s'",
                      colecao, valor_nome_tipo(indice));
    long long original = indice.u.inteiro;
    long long posicao = original;
    if (posicao < 0) posicao += tamanho;
    if (posicao < 0 || posicao >= tamanho)
        erro_execucao(linha, "indice %lld fora do intervalo (%s de tamanho %lld)",
                      original, colecao, tamanho);
    return (int)posicao;
}

static int indice_lista(Valor base, Valor indice, int linha) {
    if (base.tipo != VAL_LISTA)
        erro_execucao(linha, "'%s' nao pode ser indexado", valor_nome_tipo(base));
    return posicao_indice(indice, base.u.lista->tamanho, "lista", linha);
}

/* [P4] Sprint 5: s[i] devolve um texto NOVO de 1 caractere. Como na
 * iteracao do for, o texto e percorrido por bytes (sem decodificar UTF-8). */
static Valor caractere_texto(Valor base, Valor indice, int linha) {
    const char *texto = base.u.texto;
    int posicao = posicao_indice(indice, (long long)strlen(texto), "texto", linha);
    char caractere[2] = { texto[posicao], '\0' };
    return valor_str(caractere);
}

static Valor ler_variavel(NoAST *no) {
    Valor *v = ambiente_buscar(no->texto);
    if (!v) erro_execucao(no->linha, "variavel '%s' nao definida", no->texto);
    return valor_copiar(*v);
}

static TipoNo operador_composto(TipoNo tipo) {
    switch (tipo) {
        case AST_ATRIB_SOMA: return AST_SOMA;
        case AST_ATRIB_SUB: return AST_SUBTRACAO;
        case AST_ATRIB_MULT: return AST_MULTIPLICACAO;
        default: return AST_DIVISAO;
    }
}

Valor executar_atribuicao(NoAST *no) {
    if (no->tipo == AST_IDENTIFICADOR) return ler_variavel(no);
    if (no->tipo == AST_LISTA) {
        Lista *lista = lista_criar();
        for (NoAST *item = no->a; item; item = item->prox)
            lista_adicionar(lista, interpretar_no(item));
        return valor_lista(lista);
    }
    if (no->tipo == AST_INDEXACAO) {
        Valor base = interpretar_no(no->a);
        Valor indice = interpretar_no(no->b);
        Valor resultado;
        if (base.tipo == VAL_STR) {
            resultado = caractere_texto(base, indice, no->linha);
        } else {
            int posicao = indice_lista(base, indice, no->linha);
            resultado = valor_copiar(base.u.lista->itens[posicao]);
        }
        valor_liberar(indice);
        valor_liberar(base);
        return resultado;
    }

    NoAST *alvo = no->a;
    int simples = no->tipo == AST_ATRIBUICAO;
    /* A atribuicao simples avalia o RHS antes do alvo, como no Python.
     * A composta fixa o alvo/valor atual antes de avaliar o RHS. Copias
     * mantem a lista viva mesmo se uma chamada no indice/RHS redefinir o nome. */
    Valor novo = valor_none();
    if (simples) novo = interpretar_no(no->b);
    Valor base = valor_none();
    Valor anterior = valor_none();
    int posicao = 0;
    if (alvo->tipo == AST_INDEXACAO) {
        base = ler_variavel(alvo->a);
        Valor indice = interpretar_no(alvo->b);
        /* [P4] Texto e imutavel, como no Python: so a leitura s[i] e permitida. */
        if (base.tipo == VAL_STR)
            erro_execucao(alvo->linha, "'str' nao permite atribuicao por indice");
        posicao = indice_lista(base, indice, alvo->linha);
        valor_liberar(indice);
        if (!simples) anterior = valor_copiar(base.u.lista->itens[posicao]);
    } else if (!simples) {
        anterior = ler_variavel(alvo);
    }
    if (!simples) {
        novo = interpretar_no(no->b);
        Valor resultado = operar_aritmetica(operador_composto(no->tipo),
                                            anterior, novo, no->linha);
        valor_liberar(anterior);
        valor_liberar(novo);
        novo = resultado;
    }
    if (alvo->tipo == AST_INDEXACAO) {
        valor_liberar(base.u.lista->itens[posicao]);
        base.u.lista->itens[posicao] = novo;
        valor_liberar(base);
    } else {
        ambiente_definir(alvo->texto, novo);
    }
    return valor_none();
}
