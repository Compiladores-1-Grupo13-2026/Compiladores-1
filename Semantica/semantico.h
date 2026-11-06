#ifndef MINIPY_SEMANTICO_H
#define MINIPY_SEMANTICO_H

#include "Ast/ast.h"

/* ===== [P1] API de analise de expressoes ===== */
const char *semantico_tipo_expr(const NoAST *no);
/* ===== FIM [P1] API de analise de expressoes ===== */

/* ===== [P4] Atribuição, listas e indexação ===== */

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

/* ===== [P5] Ponto de entrada da analise semantica ===== */
/* Percorre a AST inteira do programa (raiz AST_PROGRAMA), criando e
 * consultando a tabela de simbolos. Imprime cada erro em stderr e devolve
 * a quantidade de erros semanticos encontrados. Libera a tabela ao final. */
int semantico_analisar(NoAST *raiz);
/* ===== FIM [P5] Ponto de entrada da analise semantica ===== */

#endif
