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

/* ===== [P5] Raiz da AST e encadeamento de elif/else ===== */
/* Raiz AST_PROGRAMA montada ao fim da analise; o main imprime e analisa. */
static NoAST *raiz_programa = NULL;
/* Liga um elif (outro IF) ou um else (BLOCO) ao fim de uma cadeia de IFs. */
static NoAST *anexar_senao(NoAST *se, NoAST *senao);
/* ===== FIM [P5] Raiz da AST e encadeamento de elif/else ===== */
%}

%locations
/* Os dois conflitos intencionais sao ELSE do if versus ELSE sem if. */
%expect 2

%union {
    double numero;
    char *texto;

    ValorExpressao expr;
    NoAST *no;            /* [P5] comandos, blocos e listas de nos */
    TipoNo operador;      /* [P4] operador de atribuicao (=, +=, ...) */
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
%type <expr> expr chamada_funcao
%destructor { ast_liberar($$.ast); } <expr>

/* [P5] Todo comando devolve um no da AST; listas usam o campo prox. */
%type <no> programa elemento bloco comando
%type <no> comando_if lista_elif                               /* P2 */
%type <no> laco_while laco_for break_stmt continue_stmt        /* P3 */
%type <no> atribuicao alvo lista elementos indexacao           /* P4 */
%type <no> def_funcao params_opt params comando_return args_opt args /* P5 */
%type <operador> op_atrib
/* Nos descartados pela recuperacao de erros sao liberados aqui. */
%destructor { ast_liberar($$); } <no>

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
%start entrada

%%

/* ===== ESTRUTURA GERAL DE EXECUCAO ===== */
/* [P5] A raiz: todos os comandos do arquivo viram filhos de um AST_PROGRAMA. */
entrada:
      programa {
          if ($1) raiz_programa = ast_criar(AST_PROGRAMA, $1, NULL, NULL, $1->linha);
      }
    ;

/* [P5] Sequencia de comandos encadeada pelo prox (linhas vazias viram NULL). */
programa:
      %empty { $$ = NULL; }
    | programa elemento { $$ = ast_anexar($1, $2); }
    ;

elemento:
      comando { $$ = $1; }
    | NEWLINE { $$ = NULL; }
    ;

comando:
      laco_while                   /* P3 */
    | laco_for                     /* P3 */
    | break_stmt                   /* P3 */
    | continue_stmt                /* P3 */
    | atribuicao terminador { $$ = $1; }  /* P4 */
    | expr terminador  {
          emitir_pendente();
          $$ = $1.ast;
      } /* [P1] A expressao vira um comando da AST; a analise ocorre no fim. */
    ;

/* [P5] Um bloco { ... } vira um AST_BLOCO com os comandos internos. */
bloco:
      LBRACE inicio_bloco programa RBRACE {
          $$ = ast_criar(AST_BLOCO, $3, NULL, NULL, @1.first_line);
      }
    | LBRACE inicio_bloco programa error FIM {
          ERRO_SINTATICO(@1, "bloco aberto sem '}'");
          yyerrok;
          ast_liberar($3);
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
    error terminador { emitir_pendente(); yyerrok; $$ = NULL; }
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
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero == $3.numero);
          $$.ast = ast_criar(AST_IGUAL, $1.ast, $3.ast, NULL, @2.first_line);
      }
    | expr NE expr {
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero != $3.numero);
          $$.ast = ast_criar(AST_DIFERENTE, $1.ast, $3.ast, NULL, @2.first_line);
      }
    | expr LT expr {
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero < $3.numero);
          $$.ast = ast_criar(AST_MENOR, $1.ast, $3.ast, NULL, @2.first_line);
      }
    | expr GT expr {
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero > $3.numero);
          $$.ast = ast_criar(AST_MAIOR, $1.ast, $3.ast, NULL, @2.first_line);
      }
    | expr LE expr {
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero <= $3.numero);
          $$.ast = ast_criar(AST_MENOR_IGUAL, $1.ast, $3.ast, NULL, @2.first_line);
      }
    | expr GE expr {
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero >= $3.numero);
          $$.ast = ast_criar(AST_MAIOR_IGUAL, $1.ast, $3.ast, NULL, @2.first_line);
      }
    | expr AND expr {
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero != 0 && $3.numero != 0);
          $$.ast = ast_criar(AST_E, $1.ast, $3.ast, NULL, @2.first_line);
      }
    | expr OR expr {
          $$.numero = isnan($1.numero) || isnan($3.numero) ? NAN : ($1.numero != 0 || $3.numero != 0);
          $$.ast = ast_criar(AST_OU, $1.ast, $3.ast, NULL, @2.first_line);
      }
    | NOT expr {
          $$.numero = isnan($2.numero) ? NAN : ($2.numero == 0);
          $$.ast = ast_criar(AST_NAO, $2.ast, NULL, NULL, @1.first_line);
      }
    ;

