%{
#include <math.h>

int yylex(void);
void yyerror(const char *mensagem);
%}

%union {
    double numero;
}

/* Declaracoes compartilhadas previstas no contrato da Sprint 0. */
%token <numero> NUM
%token ID PLUS MINUS TIMES DIVIDE LPAREN RPAREN LBRACE RBRACE COMMA SEMICOLON
%token NEWLINE
%token STRING TRUE FALSE NONE FLOORDIV MOD POWER
%token EQ NE LT GT LE GE AND OR NOT IF ELIF ELSE
%token WHILE FOR IN BREAK CONTINUE
%token ASSIGN PLUSEQ MINUSEQ TIMESEQ DIVEQ LBRACKET RBRACKET
%token DEF RETURN
%type <numero> expr

/* Contrato de precedencia da Sprint 0, na ordem exata do plano. */
%left OR
%left AND
%right NOT
%left EQ NE LT GT LE GE
%left PLUS MINUS
%left TIMES DIVIDE FLOORDIV MOD
%right UMINUS
%right POWER

/* [P4] Apoio para as mensagens de erro de atribuicao e listas. */
%{
#include <stdio.h>
extern int yylineno;
#define ERRO_P4(msg) fprintf(stderr, "Erro sintatico na linha %d: %s\n", yylineno, msg)
%}

/* Define o ponto de entrada principal do interpretador */
%start programa

%%

/* ===== ESTRUTURA GERAL DE EXECUCAO ===== */
programa:
      /* vazio */
    | programa elemento
    ;

elemento:
      comando
    | NEWLINE
    ;

comando:
      laco_while       /* P3 */
    | laco_for         /* P3 */
    | comando_break    /* P3 */
    | comando_continue /* P3 */
    | atribuicao       /* P4 */
    | expr             /* P1 */
    ;

bloco:
      LBRACE programa RBRACE
    ;

/* ===== [P1] EXPRESSOES E LITERAIS ===== */
expr:
    MINUS expr %prec UMINUS { $$ = -$2; }
  /* Divisao da base com a verificacao de zero prevista para P1. */
  | expr DIVIDE expr {
        if ($3 == 0) {
            yyerror("divisao por zero");
            YYABORT;
        }
        $$ = $1 / $3;
    }
  | expr FLOORDIV expr {
        if ($3 == 0) {
            yyerror("divisao por zero");
            YYABORT;
        }
        $$ = floor($1 / $3);
    }
  | expr MOD expr {
        if ($3 == 0) {
            yyerror("divisao por zero");
            YYABORT;
        }
        $$ = $1 - floor($1 / $3) * $3;
    }
  | expr POWER expr {
        $$ = (isnan($1) || isnan($3)) ? NAN : pow($1, $3);
    }
  /* STRING e NONE sao reconhecidos, sem valor numerico (NAN). */
  | STRING                  { $$ = NAN; }
  | TRUE                    { $$ = 1; }
  | FALSE                   { $$ = 0; }
  | NONE                    { $$ = NAN; }
;

/* ===== [P2] CONDICOES E LOGICA ===== */
/* Comparacoes, operadores logicos e comandos if / elif / else. */

/* ===== [P3] LACOS ===== */
laco_while:
      WHILE expr bloco
    ;

laco_for:
      FOR ID IN expr bloco
    ;

comando_break:
      BREAK
    ;

comando_continue:
      CONTINUE
    ;

/* ===== [P4] ATRIBUICAO, LISTAS E COMENTARIOS ===== */
/* Atribuicoes, listas e indexacao. Comentarios sao ignorados no scanner. */

/* Atribuicao simples e composta: x = 1, x += 1, l[0] = 1 */
atribuicao:
      alvo op_atrib expr
    | literal op_atrib expr {
        ERRO_P4("atribuicao invalida: o lado esquerdo deve ser variavel ou l[i]");
        YYABORT;
    }
    ;

op_atrib:
      ASSIGN | PLUSEQ | MINUSEQ | TIMESEQ | DIVEQ
    ;

/* Alvo valido: variavel ou posicao de lista. */
alvo:
      ID
    | indexacao
    ;

/* Literais que nao podem ficar a esquerda de "=" (ex.: 3 = x). */
literal:
      NUM | STRING | TRUE | FALSE | NONE
    ;

/* Lista [a, b]. Como STRING e NONE, seu valor numerico e NAN. */
expr:
      lista       { $$ = NAN; }
    | indexacao   { $$ = NAN; }
    ;

lista:
      LBRACKET RBRACKET
    | LBRACKET elementos RBRACKET
    | LBRACKET elementos error {
        ERRO_P4("lista mal formada: faltou ']' (ou ',' entre elementos)");
        YYABORT;
    }
    | LBRACKET error {
        ERRO_P4("lista mal formada: faltou ']'");
        YYABORT;
    }
    ;

elementos:
      expr
    | elementos COMMA expr
    ;

/* Indexacao l[0]. */
indexacao:
      ID LBRACKET expr RBRACKET
    ;

/* ===== [P5] FUNCOES E CHAMADAS ===== */
/* Definicoes de funcoes, parametros, return e chamadas com argumentos. */

%%

void yyerror(const char *mensagem) {
    /* Funcao auxiliar de erro */
}