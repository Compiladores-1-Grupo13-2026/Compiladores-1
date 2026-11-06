# Interpretador

O interpretador executa a AST de MiniPy depois que as análises léxica,
sintática e semântica terminam sem erros. A linguagem oferece expressões,
condições, laços, variáveis, listas, funções e os builtins `print` e `range`.

## Executar um programa

Na raiz do projeto:

```sh
make
./parser < programa.txt
```

O modo padrão mostra somente a saída produzida por `print` e os diagnósticos.
Uma expressão usada como comando tem seu resultado descartado. Para examinar
a estrutura do programa sem executá-lo, use:

```sh
./parser --ast < programa.txt
```

Esse modo mostra os reconhecimentos do parser e a AST. O alvo auxiliar
`make run ARQ=programa.txt` executa o programa depois de construir `parser`.
As instruções de ambiente e compilação estão em [Código-fonte e build](src.md).

## Fluxo de execução

```text
Código MiniPy → scanner Flex → parser Bison → AST
                                            ↓
                                   análise semântica
                                            ↓
                                       execução
```

Um erro em qualquer análise impede a execução de todo o programa, inclusive
de comandos que aparecem antes do erro. Durante a execução, um erro interrompe
o programa no ponto em que ocorreu; a saída já impressa é preservada.

| Componente | Função |
| --- | --- |
| `interpretador.c` | Despacha os nós, percorre blocos e interrompe erros de execução |
| `valor.c` | Constrói, copia, libera e imprime valores |
| `exec_expressoes.c` | Avalia literais, operadores aritméticos e menos unário |
| `exec_condicoes.c` | Avalia comparações, lógica e condicionais |
| `exec_lacos.c` | Executa `while`, `for`, `break` e `continue` |
| `ambiente.c` | Mantém o quadro global e os quadros de chamadas |
| `lista.c` | Mantém o vetor dinâmico de elementos de uma lista |
| `exec_atribuicao.c` | Lê nomes, atribui valores e acessa listas |
| `exec_funcoes.c` | Executa definições, chamadas, retornos e builtins |

Todos esses arquivos ficam em `Interpretador/`. O formato dos nós está em
[AST](ast.md), e as políticas de tipos e memória estão em
[Decisões técnicas](decisoes.md).

## Valores e impressão

`Valor` distingue `None`, inteiro, real, booleano, texto, lista e função.
Inteiros usam `long long`; reais usam `double`. Um literal numérico é
classificado pelo seu valor: se for integral e couber em `long long`, produz
um inteiro; caso contrário, produz um real.

| Valor | Exemplo de saída de `print` |
| --- | --- |
| Inteiro | `7` |
| Real | `2.0` ou `3.5` |
| Booleano | `True` ou `False` |
| Ausência de valor | `None` |
| Texto | `fim` |
| Lista | `[1, 2, 'a']` |
| Função | `<funcao somar>` |

Reais são formatados com até 15 algarismos significativos. Quando a
representação finita de um real não contém ponto ou expoente, a impressão
acrescenta `.0`. Textos aparecem sem aspas fora de listas; dentro delas,
recebem aspas simples e escapes para caracteres como quebra de linha,
tabulação, barra e apóstrofo.

`print` avalia todos os argumentos antes de imprimir, separa os valores por
um espaço e termina com uma quebra de linha. `print()` imprime apenas a
quebra de linha. Veja também [Funções e chamadas](p5.md).

## Expressões e textos

MiniPy implementa `+`, `-`, `*`, `/`, `//`, `%`, `**` e o menos unário.
Booleanos participam das contas como inteiros: `True` vale 1 e `False` vale 0.
Operações numéricas com um real produzem um real; `/` sempre produz um real.

```text
print(3 + 4 * 2, 7 / 2, -7 // 2, -7 % 3, 2 ** 10)
```

Saída:

```text
11 3.5 -4 2 1024
```

`//` arredonda o quociente para baixo, e `%` produz resto com o sinal do
divisor. Potência entre inteiros produz inteiro quando o expoente é não
negativo e real quando é negativo. A aritmética inteira detecta resultados
fora do intervalo suportado.

Textos aceitam concatenação e repetição por inteiro ou booleano, em qualquer
ordem na multiplicação. Repetir zero vezes ou um número negativo produz
texto vazio.

```text
print("Mini" + "Py", "ab" * 2, 3 * "x")
print("linha 1\nlinha 2", "coluna\tvalor")
```

Os escapes tratados em literais são `\n`, `\t`, `\\`, `\"` e `\'`.
Um escape desconhecido preserva a barra e a letra.

## Condições e lógica

`None`, `False`, zero numérico, texto vazio e lista vazia são falsos. Os
demais valores são verdadeiros, inclusive funções. `not` devolve um booleano;
`and` e `or` usam curto-circuito e devolvem um dos operandos.

```text
print(0 or 5, "ok" and 7, not [])
if 2 < 1 { print("menor"); } elif 2 == 2 { print("igual"); } else { print("maior"); }
```

Saída:

```text
5 7 True
igual
```

Igualdade e diferença aceitam tipos distintos. Números são comparados por
valor, textos por conteúdo, listas elemento a elemento e funções pela
identidade da definição. Comparações de ordem aceitam dois números ou dois
textos. `elif` e `else` devem acompanhar o `}` anterior na mesma linha.

## Variáveis, listas e atribuições

`=` define ou substitui uma variável no quadro atual. `+=`, `-=`, `*=` e `/=`
calculam o novo valor usando o anterior. Esses operadores também podem ter
um elemento de lista como alvo.