/* Comandos condicionais. */
comando:
      comando_if
    ;

/* [P5/integracao] AST_IF: a = condicao, b = bloco, c = elif (outro IF) ou else (BLOCO). */
comando_if:
      IF expr bloco {
          $$ = ast_criar(AST_IF, $2.ast, $3, NULL, @1.first_line);
          RECONHECIDO("[OK] if reconhecido");
      }
    | IF expr bloco ELSE bloco {
          $$ = ast_criar(AST_IF, $2.ast, $3, $5, @1.first_line);
          RECONHECIDO("[OK] if reconhecido");
      }
    | IF expr bloco lista_elif {
          $$ = ast_criar(AST_IF, $2.ast, $3, $4, @1.first_line);
          RECONHECIDO("[OK] if reconhecido");
      }
    | IF expr bloco lista_elif ELSE bloco {
          $$ = ast_criar(AST_IF, $2.ast, $3, anexar_senao($4, $6), @1.first_line);
          RECONHECIDO("[OK] if reconhecido");
      }
    | IF { ERRO_SINTATICO(@1, "condicao vazia no if"); } bloco { ast_liberar($3); $$ = NULL; }
    | IF error {
          preparar_diagnostico(@1.first_line, @1.first_column, "condicao invalida no if");
      } bloco { yyerrok; ast_liberar($4); $$ = NULL; }
    /* Nao aceitar IF expr error: interceptava expressoes incompletas e
- confundia a abertura do bloco com a ausencia de bloco. */
    | IF error terminador {
          ERRO_SINTATICO(@1, "bloco ausente no if (esperado '{')"); yyerrok;
          $$ = NULL;
      }
    | ELSE bloco { ERRO_SINTATICO(@1, "else sem if correspondente"); ast_liberar($2); $$ = NULL; }
    ;

/* Cada elif vira um AST_IF pendurado no filho c do anterior. */
lista_elif:
      ELIF expr bloco { $$ = ast_criar(AST_IF, $2.ast, $3, NULL, @1.first_line); }
    | lista_elif ELIF expr bloco {
          $$ = anexar_senao($1, ast_criar(AST_IF, $3.ast, $4, NULL, @2.first_line));
      }
    | ELIF { ERRO_SINTATICO(@1, "condicao vazia no elif"); } bloco { ast_liberar($3); $$ = NULL; }
    | ELIF error {
          preparar_diagnostico(@1.first_line, @1.first_column, "condicao invalida no elif");
      } bloco { yyerrok; ast_liberar($4); $$ = NULL; }
    | ELIF error terminador {
          ERRO_SINTATICO(@1, "bloco ausente no elif (esperado '{')"); yyerrok;
          $$ = NULL;
      }
    ;

/* ===== FIM [P2] CONDICOES E LOGICA ===== */


/* ===== [P3] LACOS ===== */
/* [P5/integracao] As regras com recuperacao de erro passaram a gerar os nos
 * AST_WHILE (a = condicao, b = corpo) e AST_FOR (texto = variavel,
 * a = iteravel, b = corpo). As copias sem recuperacao foram unificadas aqui. */
laco_while:
      WHILE expr bloco {
          $$ = ast_criar(AST_WHILE, $2.ast, $3, NULL, @1.first_line);
          RECONHECIDO("[OK] while reconhecido");
      }
    | WHILE { ERRO_SINTATICO(@1, "condicao vazia no while"); } bloco { ast_liberar($3); $$ = NULL; }
    | WHILE error {
          preparar_diagnostico(@1.first_line, @1.first_column, "condicao invalida no while");
      } bloco { yyerrok; ast_liberar($4); $$ = NULL; }
    ;

laco_for:
      FOR ID IN expr bloco {
          /* O texto de ID pertence ao scanner; ast_criar_texto guarda uma copia. */
          $$ = ast_criar_texto(AST_FOR, $2, @1.first_line);
          if ($$) { $$->a = $4.ast; $$->b = $5; }
          else { ast_liberar($4.ast); ast_liberar($5); }
          RECONHECIDO("[OK] for reconhecido");
      }
    | FOR ID error {
          preparar_diagnostico(@1.first_line, @1.first_column, "for sem a palavra 'in' ou expressao iteravel invalida");
      } bloco { yyerrok; ast_liberar($5); $$ = NULL; }
    | FOR error {
          preparar_diagnostico(@1.first_line, @1.first_column, "for sem variavel de controle");
      } bloco { yyerrok; ast_liberar($4); $$ = NULL; }
    ;

