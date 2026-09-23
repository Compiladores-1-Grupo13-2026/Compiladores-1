# Analisador léxico

Fonte: [Lexico/scanner.l](https://github.com/Compiladores-1-Grupo13-2026/Compiladores-1/blob/main/Lexico/scanner.l).

| Token | Lexema / expressão regular | Fatia |
|---|---|---|
| NUM | `[0-9]+(\.[0-9]+)?` | base / P1 |
| ID | `[a-zA-Z_][a-zA-Z0-9_]*` | base / todas |
| STRING | aspas simples ou duplas, escapes com `\`, sem quebra de linha | P1 |
| TRUE, FALSE, NONE | `True`, `False`, `None` | P1 |
| PLUS, MINUS, TIMES, DIVIDE | `+`, `-`, `*`, `/` | base / P1 |
| FLOORDIV, MOD, POWER | `//`, `%`, `**` | P1 |
| LPAREN, RPAREN, COMMA | `(`, `)`, `,` | base / P1 e P5 |
| EQ, NE, LT, GT, LE, GE | `==`, `!=`, `<`, `>`, `<=`, `>=` | P2 |
| AND, OR, NOT | `and`, `or`, `not` | P2 |
| IF, ELIF, ELSE | `if`, `elif`, `else` | P2 |
| WHILE, FOR, IN, BREAK, CONTINUE | `while`, `for`, `in`, `break`, `continue` | P3 |
| LBRACE, RBRACE, SEMICOLON | `{`, `}`, `;` | base / P3 |
| NEWLINE | `\n` significativo ou terminador sintetizado antes de `}` / EOF | base / P2 e P3 |
| ASSIGN, PLUSEQ, MINUSEQ, TIMESEQ, DIVEQ | `=`, `+=`, `-=`, `*=`, `/=` | P4 |
| LBRACKET, RBRACKET | `[`, `]` | P4 |
| DEF, RETURN | `def`, `return` | P5 |
| ignorado | `#.*` (comentário); `[ \t\r]+` (espaços) | P4 / base |
| YYUNDEF | `.` (caractere não reconhecido) | erro léxico global |

Catálogo extraído de `Lexico/scanner.l`. A regra de STRING é
`\"([^\"\\\n]|\\[^\n])*\"` para aspas duplas; a versão com aspas simples é análoga.
`FIM`, de código 0, representa EOF internamente; não é palavra reservada.
`print` e `range` são `ID`. O modo tokens exibe `CARACTERE_INVALIDO` para `YYUNDEF`.

## Decisões do scanner

As palavras reservadas vêm antes de ID; Flex escolhe o maior lexema e, em empate,
a primeira regra. `NUM` preenche `yylval.numero` antes de retornar seu token.
O cabeçalho `parser.tab.h` compartilha tokens, `YYSTYPE` e `YYLTYPE` com o parser.

`linha_tem_token` é marcado apenas quando se retorna um token. Espaços e
comentários não o marcam; linhas vazias não geram `NEWLINE`. O EOF sintetiza
o terminador de um último comando sem quebra de linha. Antes de `}`, `yyless(0)`
devolve primeiro `NEWLINE` quando há comando simples pendente; `{`, `}` e `;`
já delimitados dispensam essa inserção. Isso aceita blocos numa só linha.

`aninhamento` conta colchetes abertos. Dentro de listas, as quebras de linha são
ignoradas. Para recuperar um `]` ausente, uma atribuição no início da próxima
linha devolve o `NEWLINE` suprimido com `yyless(0)`. A mesma técnica preserva
um novo operando em linha seguinte quando o anterior já terminou e faltou vírgula;
`and` e `or` continuam permitidos. Fora desse contexto, `REJECT` deixa as regras
normais reconhecerem o texto. Essas heurísticas não substituem a gramática.
`;`, a recuperação do parser e o EOF encerram o aninhamento inválido.

`YY_USER_ACTION` preenche `yylloc`. Para a regra de `\n`, Flex já avançou
`yylineno`, então a localização usa `yylineno - 1`; a coluna recomeça em 1.
Antes de `REJECT` ou `yyless(0)`, restaura-se a coluna para não contar os mesmos
bytes duas vezes. O terminador de uma lista incompleta aponta o fim do token
anterior, sem consumir o começo do comando seguinte.

A regra coringa `.` é a última: imprime `[ERRO LEXICO] Linha N`, incrementa
`erros_lexicos` e retorna `YYUNDEF`. O modo tokens também retorna 1 se houve
caractere inválido; não o ignora silenciosamente.

## Evidência da Semana 3

```sh
make tokens
./tokens < testes/validos/base_programa_referencia.txt
./tokens < testes/validos/base_catalogo_tokens.txt
```

`SO_TOKENS` compila o mesmo scanner com um `main` que imprime `TOKEN(lexema)`;
não precisa vincular o parser. `make test` compara essas saídas com
`*.tokens.esperado` e também valida o caractere inválido `@`.
