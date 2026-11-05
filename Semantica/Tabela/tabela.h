#ifndef MINIPY_TABELA_H
#define MINIPY_TABELA_H

/* ===== [P4] Tabela de símbolos e escopos ===== */

/* [P4] Contrato da tabela de simbolos da Sprint 3.
 * Implementacao: hash privado de 211 buckets por escopo, com encadeamento,
 * adaptado do exemplo do professor sem alterar a interface combinada:
 * https://github.com/sergioaafreitas/COMP1/blob/main/semana%2007/src/tabela.c
 * Simbolo.prox preserva a ordem de insercao; nao e a cadeia de colisao. */
typedef enum {
    CAT_VARIAVEL, CAT_FUNCAO, CAT_PARAMETRO, CAT_BUILTIN
} Categoria;

typedef struct Simbolo {
    char *nome;
    Categoria categoria;
    char tipo[16];
    int num_params;
    int escopo;
    int linha;
    struct Simbolo *prox;
} Simbolo;

/* O escopo global (0), print e range sao criados no primeiro uso.
 * Cada entrada recebe um ID novo; so funcoes devem chamar esta operacao. */
void tabela_entrar_escopo(const char *nome_funcao);
void tabela_sair_escopo(void);

/* Insere no escopo atual: 0 = sucesso, -1 = nome duplicado no escopo.
 * nome e tipo sao copiados; o chamador continua dono dos argumentos.
 * tipo tem no maximo 15 bytes (mais '\0'); textos maiores sao truncados.
 * num_params inicia em -1 para funcoes/builtins e 0 para as demais categorias. */
int tabela_inserir(const char *nome, Categoria cat, const char *tipo, int linha);
Simbolo *tabela_buscar(const char *nome);
Simbolo *tabela_buscar_local(const char *nome);

/* Colunas separadas por tab: NOME CATEGORIA TIPO ESCOPO LINHA.
 * Ordem: global ate o escopo atual, por ordem de insercao em cada escopo.
 * Somente escopos ativos sao impressos; simbolos locais morrem na saida. */
void tabela_imprimir(void);

/* Libera tudo. Uma chamada posterior reinicia a tabela com seus builtins. */
void tabela_liberar(void);

#endif