break_stmt:
    BREAK terminador {
        $$ = ast_criar(AST_BREAK, NULL, NULL, NULL, @1.first_line);
        RECONHECIDO("[OK] break reconhecido");
    }
;

continue_stmt:
    CONTINUE terminador {
        $$ = ast_criar(AST_CONTINUE, NULL, NULL, NULL, @1.first_line);
        RECONHECIDO("[OK] continue reconhecido");
    }
;
/* ===== [P4] ATRIBUICAO, LISTAS E COMENTARIOS ===== */
/* Atribuicoes, listas e indexacao. Comentarios sao ignorados no scanner. */
/* [P5/integracao] As acoes AST que a P4 deixou preparadas (comentadas) foram
 * ativadas: as regras agora devolvem nos via ast_p4_atribuicao/lista/indexacao. */

/* Atribuicao simples e composta: x = 1, x += 1, l[0] = 1 */
atribuicao:
      alvo op_atrib expr {
          $$ = ast_p4_atribuicao($2, $1, $3.ast, @1.first_line);
          RECONHECIDO("[OK] atribuicao reconhecida");
      }
    | literal op_atrib expr {
        ast_liberar($3.ast);
        ERRO_SINTATICO(@1, "atribuicao invalida: o lado esquerdo deve ser variavel ou l[i]");
        yyerrok;
        $$ = NULL;
    }
    ;

op_atrib:
      ASSIGN  { $$ = AST_ATRIBUICAO; }
    | PLUSEQ  { $$ = AST_ATRIB_SOMA; }
    | MINUSEQ { $$ = AST_ATRIB_SUB; }
    | TIMESEQ { $$ = AST_ATRIB_MULT; }
    | DIVEQ   { $$ = AST_ATRIB_DIV; }
    ;

/* Alvo valido: variavel ou posicao de lista. */
alvo:
      ID        { $$ = ast_criar_texto(AST_IDENTIFICADOR, $1, @1.first_line); }
    | indexacao { $$ = $1; }
    ;

/* Literais que nao podem ficar a esquerda de "=" (ex.: 3 = x). */
literal:
      NUM | STRING | TRUE | FALSE | NONE
    ;

/* Lista [a, b]. Como STRING e NONE, seu valor numerico e NAN. */
expr:
      lista       { $$.numero = NAN; $$.ast = $1; }
    | indexacao   { $$.numero = NAN; $$.ast = $1; }
    ;

lista:
      LBRACKET RBRACKET { $$ = ast_p4_lista(NULL, @1.first_line); }
    | LBRACKET elementos RBRACKET { $$ = ast_p4_lista($2, @1.first_line); }
    | LBRACKET elementos error {
        ast_liberar($2);
        ERRO_SINTATICO(@1, "lista mal formada: faltou ']' (ou ',' entre elementos)");
        sincronizar_lista();
        yyerrok;
        $$ = ast_p4_lista(NULL, @1.first_line);
    }
    | LBRACKET error {
        ERRO_SINTATICO(@1, "lista mal formada: faltou ']'");
        sincronizar_lista();
        yyerrok;
        $$ = ast_p4_lista(NULL, @1.first_line);
    }
    ;

elementos:
      expr { $$ = $1.ast; }
    | elementos COMMA expr { $$ = ast_anexar($1, $3.ast); }
    ;

/* Indexacao l[0]. */
indexacao:
      ID LBRACKET expr RBRACKET {
          $$ = ast_p4_indexacao(ast_criar_texto(AST_IDENTIFICADOR, $1, @1.first_line),
                                $3.ast, @1.first_line);
      }
    ;

/* ===== [P5] FUNCOES E CHAMADAS ===== */
/* Definicoes de funcoes, parametros, return e chamadas com argumentos.
 * Cada regra devolve um no da AST; a verificacao (nome existe? quantos
 * argumentos? return dentro de funcao?) fica para a analise semantica. */

comando:
      def_funcao
    | comando_return
    | ID LPAREN args_opt error terminador {
          ERRO_SINTATICO(@1, "chamada de funcao sem fecha parenteses ')' antes do terminador");
          yyerrok;
          ast_liberar($3);
          $$ = NULL;
      }
    ;

