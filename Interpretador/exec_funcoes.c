#include "Interpretador/interpretador.h"
#include "Interpretador/ambiente.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t contar_argumentos(NoAST *primeiro) {
    size_t total = 0;
    for (NoAST *item = primeiro; item; item = item->prox) total++;
    return total;
}

static Valor *avaliar_argumentos(NoAST *chamada, size_t quantidade) {
    if (quantidade == 0) return NULL;
    if (quantidade > SIZE_MAX / sizeof(Valor)) {
        erro_execucao(chamada->linha, "memoria insuficiente");
        return NULL;
    }
    Valor *valores = malloc(quantidade * sizeof *valores);
    if (!valores) {
        erro_execucao(chamada->linha, "memoria insuficiente");
        return NULL;
    }
    size_t i = 0;
    for (NoAST *item = chamada->a; item; item = item->prox)
        valores[i++] = interpretar_no(item);
    return valores;
}

static void liberar_argumentos(Valor *valores, size_t quantidade) {
    for (size_t i = 0; i < quantidade; i++) valor_liberar(valores[i]);
    free(valores);
}

static Valor chamar_print(Valor *valores, size_t quantidade) {
    for (size_t i = 0; i < quantidade; i++) {
        if (i) putchar(' ');
        valor_imprimir(valores[i], 0);
    }
    putchar('\n');
    liberar_argumentos(valores, quantidade);
    return valor_none();
}

static Valor chamar_range(NoAST *chamada, Valor *valores, size_t quantidade) {
    if (quantidade < 1 || quantidade > 3)
        erro_execucao(chamada->linha, "funcao 'range' espera de 1 a 3 argumentos, recebeu %zu",
                      quantidade);
    for (size_t i = 0; i < quantidade; i++)
        if (valores[i].tipo != VAL_INT)
            erro_execucao(chamada->linha, "argumentos de range devem ser int");

    long long inicio = quantidade == 1 ? 0 : valores[0].u.inteiro;
    long long fim = valores[quantidade == 1 ? 0 : 1].u.inteiro;
    long long passo = quantidade == 3 ? valores[2].u.inteiro : 1;
    liberar_argumentos(valores, quantidade);
    if (passo == 0)
        erro_execucao(chamada->linha, "passo de range nao pode ser zero");

    Lista *lista = lista_criar();
    for (long long atual = inicio; passo > 0 ? atual < fim : atual > fim;) {
        lista_adicionar(lista, valor_int(atual));
        /* Se a proxima soma sair de long long, ela tambem ultrapassaria fim.
         * Testar antes da soma evita overflow inclusive com LLONG_MIN. */
        if ((passo > 0 && atual > LLONG_MAX - passo) ||
            (passo < 0 && atual < LLONG_MIN - passo)) break;
        atual += passo;
    }
    return valor_lista(lista);
}

static Valor chamar_usuario(NoAST *chamada, Valor *valores, size_t quantidade) {
    /* Os argumentos ja foram avaliados no quadro de quem chama. Buscar o
     * destino depois disso preserva os efeitos dessas avaliacoes. */
    Valor *destino = ambiente_buscar(chamada->texto);
    if (!destino)
        erro_execucao(chamada->linha, "funcao '%s' nao definida", chamada->texto);
    if (destino->tipo != VAL_FUNCAO)
        erro_execucao(chamada->linha, "'%s' nao e uma funcao (tipo '%s')",
                      chamada->texto, valor_nome_tipo(*destino));
    NoAST *def = destino->u.funcao;
    size_t parametros = contar_argumentos(def->a);
    /* Chamadas por parametro podem ter aridade desconhecida na semantica. */
    if (parametros != quantidade)
        erro_execucao(chamada->linha, "funcao '%s' espera %zu argumento(s), recebeu %zu",
                      chamada->texto, parametros, quantidade);
    if (ambiente_profundidade() >= 1000)
        erro_execucao(chamada->linha, "limite de recursao excedido (1000 chamadas)");

    ambiente_entrar();
    size_t i = 0;
    for (NoAST *parametro = def->a; parametro; parametro = parametro->prox)
        ambiente_definir(parametro->texto, valores[i++]);
    free(valores); /* O quadro assumiu a posse de cada Valor. */
    valor_liberar(interpretar_no(def->b));

    Valor resultado = valor_none();
    if (sinal_atual == SINAL_RETURN) {
        resultado = valor_retorno;
        valor_retorno = valor_none();
        sinal_atual = SINAL_NORMAL;
    }
    ambiente_sair();
    return resultado;
}

Valor executar_funcao(NoAST *no) {
    switch (no->tipo) {
        case AST_FUNCAO:
            ambiente_definir(no->texto, valor_funcao(no));
            return valor_none();
        case AST_PARAMETRO:
            return valor_none();
        case AST_RETURN:
            valor_retorno = no->a ? interpretar_no(no->a) : valor_none();
            sinal_atual = SINAL_RETURN;
            return valor_none();
        case AST_CHAMADA: {
            size_t quantidade = contar_argumentos(no->a);
            Valor *valores = avaliar_argumentos(no, quantidade);
            if (strcmp(no->texto, "print") == 0)
                return chamar_print(valores, quantidade);
            if (strcmp(no->texto, "range") == 0)
                return chamar_range(no, valores, quantidade);
            return chamar_usuario(no, valores, quantidade);
        }
        default:
            erro_execucao(no->linha, "no de funcao desconhecido");
            return valor_none();
    }
}
