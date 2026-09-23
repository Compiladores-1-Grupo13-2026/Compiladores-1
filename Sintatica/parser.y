%{
#include <stdio.h>
#include <math.h>

int yylex(void);
void yyerror(const char *mensagem);
extern char *yytext;
extern int erros_lexicos;
void encerrar_lista_invalida(void);
static void sincronizar_lista(void);
static int erros_p1 = 0;
int erros_sintaticos = 0;

/* P2: guarda a deteccao para permitir um diagnostico especifico na recuperacao.
 * Copiar o lexema evita consultar yytext depois que o scanner ja avancou. */
static int erro_pendente = 0, erro_linha, erro_coluna;
static char erro_lexema[128];
static const char *erro_mensagem = "syntax error";
static void preparar_diagnostico(int linha, int coluna, const char *mensagem);
static void emitir_pendente(void);
static void diagnostico(int linha, int coluna, const char *mensagem);
#define ERRO_SINTATICO(loc, mensagem) \
    diagnostico((loc).first_line, (loc).first_column, (mensagem))
#define RECONHECIDO(mensagem) do { emitir_pendente(); puts(mensagem); } while (0)

static void erro_numerico_p1(int linha, const char *mensagem) {
    emitir_pendente();
    fprintf(stderr, "[ERRO SEMANTICO] Linha %d: %s\n", linha, mensagem);
    erros_p1++;
}

/* NAN ja representa valores apenas reconhecidos, como STRING e NONE.
 * Nao confundir esse marcador com um erro produzido por operandos numericos. */
static int resultado_finito_p1(double resultado, double esquerda, double direita) {
    return isnan(esquerda) || isnan(direita) || isfinite(resultado);
}
%}

%locations
/* Os dois conflitos intencionais sao ELSE do if versus ELSE sem if. */
%expect 2

%union {
    double numero;
}

/* Declaracoes compartilhadas previstas no contrato da Sprint 0. */
%token <numero> NUM
%token ID PLUS MINUS TIMES DIVIDE LPAREN RPAREN LBRACE RBRACE COMMA SEMICOLON
%token FIM 0 "fim do arquivo"
%token NEWLINE
%token STRING TRUE FALSE NONE FLOORDIV MOD POWER
%token EQ NE LT GT LE GE AND OR NOT IF ELIF ELSE
%token WHILE FOR IN BREAK CONTINUE
%token ASSIGN PLUSEQ MINUSEQ TIMESEQ DIVEQ LBRACKET RBRACKET
%token DEF RETURN
%type <numero> expr chamada_funcao

/* Contrato de precedencia da Sprint 0, na ordem exata do plano. */
%left OR
%left AND
%precedence NOT
%left EQ NE LT GT LE GE
%left PLUS MINUS
%left TIMES DIVIDE FLOORDIV MOD
%precedence UMINUS
%right POWER

/* Define o ponto de entrada principal do interpretador */
%start programa

%%

/* ===== ESTRUTURA GERAL DE EXECUCAO ===== */
programa:
      %empty
    | programa elemento
    ;

elemento:
      comando
    | NEWLINE
    ;

comando:
      laco_while       /* P3 */
    | laco_for         /* P3 */
    | comando_break terminador    /* P3 */
    | comando_continue terminador /* P3 */
    | atribuicao terminador       /* P4 */
    | expr terminador  { emitir_pendente(); if (!isnan($1)) printf("Resultado: %.15g\n", $1); } /* P1 */
    ;

bloco:
      LBRACE inicio_bloco programa RBRACE
    | LBRACE inicio_bloco programa error FIM {
          ERRO_SINTATICO(@1, "bloco aberto sem '}'");
          yyerrok;
      }
    ;

/* A abertura e a ancora dos erros de cabecalho. So aqui imprimimos o
 * diagnostico e voltamos ao modo normal: antes disso ainda ha tokens a descartar. */
inicio_bloco:
      %empty { emitir_pendente(); yyerrok; }
    ;

/* ===== [P1] EXPRESSOES E LITERAIS ===== */
terminador:
    SEMICOLON
  | NEWLINE
;

/* Recupera no fim do comando sem descartar as expressoes seguintes. */
comando:
    error terminador { emitir_pendente(); yyerrok; }
