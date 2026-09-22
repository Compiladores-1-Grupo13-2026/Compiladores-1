 /* scanner.l - Analisador lexico (Flex) */

 /* opcoes do Flex: sem yywrap, conta linhas (yylineno) */
%option noyywrap yylineno nounput noinput

%{
#include <stdio.h>
#include <stdlib.h>
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



 /* =========== PARTE 1: EXPRESSOES E LITERAIS =========== */

 /* Literais MiniPy: aspas simples ou duplas, com escapes e sem quebra de linha. */
\"([^\"\\\n]|\\[^\n])*\"    { return STRING; }
\'([^\'\\\n]|\\[^\n])*\'    { return STRING; }
"True"      { return TRUE; }
"False"     { return FALSE; }
"None"      { return NONE; }

 /* Operadores da P1, antes das regras genericas da base. */
"//"        { return FLOORDIV; }
"%"         { return MOD; }
"**"        { return POWER; }


 /* ===== [P2] CONDIÇÕES E LÓGICA ===== */
"=="        { return EQ; }
"!="        { return NE; }
"<="        { return LE; }
">="        { return GE; }
"<"         { return LT; }
">"         { return GT; }

"and"       { return AND; }
"or"        { return OR; }
"not"       { return NOT; }

"if"        { return IF; }
"elif"      { return ELIF; }
"else"      { return ELSE; }

 /* =========== PARTE 3: Laços =========== */
 /* Laços: while, for, in, break, continue */
"while"                 { return WHILE; }
"for"                   { return FOR; }
"in"                    { return IN; }
"break"                 { return BREAK; }
"continue"              { return CONTINUE; }

 /* Delimitadores: ; { } */
";"                     { return SEMICOLON; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }

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

 /* =========== PARTE 5: FUNÇÕES E CHAMADAS =========== */
"def"                   { return DEF; }
"return"                { return RETURN; }

 /* ======================== BASE ======================== */
 /* Operadores e delimitadores basicos */
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return TIMES; }
"/"                     { return DIVIDE; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
","                     { return COMMA; }

 /* Literais numericos: inteiros ou decimais */
[0-9]+(\.[0-9]+)?       { yylval.numero = atof(yytext); return NUM; }

 /* Identificador: letra ou "_" seguido de letras, digitos ou "_".
  * Posicionado apos as palavras reservadas (P1 a P5) para evitar que
  * palavras-chave como def ou return sejam classificadas como ID. */
[a-zA-Z_][a-zA-Z0-9_]*  { return ID; }

 /* ======================== ERRO ======================== */
 /* Caractere que nenhuma regra acima reconheceu. Deve ser o ultimo bloco. */
.           {
    fprintf(stderr, "Erro lexico na linha %d: caractere invalido '%s'\n", yylineno, yytext);
    return YYUNDEF;
}

%%

/* Imprime os tokens somente no modo make tokens. */
#ifdef SO_TOKENS
YYSTYPE yylval;

int main(void)
{
    int token;
    while ((token = yylex()) != 0) {
        const char *nome;
        switch (token) {
#define NOME_TOKEN(t) case t: nome = #t; break
            NOME_TOKEN(NUM); NOME_TOKEN(ID);
            NOME_TOKEN(PLUS); NOME_TOKEN(MINUS);
            NOME_TOKEN(TIMES); NOME_TOKEN(DIVIDE);
            NOME_TOKEN(LPAREN); NOME_TOKEN(RPAREN);
            NOME_TOKEN(LBRACE); NOME_TOKEN(RBRACE);
            NOME_TOKEN(COMMA); NOME_TOKEN(SEMICOLON); NOME_TOKEN(NEWLINE);
            NOME_TOKEN(STRING); NOME_TOKEN(TRUE); NOME_TOKEN(FALSE);
            NOME_TOKEN(NONE); NOME_TOKEN(FLOORDIV); NOME_TOKEN(MOD);
            NOME_TOKEN(POWER);
            NOME_TOKEN(EQ); NOME_TOKEN(NE); NOME_TOKEN(LT); NOME_TOKEN(GT);
            NOME_TOKEN(LE); NOME_TOKEN(GE); NOME_TOKEN(AND); NOME_TOKEN(OR);
            NOME_TOKEN(NOT); NOME_TOKEN(IF); NOME_TOKEN(ELIF); NOME_TOKEN(ELSE);
            NOME_TOKEN(WHILE); NOME_TOKEN(FOR); NOME_TOKEN(IN);
            NOME_TOKEN(BREAK); NOME_TOKEN(CONTINUE);
            NOME_TOKEN(ASSIGN); NOME_TOKEN(PLUSEQ); NOME_TOKEN(MINUSEQ);
            NOME_TOKEN(TIMESEQ); NOME_TOKEN(DIVEQ);
            NOME_TOKEN(LBRACKET); NOME_TOKEN(RBRACKET);
            NOME_TOKEN(DEF); NOME_TOKEN(RETURN);
            case YYUNDEF: nome = "CARACTERE_INVALIDO"; break;
#undef NOME_TOKEN
            default: nome = "TOKEN_DESCONHECIDO"; break;
        }
        printf("%s(%s)\n", nome, token == NEWLINE ? "\\n" : yytext);
    }
    return 0;
}
#endif
