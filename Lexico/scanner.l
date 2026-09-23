 /* scanner.l - Analisador lexico (Flex) */

 /* opcoes do Flex: sem yywrap, conta linhas (yylineno) */
%option noyywrap yylineno nounput noinput

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"   /* nomes dos tokens, gerado pelo Bison */

/* P2: localizacao em bytes (tab ocupa uma coluna); Flex conta as linhas.
 * REJECT/yyless precisam desfazer o avanco da coluna antes da releitura. */
static int coluna = 1;
static int linha_tem_token = 0;
static int aninhamento = 0;
static int ultimo_token = 0;
static int ultima_linha = 1, ultima_coluna = 1;
int erros_lexicos = 0;

void encerrar_lista_invalida(void) { aninhamento = 0; }

#define YY_USER_ACTION do { \
    yylloc.first_line = yylineno - (yytext[0] == '\n'); \
    yylloc.last_line = yylloc.first_line; \
    yylloc.first_column = coluna; \
    yylloc.last_column = coluna + yyleng - 1; \
    coluna += yyleng; \
} while (0);

#define RETORNA(token) do { \
    linha_tem_token = 1; ultimo_token = (token); \
    ultima_linha = yylloc.last_line; ultima_coluna = yylloc.last_column + 1; \
    return (token); \
} while (0)

static int termina_expressao(void) {
    return ultimo_token == NUM || ultimo_token == ID || ultimo_token == STRING ||
           ultimo_token == TRUE || ultimo_token == FALSE || ultimo_token == NONE ||
           ultimo_token == RPAREN || ultimo_token == RBRACKET;
}
%}

%%
 /* ======================== GERAL ======================== */

 /* Espacos e tabs: separam tokens, nao geram token. */
[ \t\r]+    { }

 /* NEWLINE: marca o fim de um comando (fim da linha).
  * - Linha em branco ou so com comentario: nao gera NEWLINE.
  * - Dentro de [ ]: a quebra de linha e ignorada (lista em varias linhas). */
\n          {
    coluna = 1;
    if (linha_tem_token && aninhamento == 0) {
        linha_tem_token = 0;
        return NEWLINE;
    }
}

 /* EOF sintetiza um terminador tambem para o ultimo comando sem \n. */
<<EOF>>     {
    yylloc.first_line = yylloc.last_line = yylineno;
    yylloc.first_column = yylloc.last_column = coluna;
    if (linha_tem_token) {
        linha_tem_token = 0;
        aninhamento = 0;
        return NEWLINE;
    }
    return 0;
}

 /* =========== [P1] EXPRESSOES E LITERAIS =========== */

 /* Literais MiniPy: aspas simples ou duplas, com escapes e sem quebra de linha. */
\"([^\"\\\n]|\\[^\n])*\"    { RETORNA(STRING); }
\'([^\'\\\n]|\\[^\n])*\'    { RETORNA(STRING); }
"True"      { RETORNA(TRUE); }
"False"     { RETORNA(FALSE); }
"None"      { RETORNA(NONE); }

 /* Operadores da P1, antes das regras genericas da base. */
"//"        { RETORNA(FLOORDIV); }
"%"         { RETORNA(MOD); }
"**"        { RETORNA(POWER); }


 /* ===== [P2] CONDIÇÕES E LÓGICA ===== */
"=="        { RETORNA(EQ); }
"!="        { RETORNA(NE); }
"<="        { RETORNA(LE); }
">="        { RETORNA(GE); }
"<"         { RETORNA(LT); }
">"         { RETORNA(GT); }

"and"       { RETORNA(AND); }
"or"        { RETORNA(OR); }
"not"       { RETORNA(NOT); }

"if"        { RETORNA(IF); }
"elif"      { RETORNA(ELIF); }
"else"      { RETORNA(ELSE); }

 /* =========== [P3] LACOS =========== */
 /* Laços: while, for, in, break, continue */
"while"                 { RETORNA(WHILE); }
"for"                   { RETORNA(FOR); }
"in"                    { RETORNA(IN); }
"break"                 { RETORNA(BREAK); }
"continue"              { RETORNA(CONTINUE); }

 /* Delimitadores: ; { } */
