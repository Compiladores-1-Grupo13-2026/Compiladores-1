/* ===== [P4] Dubles de teste das dependencias da AST ===== */
#ifndef MINIPY_P4_DUBLES_H
#define MINIPY_P4_DUBLES_H

#include <stddef.h>
#include "contrato_ast_teste.h"

/* Dados pre-montados em arena estatica. Nao implementam o nucleo AST P1. */
void teste_reiniciar(void);
NoAST *teste_no(TipoNo tipo, int linha);
NoAST *teste_numero(int valor, int linha);
NoAST *teste_texto(TipoNo tipo, const char *texto, int linha);

typedef struct TesteImpressao {
    NoAST *no;
    int nivel;
} TesteImpressao;

void teste_impressao_reiniciar(void);
size_t teste_impressao_total(void);
const TesteImpressao *teste_impressao_chamada(size_t indice);

#endif
