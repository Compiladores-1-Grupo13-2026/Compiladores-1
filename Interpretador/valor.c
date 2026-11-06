#include "Interpretador/valor.h"
#include "Interpretador/interpretador.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Valor valor_none(void) {
    Valor v = { .tipo = VAL_NONE };
    return v;
}

Valor valor_int(long long numero) {
    Valor v = { .tipo = VAL_INT };
    v.u.inteiro = numero;
    return v;
}

Valor valor_real(double numero) {
    Valor v = { .tipo = VAL_REAL };
    v.u.real = numero;
    return v;
}

Valor valor_bool(int booleano) {
    Valor v = { .tipo = VAL_BOOL };
    v.u.booleano = booleano != 0;
    return v;
}

Valor valor_str(const char *texto) {
    Valor v = { .tipo = VAL_STR };
    if (!texto) texto = "";
    size_t tamanho = strlen(texto) + 1;
    v.u.texto = malloc(tamanho);
    if (!v.u.texto) erro_execucao(1, "memoria insuficiente");
    memcpy(v.u.texto, texto, tamanho);
    return v;
}

Valor valor_funcao(NoAST *def) {
    Valor v = { .tipo = VAL_FUNCAO };
    v.u.funcao = def;
    return v;
}

Valor valor_copiar(Valor v) {
    if (v.tipo == VAL_STR) return valor_str(v.u.texto);
    if (v.tipo == VAL_LISTA) {
        if (v.u.lista->referencias == INT_MAX)
            erro_execucao(1, "limite de referencias de lista excedido");
        v.u.lista->referencias++;
    }
    return v;
}

void valor_liberar(Valor v) {
    if (v.tipo == VAL_STR) free(v.u.texto);
    else if (v.tipo == VAL_LISTA && --v.u.lista->referencias == 0)
        lista_liberar(v.u.lista);
}

/* A pilha de listas em impressao impede recursao infinita em l[0] = l. */
typedef struct ListaImpressa {
    const Lista *lista;
    const struct ListaImpressa *anterior;
} ListaImpressa;

static void imprimir_texto(const char *texto, int dentro_de_lista) {
    if (!dentro_de_lista) {
        fputs(texto, stdout);
        return;
    }
    putchar('\'');
    for (const char *p = texto; *p; p++) {
        switch (*p) {
            case '\n': fputs("\\n", stdout); break;
            case '\t': fputs("\\t", stdout); break;
            case '\r': fputs("\\r", stdout); break;
            case '\\': fputs("\\\\", stdout); break;
            case '\'': fputs("\\'", stdout); break;
            default: putchar((unsigned char)*p); break;
        }
    }
    putchar('\'');
}

static void imprimir(Valor v, int dentro_de_lista, const ListaImpressa *pilha) {
    switch (v.tipo) {
        case VAL_NONE:
            fputs("None", stdout);
            break;
        case VAL_INT:
            printf("%lld", v.u.inteiro);
            break;
        case VAL_REAL: {
            char texto[64];
            snprintf(texto, sizeof texto, "%.15g", v.u.real);
            fputs(texto, stdout);
            if (isfinite(v.u.real) && !strchr(texto, '.') &&
                !strchr(texto, 'e') && !strchr(texto, 'E')) fputs(".0", stdout);
            break;
        }
        case VAL_BOOL:
            fputs(v.u.booleano ? "True" : "False", stdout);
            break;
        case VAL_STR:
            imprimir_texto(v.u.texto, dentro_de_lista);
            break;
        case VAL_LISTA: {
            for (const ListaImpressa *p = pilha; p; p = p->anterior) {
                if (p->lista == v.u.lista) {
                    fputs("[...]", stdout);
                    return;
                }
            }
            ListaImpressa atual = { v.u.lista, pilha };
            putchar('[');
            for (int i = 0; i < v.u.lista->tamanho; i++) {
                if (i) fputs(", ", stdout);
                imprimir(v.u.lista->itens[i], 1, &atual);
            }
            putchar(']');
            break;
        }
        case VAL_FUNCAO:
            printf("<funcao %s>", v.u.funcao->texto);
            break;
    }
}

void valor_imprimir(Valor v, int dentro_de_lista) {
    imprimir(v, dentro_de_lista, NULL);
}

const char *valor_nome_tipo(Valor v) {
    switch (v.tipo) {
        case VAL_NONE: return "NoneType";
        case VAL_INT: return "int";
        case VAL_REAL: return "float";
        case VAL_BOOL: return "bool";
        case VAL_STR: return "str";
        case VAL_LISTA: return "list";
        case VAL_FUNCAO: return "function";
    }
    return "?";
}
