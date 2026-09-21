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

%%

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
/* Comandos while, for, break e continue. */

/* ===== [P4] ATRIBUICAO, LISTAS E COMENTARIOS ===== */
/* Atribuicoes, listas e indexacao. Comentarios sao ignorados no scanner. */

/* ===== [P5] FUNCOES E CHAMADAS ===== */
/* Definicoes de funcoes, parametros, return e chamadas com argumentos. */

%%
