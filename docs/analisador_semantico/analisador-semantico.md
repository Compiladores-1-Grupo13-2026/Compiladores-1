# Analisador semântico

A análise semântica verifica nomes, tipos conhecidos e o contexto dos comandos
depois que o parser monta a [AST](../ast.md). A implementação fica em
`Semantica/semantico.c` e usa a tabela de símbolos de `Semantica/Tabela/`.

A função `semantico_analisar` percorre a árvore, emite diagnósticos, libera a
tabela e devolve a quantidade de erros. O modo padrão chama o interpretador
somente quando todas as análises terminam sem erro.

## Percurso da árvore

O visitante trata um nó e delega o percurso das sequências a quem conhece o
significado de `prox`. Em um bloco ou programa, percorre os comandos; em uma
lista, seus elementos; em uma chamada, os argumentos.

As expressões são verificadas depois de seus operandos. Uma sequência de
comandos tem uma passagem inicial para registrar definições de funções e outra
para analisar seus corpos e os demais comandos. Isso permite analisar chamadas
recursivas e referências a outras funções definidas mais abaixo no arquivo.

A análise também pode percorrer os comandos que a recuperação sintática
preservou. Isso amplia os diagnósticos, mas não autoriza a execução de um
programa que teve erro léxico ou sintático.

## Tabela de símbolos

Cada símbolo registra nome, categoria, tipo conhecido, escopo, linha e quantidade
de parâmetros quando aplicável. As categorias são variável, função, parâmetro
e função embutida. `print` e `range` são criados no escopo global.

A tabela mantém um hash de 211 posições por escopo. A busca local examina apenas
o escopo atual; a busca geral percorre os escopos ativos do mais interno ao
global. Apenas definições de função criam escopos de análise: blocos de
condição e laço usam o escopo que já está ativo.

| Tipo na análise | Significado |
| --- | --- |
| `int`, `float`, `bool`, `str` | Tipo conhecido da expressão |
| `lista` | Lista; os tipos de seus elementos não são guardados no símbolo |
| `none` | Literal `None` ou resultado de um comando |
| `funcao` | Valor de função |
| `?` | Tipo ainda desconhecido |
| `erro` | Marcador interno de falha já diagnosticada |

Esses nomes descrevem informação estática. Os tipos concretos de execução ficam
em `Valor`; por exemplo, os diagnósticos do executor usam `list`, `NoneType` e
`function`. A tabela semântica não armazena os valores que serão executados.

## Nomes e atribuições

A primeira atribuição simples válida cria uma variável no escopo atual.
O lado direito é visitado antes da inserção: `y = y + 1` não declara `y`
antes de verificar a leitura do próprio nome.

Reatribuições atualizam o tipo conhecido e preservam a linha da primeira
atribuição. Uma atribuição composta em um identificador exige uma variável
já existente no escopo atual. Um nome de função ou de função embutida é
protegido contra atribuição.

Listas podem ser vazias, heterogêneas e aninhadas. Seus elementos são analisados
individualmente. Quando conhecidos, o tipo da base de uma indexação deve ser
lista e o índice deve ser inteiro. O tipo do resultado de `lista[i]` continua
desconhecido, pois não há informação estática por elemento.

## Expressões e condições

A análise usa os tipos conhecidos da tabela quando um operando é um
identificador. Operações aritméticas aceitam números e booleanos, concatenação
de textos e repetição de texto por inteiro ou booleano. Uma operação entre
tipos conhecidos incompatíveis é rejeitada antes da execução.

~~~text
texto = "a";
resultado = texto - 1;
~~~

Esse programa produz erro semântico: o tipo de `texto` já é conhecido.
Quando um tipo depende de um parâmetro, de uma chamada ou de um elemento de
lista, a verificação pode ficar para o executor.

Comparações de ordem aceitam números ou dois textos quando seus tipos são
conhecidos. Igualdade e diferença admitem tipos distintos. Os operadores
`and`, `or` e `not` aceitam valores de qualquer tipo; a avaliação de verdade
e o curto-circuito pertencem à execução.

Expressões numéricas constantes também são verificadas quanto a divisão por
zero, potência sem resultado real e resultado não finito. A inferência
distingue reais, o resultado real de `/` e potências inteiras com expoente
negativo conhecido.

## Laços, funções e chamadas

| Construção | Verificação |
| --- | --- |
| `break`, `continue` | Exigem estar dentro de um laço da função atual |
| `for` | Rejeita iterável numérico literal e identificador de tipo numérico conhecido |
| `return` | Exige estar dentro de uma função |
| `def` | Verifica nomes repetidos no mesmo escopo e parâmetros duplicados |
| Chamada direta | Verifica existência do nome, possibilidade de chamada e aridade conhecida |
| Chamada global | Diagnostica função cuja definição aparece em linha posterior |
| `range` | Exige entre um e três argumentos |

Ao entrar no corpo de uma função, o contador de laços é reiniciado. Assim,
um `break` dentro de uma função não pode interromper um laço que envolve
apenas sua definição no código-fonte.

Parâmetros começam com tipo desconhecido. O retorno de uma chamada também
tem tipo desconhecido, pois a análise não infere o tipo de retorno das funções.
Chamadas por aliases ou parâmetros podem exigir validação da aridade e do
tipo de função no executor.

## Diagnósticos e execução

Os erros seguem o formato:

~~~text
[ERRO SEMANTICO] Linha N: mensagem
~~~

São escritos no stderr e resultam em código de saída 1. O analisador procura
continuar a visita para relatar erros independentes. Um marcador de erro já
relatado impede atualizações inválidas da tabela.

A análise não executa o programa nem verifica todos os caminhos possíveis.
Por exemplo, ela não propaga o valor de uma variável como uma constante:

~~~text
divisor = 0;
print(10 / divisor);
~~~

O tipo de `divisor` é conhecido, mas o zero será verificado na execução.
Também ficam para o executor os limites de índices, os tipos dinâmicos dos
argumentos de `range`, seu passo zero, o limite de recursão e o limite de
iterações.

As diferenças entre os escopos da tabela e os quadros de execução, os limites
numéricos e as operações ainda não suportadas estão em
[Decisões técnicas](../decisoes.md). Consulte o
[interpretador](../interpretador.md) para o comportamento em execução e
[testes](../testes.md) para a validação.
