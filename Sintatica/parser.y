%{
#include <math.h>
#include <stdio.h>

int yylex(void);
void yyerror(const char *mensagem);
extern int yylineno;
static int erros_p1 = 0;

static void erro_divisao_zero(void) {
    fprintf(stderr, "Erro na linha %d: divisao por zero\n", yylineno);
    erros_p1++;
}
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
    | expr terminador  { if (!isnan($1)) printf("Resultado: %.15g\n", $1); } /* P1 */
    ;

bloco:
      LBRACE programa RBRACE
    ;

/* ===== [P1] EXPRESSOES E LITERAIS ===== */
terminador:
    SEMICOLON
  | NEWLINE
;

/* Recupera no fim do comando sem descartar as expressoes seguintes. */
comando:
    error SEMICOLON { erros_p1++; yyerrok; }
  | error NEWLINE   { erros_p1++; yyerrok; }
;

expr:
    NUM                     { $$ = $1; }
  | LPAREN expr RPAREN      { $$ = $2; }
  | expr PLUS expr          { $$ = $1 + $3; }
  | expr MINUS expr         { $$ = $1 - $3; }
  | expr TIMES expr         { $$ = $1 * $3; }
  | MINUS expr %prec UMINUS { $$ = -$2; }
  /* Divisao da base com a verificacao de zero prevista para P1. */
  | expr DIVIDE expr {
        if ($3 == 0) {
            erro_divisao_zero();
            YYERROR;
        }
        $$ = $1 / $3;
    }
  | expr FLOORDIV expr {
        if ($3 == 0) {
            erro_divisao_zero();
            YYERROR;
        }
        $$ = floor($1 / $3);
    }
  | expr MOD expr {
        if ($3 == 0) {
            erro_divisao_zero();
            YYERROR;
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

/* ===== [P5] FUNCOES E CHAMADAS ===== */
/* Definicoes de funcoes, parametros, return e chamadas com argumentos. */

%%

void yyerror(const char *mensagem) {
    /* Funcao auxiliar de erro */
}

/* Ponto de entrada do build; as mensagens gerais de yyerror cabem a P2. */
int main(void) {
    int status = yyparse();
    return status != 0 || erros_p1 != 0;
}
