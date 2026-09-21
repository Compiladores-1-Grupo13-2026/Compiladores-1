 /* scanner.l - Analisador lexico (Flex) */

 /* opcoes do Flex: sem yywrap, conta linhas (yylineno) */
%option noyywrap yylineno nounput noinput

%{
#include <stdio.h>
#include "parser.tab.h"   /* nomes dos tokens, gerado pelo Bison */

/* 1 se a linha atual ja teve algum token (usado pelo NEWLINE) */
static int linha_tem_token = 0;

/* quantos "[" estao abertos: dentro de [ ] a quebra de linha e ignorada */
static int aninhamento = 0;

/* Roda antes de TODA regra: marca que a linha teve token, sem contar
 * espacos, quebra de linha e comentario. Vale para as regras de todos. */
#define YY_USER_ACTION \
    if (yytext[0] != ' ' && yytext[0] != '\t' && yytext[0] != '\r' && \
        yytext[0] != '\n' && yytext[0] != '#') linha_tem_token = 1;
%}

%%
 /* ======================== GERAL ======================== */

 /* Espacos e tabs: separam tokens, nao geram token. */
[ \t\r]+    { }

 /* NEWLINE: marca o fim de um comando (fim da linha).
  * - Linha em branco ou so com comentario: nao gera NEWLINE.
  * - Dentro de [ ]: a quebra de linha e ignorada (lista em varias linhas). */
\n          {
    if (linha_tem_token && aninhamento == 0) {
        linha_tem_token = 0;
        return NEWLINE;
    }
}

 /* Fim do arquivo sem "\n" no final: fecha o ultimo comando com NEWLINE. */
<<EOF>>     {
    if (linha_tem_token) {
        linha_tem_token = 0;
        return NEWLINE;
    }
    return 0;
}



 /* =========== PARTE 1: NOME DO TEMA =========== */


 /* ===== [P2] CONDIÇÕES E LÓGICA - início ===== */

"=="        { TOK(EQ); }
"!="        { TOK(NE); }
"<="        { TOK(LE); }
">="        { TOK(GE); }
"<"         { TOK(LT); }
">"         { TOK(GT); }

"and"       { TOK(AND); }
"or"        { TOK(OR); }
"not"       { TOK(NOT); }

"if"        { TOK(IF); }
"elif"      { TOK(ELIF); }
"else"      { TOK(ELSE); }

 /* =========== PARTE 3: NOME DO TEMA =========== */


 /* =========== PARTE 4: ATRIBUICAO, LISTAS E COMENTARIOS =========== */
 /* Comentario: de "#" ate o fim da linha. Nao gera token. */
"#".*       { }

 /* Atribuicao composta: x += 1 */
"+="        { return PLUSEQ; }
"-="        { return MINUSEQ; }
"*="        { return TIMESEQ; }
"/="        { return DIVEQ; }

 /* Atribuicao: x = 1 */
"="         { return ASSIGN; }

 /* Lista: colchetes de l[0] e [a, b] */
"["         { aninhamento++; return LBRACKET; }
"]"         { if (aninhamento > 0) aninhamento--; return RBRACKET; }

 /* =========== PARTE N: NOME DO TEMA =========== */





 /* ======================== ERRO ======================== */

 /* Caractere que nenhuma regra acima reconheceu. Deve ser o ultimo bloco. */
.           { fprintf(stderr, "Erro lexico na linha %d: caractere invalido '%s'\n", yylineno, yytext); }

%%