;

expr:
    NUM {
        if (!isfinite($1)) {
            erro_numerico_p1(@1.first_line, "literal numerico fora do intervalo suportado");
            YYERROR;
        }
        $$ = $1;
    }
  | LPAREN expr RPAREN      { $$ = $2; }
  | expr PLUS expr {
        $$ = $1 + $3;
        if (!resultado_finito_p1($$, $1, $3)) {
            erro_numerico_p1(@1.first_line, "resultado de soma fora do intervalo suportado");
            YYERROR;
        }
    }
  | expr MINUS expr {
        $$ = $1 - $3;
        if (!resultado_finito_p1($$, $1, $3)) {
            erro_numerico_p1(@1.first_line, "resultado de subtracao fora do intervalo suportado");
            YYERROR;
        }
    }
  | expr TIMES expr {
        $$ = $1 * $3;
        if (!resultado_finito_p1($$, $1, $3)) {
            erro_numerico_p1(@1.first_line, "resultado de multiplicacao fora do intervalo suportado");
            YYERROR;
        }
    }
  | MINUS expr %prec UMINUS { $$ = -$2; }
  | expr DIVIDE expr {
        if ($3 == 0) {
            erro_numerico_p1(@1.first_line, "divisao por zero");
            YYERROR;
        }
        $$ = $1 / $3;
        if (!resultado_finito_p1($$, $1, $3)) {
            erro_numerico_p1(@1.first_line, "resultado de divisao fora do intervalo suportado");
            YYERROR;
        }
    }
  | expr FLOORDIV expr {
        if ($3 == 0) {
            erro_numerico_p1(@1.first_line, "divisao por zero");
            YYERROR;
        }
        $$ = floor($1 / $3);
        if (!resultado_finito_p1($$, $1, $3)) {
            erro_numerico_p1(@1.first_line, "resultado de divisao inteira fora do intervalo suportado");
            YYERROR;
        }
    }
  | expr MOD expr {
        if ($3 == 0) {
            erro_numerico_p1(@1.first_line, "divisao por zero");
            YYERROR;
        }
        $$ = fmod($1, $3);
        /* fmod evita overflow no quociente intermediario e cancelamento
         * na subtracao. O resto segue o sinal do divisor, inclusive em zero. */
        if ($$ == 0.0) $$ = copysign(0.0, $3);
        else if (($$ < 0.0) != ($3 < 0.0)) $$ += $3;
        if (!resultado_finito_p1($$, $1, $3)) {
            erro_numerico_p1(@1.first_line, "resultado de modulo fora do intervalo suportado");
            YYERROR;
        }
    }
  | expr POWER expr {
        if (isnan($1) || isnan($3)) {
            $$ = NAN;
        } else {
            if ($1 == 0.0 && $3 < 0.0) {
                erro_numerico_p1(@1.first_line, "divisao por zero em potencia com expoente negativo");
                YYERROR;
            }
            $$ = pow($1, $3);
            if (isnan($$)) {
                erro_numerico_p1(@1.first_line, "potencia sem resultado real");
                YYERROR;
            }
            if (!isfinite($$)) {
                erro_numerico_p1(@1.first_line, "resultado de potencia fora do intervalo suportado");
                YYERROR;
            }
        }
    }
  /* STRING e NONE sao reconhecidos, sem valor numerico (NAN). */
  | STRING                  { $$ = NAN; }
  | TRUE                    { $$ = 1; }
  | FALSE                   { $$ = 0; }
  | NONE                    { $$ = NAN; }
;

/* ===== [P2] CONDICOES E LOGICA ===== */
/* Comparacoes, operadores logicos e comandos if / elif / else. */

