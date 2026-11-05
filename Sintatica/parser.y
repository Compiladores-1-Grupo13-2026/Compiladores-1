%code requires {
#include "Semantica/Ast/ast.h"
typedef struct {
    double numero;
    NoAST *ast;
} ValorExpressao;
}

%{
#include <limits.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>

#include "Semantica/semantico.h"

int yylex(void);
void yyerror(const char *mensagem);
extern char *yytext;
extern int erros_lexicos;
void lexer_liberar_textos(void);
void encerrar_lista_invalida(void);
static void sincronizar_lista(void);
int erros_sintaticos = 0;

/* P2: guarda a deteccao para permitir um diagnostico especifico na recuperacao. */
static int erro_pendente = 0, erro_linha, erro_coluna;
static char erro_lexema[128];
static const char *erro_mensagem = "syntax error";
static void preparar_diagnostico(int linha, int coluna, const char *mensagem);
static void emitir_pendente(void);
static void diagnostico(int linha, int coluna, const char *mensagem);
#define ERRO_SINTATICO(loc, mensagem) \
    diagnostico((loc).first_line, (loc).first_column, (mensagem))
#define RECONHECIDO(mensagem) do { emitir_pendente(); puts(mensagem); } while (0)

/* O contrato da AST guarda inteiros em valor; a API P1 preserva numeros reais
 * em um registro auxiliar sem alterar a struct compartilhada. */
/* ===== [P1] Conversao de literais numericos para nos AST ===== */
static NoAST *criar_numero_p1(double valor, int linha) {
    if (isfinite(valor) && valor >= INT_MIN && valor <= INT_MAX &&
        trunc(valor) == valor)
        return ast_criar_numero((int) valor, linha);
    return ast_criar_numero_real(valor, linha);

}
/* ===== FIM [P1] Conversao de literais numericos para nos AST ===== */
%}

%locations
/* Os dois conflitos intencionais sao ELSE do if versus ELSE sem if. */
%expect 2

%union {
    double numero;
    char *texto;

    ValorExpressao expr;

}

/* Declaracoes compartilhadas */
%token <numero> NUM

%token <texto> ID STRING

%token PLUS MINUS TIMES DIVIDE LPAREN RPAREN LBRACE RBRACE COMMA SEMICOLON
%token FIM 0 "fim do arquivo"
%token NEWLINE
%token TRUE FALSE NONE FLOORDIV MOD POWER
%token EQ NE LT GT LE GE AND OR NOT IF ELIF ELSE
%token WHILE FOR IN BREAK CONTINUE
%token ASSIGN PLUSEQ MINUSEQ TIMESEQ DIVEQ LBRACKET RBRACKET
%token DEF RETURN

/* Tipos retornados pelas regras */
%type <numero> expr chamada_funcao
%type <no> programa elemento bloco while_stmt for_stmt break_stmt continue_stmt comando def_funcao comando_return atribuicao

%type <expr> expr chamada_funcao
%destructor { ast_liberar($$.ast); } <expr>

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
      %empty { $$ = NULL; }
    | programa elemento { $$ = $1; }
    ;

elemento:
      comando { $$ = $1; }
    | NEWLINE { $$ = NULL; }
    ;

comando:
      while_stmt                   /* P3 */
    | for_stmt                     /* P3 */
    | break_stmt                   /* P3 */
    | continue_stmt                /* P3 */
    | atribuicao terminador       /* P4 */
    | expr terminador  {
          emitir_pendente();
          if ($1.ast) semantico_registrar_expressao($1.ast, 1);
          else if (!isnan($1.numero)) printf("Resultado: %.15g\n", $1.numero);
      } /* [P1] Encaminha a expressao para analise e impressao; P2 mantem sua saida numerica. */
    ;

bloco:
      LBRACE inicio_bloco programa RBRACE { $$ = $3; }
    | LBRACE inicio_bloco programa error FIM {
          ERRO_SINTATICO(@1, "bloco aberto sem '}'");
          yyerrok;
          $$ = NULL;
      }
    ;

/* A abertura e a ancora dos erros de cabecalho. So aqui imprimimos o
- diagnostico e voltamos ao modo normal: antes disso ainda ha tokens a descartar. */
inicio_bloco:
      %empty { emitir_pendente(); yyerrok; }
    ;

