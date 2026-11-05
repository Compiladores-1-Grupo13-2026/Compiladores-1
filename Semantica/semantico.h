#ifndef MINIPY_SEMANTICO_H
#define MINIPY_SEMANTICO_H


/* ===== [P4] Atribuição, listas e indexação ===== */

#include "Ast/ast.h"

/* Adaptadores para o visitante e erro_semantico da base P2. As strings de tipo
 * retornadas pelo visitante precisam continuar validas ate o fim da visita.
 * "?" = tipo ainda desconhecido; "erro" = filho com erro ja diagnosticado.
 * Cada chamada visita UM no; o dono de uma sequencia percorre prox. */
#define P4_TIPO_ERRO "erro"
typedef const char *(*P4Visitar)(NoAST *no);
typedef void (*P4Erro)(int linha, const char *formato, ...);

/* Trata os sete tipos P4 e IDENTIFICADOR em contexto de leitura.
 * Retorna NULL para os demais tipos, que devem ser tratados por sua fatia.
 * Nao inicializa/limpa a tabela nem cria escopos: isso cabe a base e a P5. */
const char *semantico_p4_visitar(NoAST *no, P4Visitar visitar, P4Erro erro);

/* ===== FIM [P4] Atribuição, listas e indexação ===== */
=======
#include "Ast/ast.h"

/* ===== [P1] API de registro e analise de expressoes ===== */
/* O parser tambem encaminha expressoes de outras regras para esta analise. */
void semantico_registrar_expressao(NoAST *raiz, int imprimir);
NoAST *semantico_ast(void);
int semantico_analisar(NoAST *raiz);
const char *semantico_tipo_expr(const NoAST *no);
void semantico_liberar(NoAST *raiz_impressa);
/* ===== FIM [P1] API de registro e analise de expressoes ===== */

#endif