/* Operadores de comparacao e logica. */
expr:
      expr EQ expr {
          $$ = isnan($1) || isnan($3) ? NAN : ($1 == $3);
      }
    | expr NE expr {
          $$ = isnan($1) || isnan($3) ? NAN : ($1 != $3);
      }
    | expr LT expr {
          $$ = isnan($1) || isnan($3) ? NAN : ($1 < $3);
      }
    | expr GT expr {
          $$ = isnan($1) || isnan($3) ? NAN : ($1 > $3);
      }
    | expr LE expr {
          $$ = isnan($1) || isnan($3) ? NAN : ($1 <= $3);
      }
    | expr GE expr {
          $$ = isnan($1) || isnan($3) ? NAN : ($1 >= $3);
      }
    | expr AND expr {
          $$ = isnan($1) || isnan($3) ? NAN : ($1 != 0 && $3 != 0);
      }
    | expr OR expr {
          $$ = isnan($1) || isnan($3) ? NAN : ($1 != 0 || $3 != 0);
      }
    | NOT expr {
          $$ = isnan($2) ? NAN : ($2 == 0);
      }
    ;

/* Comandos condicionais. */
comando:
      comando_if
    ;

comando_if:
      IF expr bloco { RECONHECIDO("[OK] if reconhecido"); }
    | IF expr bloco ELSE bloco { RECONHECIDO("[OK] if reconhecido"); }
    | IF expr bloco lista_elif { RECONHECIDO("[OK] if reconhecido"); }
    | IF expr bloco lista_elif ELSE bloco { RECONHECIDO("[OK] if reconhecido"); }
    | IF { ERRO_SINTATICO(@1, "condicao vazia no if"); } bloco
    | IF error {
          preparar_diagnostico(@1.first_line, @1.first_column, "condicao invalida no if");
      } bloco { yyerrok; }
    /* Nao aceitar IF expr error: interceptava expressoes incompletas e
     * confundia a abertura do bloco com a ausencia de bloco. */
    | IF error terminador {
          ERRO_SINTATICO(@1, "bloco ausente no if (esperado '{')"); yyerrok;
      }
    | ELSE bloco { ERRO_SINTATICO(@1, "else sem if correspondente"); }
    ;

lista_elif:
      ELIF expr bloco
    | lista_elif ELIF expr bloco
    | ELIF { ERRO_SINTATICO(@1, "condicao vazia no elif"); } bloco
    | ELIF error {
          preparar_diagnostico(@1.first_line, @1.first_column, "condicao invalida no elif");
      } bloco { yyerrok; }
    | ELIF error terminador {
          ERRO_SINTATICO(@1, "bloco ausente no elif (esperado '{')"); yyerrok;
      }
    ;

/* ===== FIM [P2] CONDICOES E LOGICA ===== */

/* ===== [P3] LACOS ===== */
laco_while:
      WHILE expr bloco { RECONHECIDO("[OK] while reconhecido"); }
    | WHILE { ERRO_SINTATICO(@1, "condicao vazia no while"); } bloco
    | WHILE error {
          preparar_diagnostico(@1.first_line, @1.first_column, "condicao invalida no while");
      } bloco { yyerrok; }
    ;

laco_for:
      FOR ID IN expr bloco { RECONHECIDO("[OK] for reconhecido"); }
    | FOR ID error {
          preparar_diagnostico(@1.first_line, @1.first_column, "for sem a palavra 'in' ou expressao iteravel invalida");
      } bloco { yyerrok; }
    | FOR error {
          preparar_diagnostico(@1.first_line, @1.first_column, "for sem variavel de controle");
      } bloco { yyerrok; }
    ;

comando_break:
      BREAK { RECONHECIDO("[OK] break reconhecido"); }
    ;

comando_continue:
      CONTINUE { RECONHECIDO("[OK] continue reconhecido"); }
    ;

/* ===== [P4] ATRIBUICAO, LISTAS E COMENTARIOS ===== */
/* Atribuicoes, listas e indexacao. Comentarios sao ignorados no scanner. */