/* ===== [P1] EXPRESSOES E LITERAIS ===== */
/* Terminadores e recuperacao abaixo sao apoio compartilhado do parser. */
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
        $$.numero = $1;
        $$.ast = criar_numero_p1($1, @1.first_line);
    }
  | LPAREN expr RPAREN { $$ = $2; }
  | expr PLUS expr {
        $$.numero = $1.numero + $3.numero;
        $$.ast = ast_criar(AST_SOMA, $1.ast, $3.ast, NULL, @2.first_line);
    }
  | expr MINUS expr {
        $$.numero = $1.numero - $3.numero;
        $$.ast = ast_criar(AST_SUBTRACAO, $1.ast, $3.ast, NULL, @2.first_line);
    }
  | expr TIMES expr {
        $$.numero = $1.numero * $3.numero;
        $$.ast = ast_criar(AST_MULTIPLICACAO, $1.ast, $3.ast, NULL, @2.first_line);
    }
  | MINUS expr %prec UMINUS {
        $$.numero = -$2.numero;
        $$.ast = ast_criar(AST_NEGATIVO, $2.ast, NULL, NULL, @1.first_line);
    }
  | expr DIVIDE expr {
        $$.numero = $3.numero == 0.0 ? NAN : $1.numero / $3.numero;
        $$.ast = ast_criar(AST_DIVISAO, $1.ast, $3.ast, NULL, @2.first_line);
    }
  | expr FLOORDIV expr {
        $$.numero = $3.numero == 0.0 ? NAN : floor($1.numero / $3.numero);
        $$.ast = ast_criar(AST_DIVINT, $1.ast, $3.ast, NULL, @2.first_line);
    }
  | expr MOD expr {

        if ($3.numero == 0.0) {
            $$.numero = NAN;
        } else {
            $$.numero = fmod($1.numero, $3.numero);
            if ($$.numero == 0.0) $$.numero = copysign(0.0, $3.numero);
            else if (($$.numero < 0.0) != ($3.numero < 0.0)) $$.numero += $3.numero;
        }
        $$.ast = ast_criar(AST_MODULO, $1.ast, $3.ast, NULL, @2.first_line);
    }
  | expr POWER expr {
        $$.numero = (isnan($1.numero) || isnan($3.numero)) ? NAN :
                    pow($1.numero, $3.numero);
        $$.ast = ast_criar(AST_POTENCIA, $1.ast, $3.ast, NULL, @2.first_line);
    }
  | STRING {
        $$.numero = NAN;
        $$.ast = ast_criar_texto(AST_STRING, $1, @1.first_line);
    }
  | TRUE {
        $$.numero = 1.0;
        $$.ast = ast_criar(AST_BOOL, NULL, NULL, NULL, @1.first_line);
        if ($$.ast) $$.ast->valor = 1;
    }
  | FALSE {
        $$.numero = 0.0;
        $$.ast = ast_criar(AST_BOOL, NULL, NULL, NULL, @1.first_line);
        if ($$.ast) $$.ast->valor = 0;
    }
  | NONE {
        $$.numero = NAN;
        $$.ast = ast_criar(AST_NONE, NULL, NULL, NULL, @1.first_line);
    }
  | ID {
        $$.numero = NAN;
        $$.ast = ast_criar_texto(AST_IDENTIFICADOR, $1, @1.first_line);
    }
;
/* ===== FIM [P1] EXPRESSOES E LITERAIS ===== */

/* ===== [P2] CONDICOES E LOGICA ===== */
/* Comparacoes, operadores logicos e comandos if / elif / else. */

/* Operadores de comparacao e logica. */
expr:
      expr EQ expr {
          semantico_registrar_expressao($1.ast, 0);
          semantico_registrar_expressao($3.ast, 0);
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero == $3.numero);
          $$.ast = NULL;
      }
    | expr NE expr {
          semantico_registrar_expressao($1.ast, 0);
          semantico_registrar_expressao($3.ast, 0);
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero != $3.numero);
          $$.ast = NULL;
      }
    | expr LT expr {
          semantico_registrar_expressao($1.ast, 0);
          semantico_registrar_expressao($3.ast, 0);
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero < $3.numero);
          $$.ast = NULL;
      }
    | expr GT expr {
          semantico_registrar_expressao($1.ast, 0);
          semantico_registrar_expressao($3.ast, 0);
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero > $3.numero);
          $$.ast = NULL;
      }
    | expr LE expr {
          semantico_registrar_expressao($1.ast, 0);
          semantico_registrar_expressao($3.ast, 0);
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero <= $3.numero);
          $$.ast = NULL;
      }
    | expr GE expr {
          semantico_registrar_expressao($1.ast, 0);
          semantico_registrar_expressao($3.ast, 0);
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero >= $3.numero);
          $$.ast = NULL;
      }
    | expr AND expr {
          semantico_registrar_expressao($1.ast, 0);
          semantico_registrar_expressao($3.ast, 0);
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero != 0 && $3.numero != 0);
          $$.ast = NULL;
      }
    | expr OR expr {
          semantico_registrar_expressao($1.ast, 0);
          semantico_registrar_expressao($3.ast, 0);
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero != 0 || $3.numero != 0);
          $$.ast = NULL;
      }
    | NOT expr {
          semantico_registrar_expressao($2.ast, 0);
          $$.numero = isnan($2.numero) ? NAN : ($2.numero == 0);
          $$.ast = NULL;
      }
    ;

