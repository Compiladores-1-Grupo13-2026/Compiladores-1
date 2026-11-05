#include "tabela.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ===== [P4] Tabela de símbolos e escopos ===== */

/* [P4] Hash com encadeamento, como no exemplo da Semana 7 do professor:
 * https://github.com/sergioaafreitas/COMP1/blob/main/semana%2007/src/tabela.c
 * A pilha de escopos e os atributos extras seguem o contrato do MiniPy.
 * O indice privado nao altera Simbolo nem usa seu prox como cadeia de colisao:
 * prox conserva a ordem de insercao para imprimir a tabela deterministicamente. */
#define TAM_HASH 211

typedef struct EntradaHash {
    Simbolo *simbolo;
    struct EntradaHash *proxima;
} EntradaHash;

typedef struct Escopo {
    int id;
    char *nome_funcao;
    Simbolo *primeiro;
    Simbolo *ultimo;
    EntradaHash *buckets[TAM_HASH];
    struct Escopo *anterior;
} Escopo;

static Escopo *atual;
static int proximo_escopo = 1;

/* Mesma funcao de dispersao do professor; unsigned char evita depender
 * do sinal de char quando um nome contem bytes fora de ASCII. */
static unsigned hash_nome(const char *nome) {
    unsigned h = 0;
    while (*nome) h = (h << 4) + (unsigned char)*nome++;
    return h % TAM_HASH;
}

static Simbolo *buscar_em(const Escopo *escopo, const char *nome, unsigned indice) {
    for (const EntradaHash *entrada = escopo->buckets[indice]; entrada;
         entrada = entrada->proxima) {
        if (strcmp(entrada->simbolo->nome, nome) == 0) return entrada->simbolo;
    }
    return NULL;
}

static void *alocar(size_t tamanho) {
    void *memoria = calloc(1, tamanho);
    if (!memoria) {
        fputs("Erro: memoria insuficiente na tabela de simbolos\n", stderr);
        exit(EXIT_FAILURE);
    }
    return memoria;
}

static char *copiar(const char *texto) {
    size_t tamanho = strlen(texto) + 1;
    char *copia = alocar(tamanho);
    memcpy(copia, texto, tamanho);
    return copia;
}

static void inserir_em(Escopo *escopo, const char *nome, Categoria categoria,
                      const char *tipo, int linha) {
    Simbolo *simbolo = alocar(sizeof *simbolo);
    simbolo->nome = copiar(nome);
    simbolo->categoria = categoria;
    snprintf(simbolo->tipo, sizeof simbolo->tipo, "%s", tipo);
    simbolo->num_params = categoria == CAT_FUNCAO || categoria == CAT_BUILTIN ? -1 : 0;
    simbolo->escopo = escopo->id;
    simbolo->linha = linha;
    if (escopo->ultimo) escopo->ultimo->prox = simbolo;
    else escopo->primeiro = simbolo;
    escopo->ultimo = simbolo;

    unsigned indice = hash_nome(nome);
    EntradaHash *entrada = alocar(sizeof *entrada);
    entrada->simbolo = simbolo;
    entrada->proxima = escopo->buckets[indice];
    escopo->buckets[indice] = entrada;
}

static void inicializar(void) {
    if (atual) return;
    atual = alocar(sizeof *atual);
    atual->id = 0;
    inserir_em(atual, "print", CAT_BUILTIN, "funcao", 0);
    inserir_em(atual, "range", CAT_BUILTIN, "funcao", 0);
}

void tabela_entrar_escopo(const char *nome_funcao) {
    inicializar();
    Escopo *novo = alocar(sizeof *novo);
    novo->id = proximo_escopo++;
    novo->nome_funcao = copiar(nome_funcao ? nome_funcao : "");
    novo->anterior = atual;
    atual = novo;
}

static void liberar_escopo(Escopo *escopo) {
    for (unsigned i = 0; i < TAM_HASH; i++) {
        EntradaHash *entrada = escopo->buckets[i];
        while (entrada) {
            EntradaHash *proxima = entrada->proxima;
            free(entrada);
            entrada = proxima;
        }
    }
    Simbolo *simbolo = escopo->primeiro;
    while (simbolo) {
        Simbolo *proximo = simbolo->prox;
        free(simbolo->nome);
        free(simbolo);
        simbolo = proximo;
    }
    free(escopo->nome_funcao);
    free(escopo);
}

void tabela_sair_escopo(void) {
    inicializar();
    if (!atual->anterior) return; /* O global permanece ate tabela_liberar. */
    Escopo *anterior = atual->anterior;
    liberar_escopo(atual);
    atual = anterior;
}

Simbolo *tabela_buscar_local(const char *nome) {
    inicializar();
    if (!nome) return NULL;
    return buscar_em(atual, nome, hash_nome(nome));
}

Simbolo *tabela_buscar(const char *nome) {
    inicializar();
    if (!nome) return NULL;
    unsigned indice = hash_nome(nome);
    for (Escopo *escopo = atual; escopo; escopo = escopo->anterior) {
        Simbolo *simbolo = buscar_em(escopo, nome, indice);
        if (simbolo) return simbolo;
    }
    return NULL;
}

int tabela_inserir(const char *nome, Categoria cat, const char *tipo, int linha) {
    inicializar();
    if (!nome || !*nome || tabela_buscar_local(nome)) return -1;
    inserir_em(atual, nome, cat, tipo ? tipo : "?", linha);
    return 0;
}

static const char *nome_categoria(Categoria categoria) {
    switch (categoria) {
        case CAT_VARIAVEL: return "variavel";
        case CAT_FUNCAO: return "funcao";
        case CAT_PARAMETRO: return "parametro";
        case CAT_BUILTIN: return "builtin";
        default: return "?";
    }
}

static void imprimir_escopo(const Escopo *escopo) {
    if (escopo->anterior) imprimir_escopo(escopo->anterior);
    for (const Simbolo *simbolo = escopo->primeiro; simbolo; simbolo = simbolo->prox) {
        printf("%s\t%s\t%s\t%d\t%d\n", simbolo->nome,
               nome_categoria(simbolo->categoria), simbolo->tipo,
               simbolo->escopo, simbolo->linha);
    }
}

void tabela_imprimir(void) {
    inicializar();
    puts("NOME\tCATEGORIA\tTIPO\tESCOPO\tLINHA");
    imprimir_escopo(atual);
}

void tabela_liberar(void) {
    while (atual) {
        Escopo *anterior = atual->anterior;
        liberar_escopo(atual);
        atual = anterior;
    }
    proximo_escopo = 1;
}
