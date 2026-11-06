#include "Interpretador/valor.h"
#include "Interpretador/interpretador.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

Lista *lista_criar(void) {
    Lista *l = calloc(1, sizeof *l);
    if (!l) erro_execucao(1, "memoria insuficiente");
    l->referencias = 1;
    return l;
}

void lista_adicionar(Lista *l, Valor v) {
    if (l->tamanho == l->capacidade) {
        if (l->capacidade > INT_MAX / 2)
            erro_execucao(1, "lista excedeu o tamanho suportado");
        int capacidade = l->capacidade ? l->capacidade * 2 : 4;
        if (SIZE_MAX / (size_t)capacidade < sizeof *l->itens)
            erro_execucao(1, "lista excedeu o tamanho suportado");
        Valor *itens = realloc(l->itens, (size_t)capacidade * sizeof *itens);
        if (!itens) erro_execucao(1, "memoria insuficiente");
        l->itens = itens;
        l->capacidade = capacidade;
    }
    l->itens[l->tamanho++] = v;
}

Valor valor_lista(Lista *l) {
    Valor v = valor_none();
    v.tipo = VAL_LISTA;
    v.u.lista = l;
    return v;
}

void lista_liberar(Lista *l) {
    if (!l) return;
    for (int i = 0; i < l->tamanho; i++) valor_liberar(l->itens[i]);
    free(l->itens);
    free(l);
}