/* def nome(p1, p2) { corpo }  ->  FUNCAO 'nome' (a = parametros, b = BLOCO) */
def_funcao:
      DEF ID LPAREN params_opt RPAREN bloco {
          $$ = ast_p5_funcao($2, $4, $6, @1.first_line);
          RECONHECIDO("[OK] def reconhecido");
      }
    | DEF error LPAREN params_opt RPAREN {
          ERRO_SINTATICO(@1, "definicao de funcao sem identificador (esperado nome antes de '(')");
          yyerrok;
      } bloco { ast_liberar($4); ast_liberar($7); $$ = NULL; }
    | DEF error {
          preparar_diagnostico(@1.first_line, @1.first_column, "definicao de funcao malformada (esperado nome e parametros antes do bloco)");
      } bloco { yyerrok; ast_liberar($4); $$ = NULL; }
    | DEF ID LPAREN error RPAREN {
          ERRO_SINTATICO(@1, "parametros mal formados na definicao de funcao. Recuperado apos ')'.");
          yyerrok;
      } bloco { ast_liberar($7); $$ = NULL; }
    ;

params_opt:
      %empty { $$ = NULL; }
    | params { $$ = $1; }
    ;

/* Parametros viram nos PARAMETRO encadeados pelo prox, na ordem do codigo. */
params:
      ID { $$ = ast_criar_texto(AST_PARAMETRO, $1, @1.first_line); }
    | params COMMA ID {
          $$ = ast_anexar($1, ast_criar_texto(AST_PARAMETRO, $3, @3.first_line));
      }
    ;

/* return expr;  ->  RETURN (a = expr);   return;  ->  RETURN sem filho */
comando_return:
      RETURN expr terminador {
          $$ = ast_criar(AST_RETURN, $2.ast, NULL, NULL, @1.first_line);
          RECONHECIDO("[OK] return reconhecido");
      }
    | RETURN terminador {
          $$ = ast_criar(AST_RETURN, NULL, NULL, NULL, @1.first_line);
          RECONHECIDO("[OK] return reconhecido");
      }
    | RETURN ID LPAREN args_opt error terminador {
          ERRO_SINTATICO(@1, "chamada de funcao sem fecha parenteses ')' no return antes do terminador");
          yyerrok;
          ast_liberar($4);
          $$ = NULL;
      }
    ;

expr:
      chamada_funcao { $$ = $1; }
    ;

/* nome(a1, a2)  ->  CHAMADA 'nome' (a = argumentos). print e range sao ID. */
chamada_funcao:
      ID LPAREN args_opt RPAREN {
          RECONHECIDO("[OK] chamada de funcao reconhecida");
          $$.numero = NAN;
          $$.ast = ast_p5_chamada($1, $3, @1.first_line);
      }
    ;

args_opt:
      %empty { $$ = NULL; }
    | args { $$ = $1; }
    ;

/* Argumentos sao expressoes quaisquer, encadeadas pelo prox. */
args:
      expr { $$ = $1.ast; }
    | args COMMA expr { $$ = ast_anexar($1, $3.ast); }
    ;

/* ===== FIM [P5] FUNCOES E CHAMADAS ===== */

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

/* ===== [P5] Encadeamento de elif/else e fluxo principal ===== */
/* Desce pelos filhos c da cadeia de IFs e pendura "senao" no ultimo. */
static NoAST *anexar_senao(NoAST *se, NoAST *senao) {
    if (!se) return senao;
    NoAST *ultimo = se;
    while (ultimo->c && ultimo->c->tipo == AST_IF) ultimo = ultimo->c;
    if (ultimo->c) ast_liberar(senao);   /* nao deve ocorrer: ja ha um else */
    else ultimo->c = senao;
    return se;
}

/* Fluxo do interpretador (Semana 6):
 * 1. yyparse() le stdin e monta a AST (com recuperacao de erros sintaticos);
 * 2. a AST do programa e impressa;
 * 3. a analise semantica percorre a AST e usa a tabela de simbolos;
 * 4. AST, tabela e textos do scanner sao liberados.
 * O codigo de saida e 1 se houve qualquer erro. */
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int status = yyparse();
    emitir_pendente();

    if (raiz_programa) ast_imprimir(raiz_programa, 0);

    /* Roda mesmo apos erros sintaticos (como ja fazia a P1), analisando os
     * comandos que a recuperacao preservou. Limitacao: um def descartado
     * pela recuperacao nao entra na AST, entao suas chamadas sao relatadas
     * como "funcao nao definida". */
    int erros_semanticos = semantico_analisar(raiz_programa);

    ast_liberar(raiz_programa);
    lexer_liberar_textos();
    return status != 0 || erros_semanticos != 0 ||
           erros_sintaticos != 0 || erros_lexicos != 0;
}
/* ===== FIM [P5] Encadeamento de elif/else e fluxo principal ===== */
