# Analisador léxico

O scanner em `Lexico/scanner.l` transforma o código de entrada em tokens para o parser.
A [organização dos fontes](../src.md) descreve como ele participa do build.

| Token | Lexema / expressão regular |
|---|---|
| NUM | `[0-9]+(\.[0-9]+)?` |
| ID | `[a-zA-Z_][a-zA-Z0-9_]*` |
| STRING | aspas simples ou duplas, escapes com `\`, sem quebra de linha |
| TRUE, FALSE, NONE | `True`, `False`, `None` |
| PLUS, MINUS, TIMES, DIVIDE | `+`, `-`, `*`, `/` |
| FLOORDIV, MOD, POWER | `//`, `%`, `**` |
| LPAREN, RPAREN, COMMA | `(`, `)`, `,` |
| EQ, NE, LT, GT, LE, GE | `==`, `!=`, `<`, `>`, `<=`, `>=` |
| AND, OR, NOT | `and`, `or`, `not` |
| IF, ELIF, ELSE | `if`, `elif`, `else` |
| WHILE, FOR, IN, BREAK, CONTINUE | `while`, `for`, `in`, `break`, `continue` |
| LBRACE, RBRACE, SEMICOLON | `{`, `}`, `;` |
| NEWLINE | `\n` significativo ou terminador sintetizado antes de `}` / EOF |
| ASSIGN, PLUSEQ, MINUSEQ, TIMESEQ, DIVEQ | `=`, `+=`, `-=`, `*=`, `/=` |
| LBRACKET, RBRACKET | `[`, `]` |
| DEF, RETURN | `def`, `return` |
| ignorado | `#.*` (comentário); `[ \t\r]+` (espaços) |
| YYUNDEF | `.` (caractere não reconhecido) |

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

## Inspecionar os tokens

```sh
make tokens
./tokens < testes/validos/base_programa_referencia.txt
./tokens < testes/validos/base_catalogo_tokens.txt
```

`SO_TOKENS` compila o mesmo scanner com um `main` que imprime `TOKEN(lexema)`;
não precisa vincular o parser. `make test` compara essas saídas com
`*.tokens.esperado` e também valida o caractere inválido `@`.

## Textos, números e execução

O scanner reconhece números inteiros ou decimais e os entrega como `double`.
O parser preserva o literal na AST; o interpretador decide sua representação em
tempo de execução. O sinal de um número é o operador unário `MINUS`.

Os textos de identificadores e strings são copiados pelo scanner. A AST guarda
suas próprias cópias; as funções de limpeza liberam cada estrutura separadamente.
Os escapes de strings chegam à AST como barra e caractere e são tratados pelo
interpretador, incluindo `\n`, `\t`, `\\`, `\"` e `\'`.

Esta etapa não executa comandos nem escolhe ramos de condições. Um erro léxico
impede a execução do programa, mesmo se o parser conseguir recuperar o restante
da entrada. Veja também [análise sintática](../analisador_sintatico/analisador-sintatico.md)
e [interpretador](../interpretador.md).
