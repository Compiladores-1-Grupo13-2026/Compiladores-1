# Dobramento de constantes

O MiniPy simplifica operações aritméticas com constantes na [AST](ast.md)
depois da análise semântica e antes da execução. O otimizador visita os filhos
antes do nó pai. Assim, em `(1 + 2) * (3 + 4)`, as somas viram `3` e `7`,
e a multiplicação vira `21`. O interpretador usa o valor calculado, evitando
refazer a operação a cada execução, por exemplo dentro de um laço.

## Antes e depois

Compile com `make` e salve este programa como `programa.txt`:

~~~text
x = (1 + 2) * (3 + 4)
print(x)
~~~

### AST original

`./parser --ast < programa.txt` mostra os reconhecimentos do parser e a
árvore original, sem executar o programa.

~~~text
[OK] atribuicao reconhecida
[OK] chamada de funcao reconhecida
PROGRAMA [linha 1]
  ATRIBUICAO [linha 1]
    IDENTIFICADOR 'x' [linha 1]
    MULTIPLICACAO [linha 1]
      SOMA [linha 1]
        NUMERO 1 [linha 1]
        NUMERO 2 [linha 1]
      SOMA [linha 1]
        NUMERO 3 [linha 1]
        NUMERO 4 [linha 1]
  CHAMADA 'print' [linha 2]
    IDENTIFICADOR 'x' [linha 2]
~~~

### AST otimizada

`./parser --ast-otimizada < programa.txt` mostra a árvore transformada após
as análises, sem executar o programa.

~~~text
PROGRAMA [linha 1]
  ATRIBUICAO [linha 1]
    IDENTIFICADOR 'x' [linha 1]
    NUMERO 21 [linha 1]
  CHAMADA 'print' [linha 2]
    IDENTIFICADOR 'x' [linha 2]
~~~

No modo padrão, `./parser < programa.txt` executa a árvore otimizada e imprime `21`.

## Limites

- Apenas `+`, `-` e `*` binários são candidatos. Os dois operandos devem
  ser nós `AST_NUMERO` armazenados como inteiros, e o resultado deve caber
  em `int`. Por exemplo, em uma plataforma com `int` de 32 bits,
  `2147483647 + 1` fica na árvore para o interpretador calcular com `long long`.
- A simplificação pode ser parcial. Em `2 + 3 + x * 4`, o otimizador dobra
  `2 + 3` para `5`. A multiplicação continua dependendo do valor de `x`.
- Operações com nós de literais reais, como `1.5 + 2`, e os operadores
  `/`, `//`, `%`, `**` e o menos unário ficam fora do dobramento.
  As análises e a execução continuam responsáveis pelos diagnósticos,
  inclusive divisão por zero.

A regra *as-if* permite a transformação quando a execução conserva o
comportamento observável do programa. A visualização da AST é o modo de
inspecionar quais operações foram substituídas.
