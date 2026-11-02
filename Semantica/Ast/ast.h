#ifndef MINIPY_AST_H
#define MINIPY_AST_H

/* ===== [P4] Atribuição, listas e indexação ===== */

/* NoAST, TipoNo e API comum serao acrescentados pela base do grupo. */

/* Os construtores recebem a propriedade dos filhos; cada filho deve ter um
 * unico dono. A lista de elementos ja vem encadeada por ast_anexar/prox. */
NoAST *ast_p4_atribuicao(TipoNo operador, NoAST *alvo, NoAST *valor, int linha);
NoAST *ast_p4_lista(NoAST *elementos, int linha);
NoAST *ast_p4_indexacao(NoAST *base, NoAST *indice, int linha);

/* Retorna NULL para os tipos das outras fatias. O impressor comum pode usar
 * esse nome e percorrer a/b/c/prox no formato acordado (2 espacos por nivel). */
const char *ast_p4_nome_tipo(TipoNo tipo);
/* Imprime um no P4 e seus filhos via ast_imprimir; nao percorre no->prox,
 * responsabilidade do impressor comum. Retorna 0 quando nao for um no P4. */
int ast_p4_imprimir(NoAST *no, int nivel);

/* ===== FIM [P4] Atribuição, listas e indexação ===== */

#endif