```text
l = [1, 2]
l[0] += 5
b = l
b[1] = 9
print(l, l[-1])
```

Saída:

```text
[6, 9] 9
```

Listas podem ser vazias, aninhadas e heterogêneas. A indexação exige um
inteiro, aceita posições negativas contadas a partir do fim e verifica os
limites da lista. Strings não são indexáveis nesta implementação.

Uma atribuição simples avalia o lado direito antes de avaliar o alvo. Uma
atribuição composta fixa o alvo e copia seu valor anterior antes de avaliar
o lado direito. Essa ordem preserva o alvo quando uma chamada altera a lista
durante o cálculo.

A cópia de uma lista compartilha a mesma lista, como no exemplo de `b = l`.
Textos possuem cópias independentes. Aritmética entre listas, como `l + l`
ou `l * 2`, não está implementada no executor.

## Laços e sinais de controle

`while` avalia a condição a cada volta. `for` avalia o iterável uma vez e
percorre uma lista, um texto ou a lista produzida por `range`.

```text
x = 3
while x > 0 { print(x); x -= 1; }
for i in range(1, 8, 3) { print(i); }
for letra in "abc" { if letra == "b" { continue; } print(letra); }
```

Saída:

```text
3
2
1
1
4
7
a
c
```

`break` encerra o laço mais próximo; `continue` passa à próxima volta. Os
laços consomem esses dois sinais. Um `return` atravessa laços e condicionais
até a chamada da função. Cada execução de um laço admite até **1.000.000
de iterações**; tentar iniciar a seguinte gera erro.

O `for` mantém o iterável vivo durante o corpo. Para listas, consulta o
tamanho atual e o elemento a cada volta. A iteração de texto percorre bytes,
sem decodificar caracteres UTF-8.

## Funções e escopos

Uma definição `def` registra a função quando é executada. Uma chamada avalia
os argumentos no quadro de quem chama, empilha um novo quadro, associa os
parâmetros e executa o corpo. Ao terminar, transporta o retorno e libera os
locais. Uma função sem retorno explícito, ou com `return;`, devolve `None`.

A busca durante a execução consulta o quadro atual e depois o global.
Quadros de chamadas intermediárias não ficam visíveis, e funções não
capturam variáveis de escopos externos. O limite é de **1000 chamadas
aninhadas**. Detalhes e exemplos estão em [Funções e chamadas](p5.md).

## Programa completo

```text
def fatorial(n) {
    if n <= 1 { return 1; } else { return n * fatorial(n - 1); }
}
x = 5;
while x > 0 { print(x); x -= 1; }
for i in range(3) { print(i); }
lista = [1, 2, 3];
print(fatorial(5), lista[0], "fim")
```

Saída:

```text
5
4
3
2
1
0
1
2
120 1 fim
```

## Erros e código de saída

Sucesso retorna código **0**; erros léxicos, sintáticos, semânticos ou de
execução retornam **1**. Diagnósticos vão para stderr. Erros de execução
usam o prefixo `[ERRO EXECUCAO] Linha N:` e interrompem a execução por
`erro_execucao`.

| Situação em execução | Mensagem |
| --- | --- |
| Divisor numérico zero | `divisao por zero` |
| Zero elevado a expoente negativo | `potencia de zero com expoente negativo` |
| Operandos incompatíveis | `operacao '+' entre tipos incompativeis ('str' e 'int')` |
| Menos unário incompatível | `operacao '-' unaria com tipo incompativel ('str')` |
| Overflow de inteiro | `resultado inteiro fora do intervalo` |
| Comparação de ordem incompatível | `comparacao '<' entre tipos incompativeis ('str' e 'int')` |
| Índice fora dos limites | `indice 5 fora do intervalo (lista de tamanho 2)` |
| Índice de tipo inválido | `indice de lista deve ser int, recebido 'float'` |
| Base não indexável | `'int' nao pode ser indexado` |
| Objeto não iterável | `'int' nao e iteravel` |
| Limite de laço | `laco excedeu o limite de 1000000 iteracoes` |
| Limite de chamadas | `limite de recursao excedido (1000 chamadas)` |
| Argumento inválido de `range` | `argumentos de range devem ser int` |
| Passo zero de `range` | `passo de range nao pode ser zero` |
| Chamada de valor não chamável | `'x' nao e uma funcao (tipo 'int')` |

Quando um problema já pode ser determinado pela análise semântica, ele é
diagnosticado antes de qualquer execução, com mensagem semântica própria.
Os exemplos da tabela descrevem problemas que chegaram ao executor com
tipos ou valores conhecidos somente em execução.

## Memória e limites

Cada resultado de `interpretar_no` pertence ao chamador. Operações
aritméticas e comparações recebem operandos emprestados; o chamador os
libera. `ambiente_definir` e `lista_adicionar` assumem a posse do valor;
`ambiente_buscar` empresta um ponteiro. Strings são copiadas, listas usam
contagem de referências e valores de função apontam para definições
pertencentes à AST.

Os caminhos normais liberam os valores, quadros e a AST. O mecanismo de
erro usa `longjmp`; o contrato admite alocações temporárias remanescentes
no caminho de erro. Contagem de referências não coleta ciclos entre listas.
A impressão protege ciclos com `[...]`, mas a comparação de listas cíclicas
distintas não possui detecção de ciclo. O suporte a inteiros e reais tem os
limites das representações C utilizadas.

Os comandos de teste, a cobertura e os resultados de validação estão em
[Testes](testes.md).
