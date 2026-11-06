#ifndef MINIPY_AMBIENTE_H
#define MINIPY_AMBIENTE_H

#include "Interpretador/valor.h"

/* Pilha de quadros. Quadro 0 = global. Cada chamada de funcao empilha um.
 * A busca olha o quadro atual e depois o global (como o Python). */
void   ambiente_iniciar(void);
void   ambiente_entrar(void);                        /* empilha quadro (P5 chama) */
void   ambiente_sair(void);                          /* desempilha e libera os valores */
int    ambiente_profundidade(void);                  /* 0 = so o global */
void   ambiente_definir(const char *nome, Valor v);  /* no quadro atual; assume a posse de v e libera o valor antigo */
Valor *ambiente_buscar(const char *nome);            /* ponteiro emprestado; NULL se nao existe */
void   ambiente_liberar(void);

#endif