/* Comandos condicionais. */
comando:
      comando_if
    ;

comando_if:
      IF expr bloco { semantico_registrar_expressao($2.ast, 0); RECONHECIDO("[OK] if reconhecido"); }
    | IF expr bloco ELSE bloco { semantico_registrar_expressao($2.ast, 0); RECONHECIDO("[OK] if reconhecido"); }
    | IF expr bloco lista_elif { semantico_registrar_expressao($2.ast, 0); RECONHECIDO("[OK] if reconhecido"); }
    | IF expr bloco lista_elif ELSE bloco { semantico_registrar_expressao($2.ast, 0); RECONHECIDO("[OK] if reconhecido"); }
    | IF { ERRO_SINTATICO(@1, "condicao vazia no if"); } bloco
    | IF error {
          preparar_diagnostico(@1.first_line, @1.first_column, "condicao invalida no if");
      } bloco { yyerrok; }
    /* Nao aceitar IF expr error: interceptava expressoes incompletas e
- confundia a abertura do bloco com a ausencia de bloco. */
    | IF error terminador {
          ERRO_SINTATICO(@1, "bloco ausente no if (esperado '{')"); yyerrok;
      }
    | ELSE bloco { ERRO_SINTATICO(@1, "else sem if correspondente"); }
    ;

lista_elif:
      ELIF expr bloco { semantico_registrar_expressao($2.ast, 0); }
    | lista_elif ELIF expr bloco { semantico_registrar_expressao($3.ast, 0); }
    | ELIF { ERRO_SINTATICO(@1, "condicao vazia no elif"); } bloco
    | ELIF error {
          preparar_diagnostico(@1.first_line, @1.first_column, "condicao invalida no elif");
      } bloco { yyerrok; }
    | ELIF error terminador {
          ERRO_SINTATICO(@1, "bloco ausente no elif (esperado '{')"); yyerrok;
      }
    ;

/* ===== FIM [P2] CONDICOES E LOGICA ===== */


/* ===== [P3] LAÇOS ===== */

while_stmt:
    WHILE expr bloco {
        $$ = ast_criar(AST_WHILE, $2, $3, NULL, @1.first_line);
    }
;

/* ===== [P3] LACOS ===== */
laco_while:
      WHILE expr bloco { semantico_registrar_expressao($2.ast, 0); RECONHECIDO("[OK] while reconhecido"); }
    | WHILE { ERRO_SINTATICO(@1, "condicao vazia no while"); } bloco
    | WHILE error {
          preparar_diagnostico(@1.first_line, @1.first_column, "condicao invalida no while");
      } bloco { yyerrok; }
    ;

laco_for:
      FOR ID IN expr bloco { semantico_registrar_expressao($4.ast, 0); RECONHECIDO("[OK] for reconhecido"); }
    | FOR ID error {
          preparar_diagnostico(@1.first_line, @1.first_column, "for sem a palavra 'in' ou expressao iteravel invalida");
      } bloco { yyerrok; }
    | FOR error {
          preparar_diagnostico(@1.first_line, @1.first_column, "for sem variavel de controle");
      } bloco { yyerrok; }
    ;

for_stmt:
    FOR ID IN expr bloco {
        $$ = ast_criar_texto(AST_FOR, $2, @1.first_line);
        $$->a = $4; /* Iterável */
        $$->b = $5; /* Corpo */
        free($2);
    }
;

break_stmt:
    BREAK terminador {
        $$ = ast_criar(AST_BREAK, NULL, NULL, NULL, @1.first_line);
    }
;

continue_stmt:
    CONTINUE terminador {
        $$ = ast_criar(AST_CONTINUE, NULL, NULL, NULL, @1.first_line);
    }
;
/* ===== [P4] ATRIBUICAO, LISTAS E COMENTARIOS ===== */
/* Atribuicoes, listas e indexacao. Comentarios sao ignorados no scanner. */

