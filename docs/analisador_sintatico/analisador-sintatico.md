# Analisador sintático

O parser em `Sintatica/parser.y`, gerado pelo Bison, organiza os tokens do
scanner em uma [árvore sintática abstrata](../ast.md). Cada comando, expressão
e bloco produz nós que preservam a linha do código-fonte.

O programa passa então pela [análise semântica](../analisador_semantico/analisador-semantico.md).
O [interpretador](../interpretador.md) só é chamado quando não há erros de análise.

## Gramática formal

Na EBNF abaixo, `{ ... }` indica repetição, `[ ... ]` indica opção e os nomes em
maiúsculas são tokens. As chaves de um programa MiniPy são os tokens `LBRACE` e
`RBRACE`, não os marcadores de repetição da notação.

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

As produções de recuperação permitem continuar a análise de uma entrada
inválida para relatar mais erros. Elas não tornam essa entrada um programa válido.
Listas aceitam elementos em várias linhas, separados por vírgulas, sem vírgula
final. A indexação parte de um nome, como `lista[i]`; não aceita `lista[0][1]`
ou `[1, 2][0]`.

## Precedência e associação

Da menor para a maior precedência:

| Nível | Operadores | Associação |
| --- | --- | --- |
| 1 | `or` | Esquerda |
| 2 | `and` | Esquerda |
| 3 | `not` | Unária |
| 4 | `==`, `!=`, `<`, `>`, `<=`, `>=` | Esquerda |
| 5 | `+`, `-` | Esquerda |
| 6 | `*`, `/`, `//`, `%` | Esquerda |
| 7 | Menos unário | Unária |
| 8 | `**` | Direita |

Por exemplo, `print(-2 ** 2, 2 ** 3 ** 2, not 1 == 2)` imprime
`-4 512 True`. O marcador `UMINUS` controla a precedência no Bison e não é
um token do scanner. Comparações encadeadas associam à esquerda; não usam
a regra especial de comparações encadeadas do Python.

## Construção da AST e modos de uso

Literais, operadores, atribuições, condições, laços e funções são representados
na AST. Sequências de comandos, parâmetros, argumentos e elementos de listas
usam o encadeamento `prox`. Nós descartados na recuperação são liberados pelos
destrutores declarados na gramática e pelas ações de recuperação.

```sh
./parser < programa.txt
./parser --ast < programa.txt
```

No modo padrão, a saída contém os `print` do programa e os diagnósticos.
Expressões soltas não imprimem resultados. No modo `--ast`, o parser mostra
reconhecimentos e a árvore, sem executar o programa. A análise semântica ocorre
nos dois modos. Se houve erro sintático, ela ainda pode apontar problemas nos
comandos preservados pela recuperação.

## Conflitos da gramática

A declaração `%expect 2` registra dois conflitos shift/reduce intencionais em
`ELSE`. Depois de um `if`, com ou sem `elif`, o parser pode anexar o `else`
ao condicional ou tratá-lo como o diagnóstico de `else` sem `if`. A escolha
de shift anexa o ramo ao condicional correto.

O build usa Bison com `-Wall -Werror -v` e gera `parser.output`. A gramática
não possui conflitos reduce/reduce. Uma quantidade diferente dos dois conflitos
esperados faz a geração falhar.

## Diagnósticos e recuperação

O parser usa recuperação por âncoras: terminadores, delimitadores e início de
blocos. `yyerror` registra o erro e sua localização; uma produção específica
pode substituir a mensagem genérica antes de ela ser emitida.

```text
[ERRO SINTATICO] Linha N, coluna C: mensagem (proximo a 'lexema')
```

Linhas e colunas começam em 1. As colunas contam bytes; uma tabulação conta
uma coluna. `\n` e `EOF` identificam o fim de linha e o fim de arquivo nos
diagnósticos. A localização pode apontar o início do comando, enquanto o lexema
mostra o token que revelou a falha.

| Erro | Recuperação |
| --- | --- |
| Operando ou parêntese ausente | Avança até `;` ou quebra de linha |
| Condição vazia ou inválida em `if`/`elif` | Usa a abertura de bloco como âncora |
| Bloco ausente ou `else` sem `if` | Preserva o terminador ou o bloco reconhecido |
| Cabeçalho inválido de `while`/`for` | Recupera na abertura do corpo |
| Bloco sem fechamento | Diagnostica no EOF, apontando a abertura |
| Alvo literal de atribuição | Emite diagnóstico e preserva o próximo comando |
| Lista sem `]` ou vírgula | Sincroniza em `]`, `;`, quebra de linha, `}` ou EOF |
| Definição de função malformada | Recupera em parênteses ou na abertura do corpo |
| Chamada sem `)` | Recupera no terminador, inclusive em `return` |

Divisão por zero literal, tipos incompatíveis, nomes ausentes, aridade e uso
de `break`, `continue` e `return` fora de contexto são verificações semânticas.
Problemas que dependem dos valores efetivos são tratados na execução.

Um erro léxico, sintático ou semântico impede a execução inteira. O código de
saída é 1, inclusive quando a recuperação permitiu analisar comandos posteriores.
Sucesso retorna 0. Os reconhecimentos `[OK]` do modo `--ast` não substituem esse
resultado global.

## Regras de entrada

- Blocos sempre usam `{ }`; não há indentação significativa.
- Comandos simples terminam com `;` ou quebra de linha. O scanner pode fornecer
  um terminador antes de `}` ou no EOF.
- `elif` e `else` ficam na mesma linha da chave `}` anterior.
- Quebras de linha são ignoradas dentro de listas, mas não em parênteses de
  chamadas ou expressões.
- A entrada vem do stdin. O caminho de arquivo é passado por redirecionamento,
  não como argumento de `parser`.
- A recuperação de listas usa heurísticas; uma entrada malformada ambígua pode
  perder tokens até a próxima âncora explícita.

A cobertura está descrita em [Testes](../testes.md). Os limites de valores e
execução estão no [Interpretador](../interpretador.md).