/* Atribuicao simples e composta: x = 1, x += 1, l[0] = 1 */
atribuicao:
      alvo op_atrib expr { RECONHECIDO("[OK] atribuicao reconhecida"); }
    | literal op_atrib expr {
        ERRO_SINTATICO(@1, "atribuicao invalida: o lado esquerdo deve ser variavel ou l[i]");
        yyerrok;
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
        ERRO_SINTATICO(@1, "lista mal formada: faltou ']' (ou ',' entre elementos)");
        sincronizar_lista();
        yyerrok;
    }
    | LBRACKET error {
        ERRO_SINTATICO(@1, "lista mal formada: faltou ']'");
        sincronizar_lista();
        yyerrok;
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

comando:
      def_funcao
    | comando_return
    | ID LPAREN args_opt error terminador {
          ERRO_SINTATICO(@1, "chamada de funcao sem fecha parenteses ')' antes do terminador");
          yyerrok;
      }
    ;

def_funcao:
      DEF ID LPAREN params_opt RPAREN bloco {
          RECONHECIDO("[OK] def reconhecido");
      }
    | DEF error LPAREN params_opt RPAREN {
          ERRO_SINTATICO(@1, "definicao de funcao sem identificador (esperado nome antes de '(')");
          yyerrok;
      } bloco
    | DEF error {
          preparar_diagnostico(@1.first_line, @1.first_column, "definicao de funcao malformada (esperado nome e parametros antes do bloco)");
      } bloco { yyerrok; }
    | DEF ID LPAREN error RPAREN {
          ERRO_SINTATICO(@1, "parametros mal formados na definicao de funcao. Recuperado apos ')'.");
          yyerrok;
      } bloco
    ;

params_opt:
      %empty
    | params
    ;

params:
      ID
    | params COMMA ID
    ;

comando_return:
      RETURN expr terminador {
          RECONHECIDO("[OK] return reconhecido");
      }
    | RETURN terminador {
          RECONHECIDO("[OK] return reconhecido");
      }
    | RETURN ID LPAREN args_opt error terminador {
          ERRO_SINTATICO(@1, "chamada de funcao sem fecha parenteses ')' no return antes do terminador");
          yyerrok;
      }
    ;

expr:
      chamada_funcao { $$ = $1; }
    | ID             { $$ = NAN; }
    ;

chamada_funcao:
      ID LPAREN args_opt RPAREN {
          RECONHECIDO("[OK] chamada de funcao reconhecida");
          $$ = NAN;
      }
    ;

args_opt:
      %empty
    | args
    ;

args:
      expr
    | args COMMA expr
    ;

%%

/* P4: preserva o terminador para a regra externa e consome um eventual ']'. */
static void sincronizar_lista(void) {
    while (yychar != FIM && yychar != NEWLINE && yychar != SEMICOLON &&
           yychar != RBRACE && yychar != RBRACKET) yychar = yylex();
    if (yychar == RBRACKET) yychar = YYEMPTY;
    encerrar_lista_invalida();
}

static const char *lexema_atual(void) {
    if (yychar == NEWLINE) return "\\n";
    if (yychar == FIM || !yytext || !*yytext) return "EOF";
    if (*yytext == '\n') return "\\n";
    return yytext;
}

static void emitir_pendente(void) {
    if (!erro_pendente) return;
    fprintf(stderr, "[ERRO SINTATICO] Linha %d, coluna %d: %s (proximo a '%s')\n",
            erro_linha, erro_coluna, erro_mensagem, erro_lexema);
    erro_pendente = 0;
}

static void preparar_diagnostico(int linha, int coluna, const char *mensagem) {
    erro_linha = linha;
    erro_coluna = coluna;
    erro_mensagem = mensagem;
}

static void diagnostico(int linha, int coluna, const char *mensagem) {
    /* A regra especifica substitui a mensagem generica ainda pendente. */
    const char *lexema = erro_pendente ? erro_lexema : lexema_atual();
    if (!erro_pendente) erros_sintaticos++;
    fprintf(stderr, "[ERRO SINTATICO] Linha %d, coluna %d: %s (proximo a '%s')\n",
            linha, coluna, mensagem, lexema);
    erro_pendente = 0;
}

void yyerror(const char *mensagem) {
    (void) mensagem;
    emitir_pendente();
    erros_sintaticos++;
    erro_pendente = 1;
    erro_mensagem = "syntax error";
    erro_linha = yylloc.first_line;
    erro_coluna = yylloc.first_column;
    snprintf(erro_lexema, sizeof erro_lexema, "%s", lexema_atual());
}

/* Semana 5: o ponto de entrada continua aqui, sem integrar src/main.c. */
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int status = yyparse();
    emitir_pendente();
    return status != 0 || erros_p1 != 0 || erros_sintaticos != 0 || erros_lexicos != 0;
}
