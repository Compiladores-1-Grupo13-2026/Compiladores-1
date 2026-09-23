# Analisador sintático

Fonte: [Sintatica/parser.y](https://github.com/Compiladores-1-Grupo13-2026/Compiladores-1/blob/main/Sintatica/parser.y).

## Gramática formal (EBNF)

```ebnf
programa        = { elemento } ;
elemento        = comando | NEWLINE ;
comando         = expr terminador | atribuicao terminador
                | comando_break terminador | comando_continue terminador
                | comando_return | comando_if | laco_while | laco_for | def_funcao ;
bloco           = LBRACE programa RBRACE ;
terminador      = SEMICOLON | NEWLINE ;
expr            = NUM | STRING | TRUE | FALSE | NONE | ID
                | LPAREN expr RPAREN | MINUS expr | NOT expr
                | expr operador expr | lista | indexacao | chamada_funcao ;
operador        = PLUS | MINUS | TIMES | DIVIDE | FLOORDIV | MOD | POWER
                | EQ | NE | LT | GT | LE | GE | AND | OR ;
comando_if      = IF expr bloco [ lista_elif ] [ ELSE bloco ] ;
lista_elif      = ELIF expr bloco { ELIF expr bloco } ;
laco_while      = WHILE expr bloco ;
laco_for        = FOR ID IN expr bloco ;
comando_break   = BREAK ;
comando_continue = CONTINUE ;
atribuicao      = alvo op_atrib expr ;
alvo            = ID | indexacao ;
op_atrib        = ASSIGN | PLUSEQ | MINUSEQ | TIMESEQ | DIVEQ ;
lista           = LBRACKET [ elementos ] RBRACKET ;
elementos       = expr { COMMA expr } ;
indexacao       = ID LBRACKET expr RBRACKET ;
def_funcao      = DEF ID LPAREN params_opt RPAREN bloco ;
params_opt      = [ params ] ;
params          = ID { COMMA ID } ;
comando_return  = RETURN [ expr ] terminador ;
chamada_funcao  = ID LPAREN args_opt RPAREN ;
args_opt        = [ args ] ;
args            = expr { COMMA expr } ;
```

EBNF extraída das produções válidas de `Sintatica/parser.y`: `{ ... }` indica
repetição, `[ ... ]` indica opção e os nomes em maiúsculas são tokens.
As ações C e as produções de recuperação não definem novos programas válidos.
No parser, `inicio_bloco` é vazio e apenas conclui a recuperação de cabeçalhos;
`literal = NUM | STRING | TRUE | FALSE | NONE` serve para diagnosticar alvos inválidos.

## Precedência

Da menor para a maior precedência, preservando o contrato da Sprint 0:

| Nível | Tokens | Associação |
|---|---|---|
| 1 | OR | esquerda |
| 2 | AND | esquerda |
| 3 | NOT | precedência unária |
| 4 | EQ NE LT GT LE GE | esquerda |
| 5 | PLUS MINUS | esquerda |
| 6 | TIMES DIVIDE FLOORDIV MOD | esquerda |
| 7 | UMINUS | precedência unária |
| 8 | POWER | direita |

`UMINUS` é um marcador de `%prec`, não um token retornado pelo scanner.
`%precedence NOT` e `%precedence UMINUS` substituem `%right` na mesma posição:
não alteram a ordem. Exemplos: `-2 ** 2` resulta em `-4`, `2 ** 3 ** 2` em `512`
e `not 1 == 2` em `1`. Comparações encadeadas associam à esquerda; não têm
a semântica especial de comparações encadeadas do Python.

## Conflitos analisados

Há exatamente **dois conflitos shift/reduce intencionais em `ELSE`**,
declarados por `%expect 2`. Depois de `IF expr bloco`, com ou sem `lista_elif`,
o parser pode anexar o `ELSE` ao `if` (shift) ou encerrar esse comando e tratar
`ELSE bloco` como o erro de `else` órfão (reduce). O shift é a escolha desejada.
As chaves eliminam o *dangling else* entre blocos válidos; os conflitos restantes
são introduzidos pela produção de diagnóstico do `else` órfão.

`parser.output` é sempre gerado por `make`. Os estados atuais são 107 e 149;
os números podem mudar com a gramática, mas os dois devem continuar em `ELSE`.
A duplicação de `expr: NUM` foi removida e não há conflitos reduce/reduce.

O build padrão usa `-Wall -Werror -v`. `-Wcounterexamples` é reservado à
investigação: ele emite warnings inclusive para conflitos cobertos por `%expect`,
por isso não fica no build que deve passar sem warnings. Para inspecionar:

```sh
bison -Wall -Wcounterexamples -v -d -o /tmp/minipy-auditoria.c Sintatica/parser.y
```

Nessa auditoria, os dois avisos de contraexemplo são esperados. No build normal,
qualquer quantidade diferente de dois conflitos falha por `%expect 2`.

## Diagnósticos e recuperação

A recuperação usa modo pânico: descarta a parte inválida até um terminador
(`;` ou `NEWLINE`), a abertura de um bloco (`{`), `)` ou `]`, conforme a produção.
`}` fecha o bloco e também é preservado como âncora da recuperação de listas.
Um bloco sem `}` é diagnosticado no EOF, apontando a localização da abertura.

`yyerror` conta a falha e guarda a localização e uma cópia do lexema detectado.
Uma produção específica substitui a mensagem genérica antes de emiti-la.
Erros sem produção específica mantêm `syntax error`. No caso léxico,
mantemos as duas mensagens: o scanner aponta o caractere e o parser aponta
o comando inválido. `\n` e `EOF` são apresentados de forma legível.

```text
[ERRO SINTATICO] Linha N, coluna C: mensagem (proximo a 'lexema')
[ERRO LEXICO] Linha N: caractere invalido 'c'
[ERRO SEMANTICO] Linha N: divisao por zero
```

Linhas e colunas começam em 1; colunas contam bytes, inclusive um byte por tab.
`%locations` registra a posição do token; diagnósticos específicos usam o início
do comando (ou do delimitador de lista/bloco). O lexema entre aspas é aquele
que revelou o erro, que pode estar adiante dessa posição.

O processo retorna 0 apenas sem erros; retorna 1 após erro léxico, sintático,
numérico ou falha de `yyparse`, mesmo quando conseguiu continuar.
`stdout` não tem buffer: `2>&1` preserva a ordem de resultados e diagnósticos.
`[OK]` registra reconhecimento de uma produção; pode aparecer dentro de um
comando recuperado e não substitui o código de saída do arquivo inteiro.

| Fatia / erro | Produção de recuperação (ações omitidas) | Sincronização |
|---|---|---|
| P1: parêntese ausente/extra, operando ausente | `comando: error terminador` | `;` / `NEWLINE` |
| P1: divisão por zero, overflow, potência fora do domínio | ação numérica chama `YYERROR`; recuperação geral | `;` / `NEWLINE` |
| P2: condição vazia | `IF bloco`, `ELIF bloco` | `{` e bloco |
| P2: condição inválida | `IF error bloco`, `ELIF error bloco` | `{` |
| P2: bloco ausente | `IF error terminador`, `ELIF error terminador` | `;` / `NEWLINE` |
| P2: else órfão | `ELSE bloco` | bloco |
| P3: condição vazia/inválida | `WHILE bloco`, `WHILE error bloco` | `{` |
| P3: falta de in / expressão iterável inválida | `FOR ID error bloco` | `{` |
| P3: falta de variável | `FOR error bloco` | `{` |
| P3: bloco sem fechamento | `LBRACE inicio_bloco programa error FIM` | EOF; posição de `{` |
| P4: alvo literal | `literal op_atrib expr` (diagnóstico explícito) | terminador do comando |
| P4: lista mal formada | `LBRACKET elementos error` ou `LBRACKET error` | `]`, `;`, `NEWLINE`, `}` ou EOF |
| P5: def sem nome | `DEF error LPAREN params_opt RPAREN bloco` | `(` / `)` |
| P5: cabeçalho de def malformado | `DEF error bloco` | `{` |
| P5: parâmetros malformados | `DEF ID LPAREN error RPAREN bloco` | `)` |
| P5: chamada sem fechamento | `ID LPAREN args_opt error terminador`, inclusive após RETURN | `;` / `NEWLINE` |

A recuperação de cabeçalhos só executa `yyerrok` depois de consumir a âncora;
antecipá-lo antes de `{` repetiria o erro sobre o mesmo token. `inicio_bloco`
emite o diagnóstico preparado antes de reconhecer o corpo. Para `if 1 + {`,
a regra antiga `IF expr error` foi substituída por `IF error terminador`, evitando
confundir uma expressão incompleta com ausência de bloco.

Na recuperação de bloco, exigir `FIM` evita um terceiro conflito em `NEWLINE`
que apareceria com `LBRACE programa error` genérico. A recuperação de listas
preserva o próximo terminador, consome um eventual `]` e zera o aninhamento
do scanner. Não usa `YYABORT` e não descarta o arquivo inteiro.

## Limites da etapa

- Sem AST, tabela de símbolos, tipos ou valores de variáveis. `Semantica/` e
  `src/main.c` são esqueletos para a Semana 6 em diante; não participam do build.
- Laços, funções, atribuições e chamadas não executam. Expressões numéricas
  são calculadas durante a análise, inclusive nos corpos de todos os ramos;
  isso não é execução de controle de fluxo nem avaliação com curto-circuito.
- ID, chamadas, strings, listas, indexações e `None` carregam `NAN`, que impede
  resultados numéricos inventados. Comparações e lógica propagam esse marcador.
- `break`/`continue` fora de laços e `return` fora de funções ainda são aceitos;
  validar o contexto será responsabilidade da semântica.
- `else`/`elif` em linha diferente do `}` anterior não são aceitos. Quebras de linha
  são ignoradas dentro de listas, mas não em parênteses de chamadas/expressões.
- Apenas stdin; não há argumento de caminho. A extensão do arquivo é livre;
  os testes usam `.txt` e a sintaxe com chaves não é Python padrão.
- A recuperação de listas usa heurísticas para reconhecer um novo comando após
  `]` ausente. Entradas ambíguas podem perder tokens até a próxima âncora explícita.
- Mensagens copiam até 127 bytes do lexema. Valores numéricos usam `double`.