";"                     { aninhamento = 0; RETORNA(SEMICOLON); }
"{"                     { RETORNA(LBRACE); }
"}"                     {
    if (linha_tem_token && ultimo_token != SEMICOLON &&
        ultimo_token != LBRACE && ultimo_token != RBRACE) {
        coluna = yylloc.first_column;
        yyless(0);
        linha_tem_token = 0;
        aninhamento = 0;
        return NEWLINE;
    }
    RETORNA(RBRACE);
}

 /* =========== [P4] ATRIBUICAO, LISTAS E COMENTARIOS =========== */
 /* Comentario: de "#" ate o fim da linha. Nao gera token. */
"#".*       { }

 /* Atribuicao composta: x += 1 */
"+="        { RETORNA(PLUSEQ); }
"-="        { RETORNA(MINUSEQ); }
"*="        { RETORNA(TIMESEQ); }
"/="        { RETORNA(DIVEQ); }

 /* Atribuicao: x = 1 */
"="         { RETORNA(ASSIGN); }

 /* Lista: colchetes de l[0] e [a, b] */
"["         { aninhamento++; RETORNA(LBRACKET); }
"]"         { if (aninhamento > 0) aninhamento--; RETORNA(RBRACKET); }

 /* Uma atribuicao na proxima linha nao pode continuar uma lista.
  * REJECT restaura a coluna: YY_USER_ACTION sera chamado novamente. */
^[ \t]*[a-zA-Z_][a-zA-Z0-9_]*[ \t]*(=[^=\n]|[-+*/]=)  {
    coluna = yylloc.first_column;
    if (aninhamento == 0) REJECT;
    aninhamento = 0;
    linha_tem_token = 0;
    yyless(0);
    yylloc.first_line = yylloc.last_line = ultima_linha;
    yylloc.first_column = yylloc.last_column = ultima_coluna;
    return NEWLINE;
}

 /* Dois operandos em linhas diferentes sem virgula nao podem continuar
  * a mesma lista. Devolve o terminador omitido e preserva o proximo comando.
  * and/or/not ficam fora: podem continuar uma expressao valida. */
^[ \t]*([0-9]+|[a-zA-Z_][a-zA-Z0-9_]*|[\"\']) {
    const char *inicio = yytext;
    while (*inicio == ' ' || *inicio == '\t') inicio++;
    coluna = yylloc.first_column;
    if (!aninhamento || !termina_expressao() ||
        !strcmp(inicio, "and") || !strcmp(inicio, "or")) REJECT;
    aninhamento = 0;
    linha_tem_token = 0;
    yyless(0);
    yylloc.first_line = yylloc.last_line = ultima_linha;
    yylloc.first_column = yylloc.last_column = ultima_coluna;
    return NEWLINE;
}

 /* =========== [P5] FUNCOES E CHAMADAS =========== */
"def"                   { RETORNA(DEF); }
"return"                { RETORNA(RETURN); }

 /* ======================== BASE ======================== */
 /* Operadores e delimitadores basicos */
"+"                     { RETORNA(PLUS); }
"-"                     { RETORNA(MINUS); }
"*"                     { RETORNA(TIMES); }
"/"                     { RETORNA(DIVIDE); }
"("                     { RETORNA(LPAREN); }
")"                     { RETORNA(RPAREN); }
","                     { RETORNA(COMMA); }

 /* Literais numericos: inteiros ou decimais */
[0-9]+(\.[0-9]+)?       { yylval.numero = atof(yytext); RETORNA(NUM); }

 /* Identificador: letra ou "_" seguido de letras, digitos ou "_".
  * Posicionado apos as palavras reservadas (P1 a P5) para evitar que
  * palavras-chave como def ou return sejam classificadas como ID. */
[a-zA-Z_][a-zA-Z0-9_]*  { RETORNA(ID); }

 /* ======================== ERRO ======================== */
 /* Caractere que nenhuma regra acima reconheceu. Deve ser o ultimo bloco. */
.           {
    fprintf(stderr, "[ERRO LEXICO] Linha %d: caractere invalido '%s'\n", yylloc.first_line, yytext);
    erros_lexicos++;
    RETORNA(YYUNDEF);
}

%%

/* Imprime os tokens somente no modo make tokens. */
#ifdef SO_TOKENS
YYSTYPE yylval;
YYLTYPE yylloc;

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
    return erros_lexicos != 0;
}
#endif
