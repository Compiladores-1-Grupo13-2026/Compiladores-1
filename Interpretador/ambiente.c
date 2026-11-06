#include "Interpretador/ambiente.h"
#include "Interpretador/interpretador.h"

#include <stdlib.h>
#include <string.h>

typedef struct Entrada {
    char *nome;
    Valor valor;
    struct Entrada *prox;
} Entrada;

typedef struct Quadro {
    Entrada *entradas;
    struct Quadro *anterior;
} Quadro;

static Quadro *atual;
static Quadro *global;
static int profundidade;

static void *alocar(size_t tamanho) {
    void *memoria = calloc(1, tamanho);
    if (!memoria) erro_execucao(1, "memoria insuficiente");
    return memoria;
}

static Valor *buscar_quadro(Quadro *quadro, const char *nome) {
    for (Entrada *e = quadro ? quadro->entradas : NULL; e; e = e->prox)
        if (strcmp(e->nome, nome) == 0) return &e->valor;
    return NULL;
}

static void liberar_quadro(Quadro *quadro) {
    Entrada *e = quadro->entradas;
    while (e) {
        Entrada *proxima = e->prox;
        valor_liberar(e->valor);
        free(e->nome);
        free(e);
        e = proxima;
    }
    free(quadro);
}

void ambiente_iniciar(void) {
    ambiente_liberar();
    atual = global = alocar(sizeof *global);
}

void ambiente_entrar(void) {
    Quadro *novo = alocar(sizeof *novo);
    novo->anterior = atual;
    atual = novo;
    profundidade++;
}

void ambiente_sair(void) {
    if (!atual || atual == global) return;
    Quadro *anterior = atual->anterior;
    liberar_quadro(atual);
    atual = anterior;
    profundidade--;
}

int ambiente_profundidade(void) {
    return profundidade;
}

void ambiente_definir(const char *nome, Valor v) {
    Valor *existente = buscar_quadro(atual, nome);
    if (existente) {
        valor_liberar(*existente);
        *existente = v;
        return;
    }
    Entrada *nova = alocar(sizeof *nova);
    nova->nome = alocar(strlen(nome) + 1);
    strcpy(nova->nome, nome);
    nova->valor = v;
    nova->prox = atual->entradas;
    atual->entradas = nova;
}

Valor *ambiente_buscar(const char *nome) {
    Valor *v = buscar_quadro(atual, nome);
    return v || atual == global ? v : buscar_quadro(global, nome);
}

void ambiente_liberar(void) {
    while (atual) {
        Quadro *anterior = atual->anterior;
        liberar_quadro(atual);
        atual = anterior;
    }
    global = NULL;
    profundidade = 0;
}