/* Atribuicao simples e composta: x = 1, x += 1, l[0] = 1 */
atribuicao:
      alvo op_atrib expr { semantico_registrar_expressao($3.ast, 0); RECONHECIDO("[OK] atribuicao reconhecida"); }
    | literal op_atrib expr {
        (void) $3;
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
      lista       { $$.numero = NAN; $$.ast = NULL; }
    | indexacao   { $$.numero = NAN; $$.ast = NULL; }
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
      expr { semantico_registrar_expressao($1.ast, 0); }
    | elementos COMMA expr { semantico_registrar_expressao($3.ast, 0); }
    ;

/* Indexacao l[0]. */
indexacao:
      ID LBRACKET expr RBRACKET { semantico_registrar_expressao($3.ast, 0); }
    ;

/* ===== [P4] AST e recuperacao - preparacao para a base do grupo =====
 * As regras ativas acima pertencem a entrega sintatica atual.
 * As acoes abaixo ficam neste mesmo arquivo, junto da secao P4, aguardando
 * a base comum, ID/STRING carregarem texto e a integracao da equipe.
 * Nao constituem um segundo parser nem um modo AST ja disponivel.
 * Substituir as producoes P4 correspondentes quando a base estiver pronta.
 * Modelo alinhado com a P1: expr <expr> usa ValorExpressao.ast;
 * atribuicao/lista/elementos/indexacao <no>; op_atrib <operador>.
 * Os membros da union e a raiz/blocos dependem da integracao P1/P5.
 * Precedencia de LBRACKET: apos POWER. Necessita <stdlib.h> e destruidores.
 *
 * atribuicao:
 *       expr op_atrib expr {
 *           $$ = ast_p4_atribuicao($2, $1.ast, $3.ast, @1.first_line);
 *       }
 *     ;
 *
 * op_atrib:
 *       ASSIGN  { $$ = AST_ATRIBUICAO; }
 *     | PLUSEQ  { $$ = AST_ATRIB_SOMA; }
 *     | MINUSEQ { $$ = AST_ATRIB_SUB; }
 *     | TIMESEQ { $$ = AST_ATRIB_MULT; }
 *     | DIVEQ   { $$ = AST_ATRIB_DIV; }
 *     ;
 *
 * expr:
 *       lista     { $$ = (ValorExpressao){ .numero = NAN, .ast = $1 }; }
 *     | indexacao { $$ = (ValorExpressao){ .numero = NAN, .ast = $1 }; }
 *     ;
 *
 * lista:
 *       LBRACKET RBRACKET { $$ = ast_p4_lista(NULL, @1.first_line); }
 *     | LBRACKET elementos RBRACKET { $$ = ast_p4_lista($2, @1.first_line); }
 *
 *     | LBRACKET elementos error {
 *           ast_liberar($2);
 *           ERRO_SINTATICO(@1, "lista mal formada: faltou ']' (ou ',' entre elementos)");
 *           sincronizar_lista();
 *           $$ = ast_p4_lista(NULL, @1.first_line);
 *           yyerrok;
 *       }
 *     | LBRACKET error {
 *           ERRO_SINTATICO(@1, "lista mal formada: faltou ']'");
 *           sincronizar_lista();
 *           $$ = ast_p4_lista(NULL, @1.first_line);
 *           yyerrok;
 *       }
 *     ;
 *
 * elementos:
 *       expr { $$ = $1.ast; }
 *     | elementos COMMA expr { $$ = ast_anexar($1, $3.ast); }
 *     ;
 *
 *
 * indexacao:
 *       expr LBRACKET expr RBRACKET {
 *           $$ = ast_p4_indexacao($1.ast, $3.ast, @1.first_line);
 *       }
 *     ;
 *
 * Helper correspondente para quando ID/STRING possuirem texto alocado:
 *
 * static void sincronizar_lista(void) {
 *     while (yychar != FIM && yychar != NEWLINE && yychar != SEMICOLON &&
 *            yychar != RBRACE && yychar != RBRACKET) {
 *         if (yychar == ID || yychar == STRING) free(yylval.texto);
 *         yychar = yylex();
 *     }
 *     if (yychar == RBRACKET) yychar = YYEMPTY;
 *     encerrar_lista_invalida();
 * }
 * ===== FIM [P4] Preparacao da AST ===== */

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
          semantico_registrar_expressao($2.ast, 0);
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
    ;

chamada_funcao:
      ID LPAREN args_opt RPAREN {
          RECONHECIDO("[OK] chamada de funcao reconhecida");
          $$.numero = NAN;
          $$.ast = NULL;
      }
    ;

args_opt:
      %empty
    | args
    ;

args:
      expr { semantico_registrar_expressao($1.ast, 0); }
    | args COMMA expr { semantico_registrar_expressao($3.ast, 0); }
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

    NoAST *raiz = semantico_ast();
    if (raiz) ast_imprimir(raiz, 0);
    int erros_semanticos = semantico_analisar(raiz);

    semantico_liberar(raiz);
    ast_liberar(raiz);
    lexer_liberar_textos();
    return status != 0 || erros_semanticos != 0 ||
           erros_sintaticos != 0 || erros_lexicos != 0;
}
