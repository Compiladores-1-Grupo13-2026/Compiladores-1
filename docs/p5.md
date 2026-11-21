# Funções e chamadas

MiniPy oferece funções com parâmetros posicionais, retorno, recursão,
chamadas por variáveis e os builtins `print` e `range`. As definições usam
`def` e um corpo delimitado por `{ }`.

## Definir e chamar

```text
def somar(a, b) {
    return a + b;
}
print(somar(2, 3))
```

Saída:

```text
5
```

A definição precisa ter sido executada antes de uma chamada no código
global. Chamadas usam a quantidade exata de argumentos posicionais da
definição. Parâmetros padrão, argumentos nomeados e argumentos variáveis
não fazem parte da linguagem.

Os argumentos são avaliados da esquerda para a direita, no quadro de quem
chama. Depois disso, o executor busca a função, cria seu quadro local e
associa os parâmetros aos valores obtidos. Essa ordem permite chamadas
aninhadas sem misturar os locais das funções.

## Retorno

`return expressão;` devolve um valor e encerra imediatamente o corpo.
`return;` e chegar ao fim do corpo devolvem `None`.

```text
def identidade(x) { return x; }
def vazia() { return; }
def sem_retorno() { 1 + 2; }
print(identidade("ok"), vazia(), sem_retorno())
```

Saída:

```text
ok None None
```

O retorno pode ocorrer dentro de `if`, `while` ou `for`. O sinal de retorno
atravessa esses comandos até a chamada, que recupera o valor e libera o
quadro local. Uma lista retornada permanece viva pela contagem de
referências; um texto retornado mantém sua própria cópia.

## Escopo das variáveis

Cada chamada possui um quadro local. A busca lê primeiro esse quadro e
depois o global; ela não lê os locais de funções que chamaram a atual.
Uma atribuição simples cria ou substitui uma variável no quadro atual.

```text
x = 10
def ler_global() { return x; }
def executar() {
    x = 20;
    return ler_global();
}
print(executar(), x)
```

Saída:

```text
10 10
```

Listas passadas como argumentos são compartilhadas. Alterar um de seus
elementos é visível ao chamador. Atribuir outra lista ao parâmetro muda
somente a variável local.

```text
def alterar(itens) {
    itens[0] = 9;
    itens = [0];
    return itens;
}
l = [1, 2]
print(alterar(l), l)
```

Saída:

```text
[0] [9, 2]
```

Não há construção `global` nem captura de ambientes externos. Uma função
definida dentro de outra não mantém os locais da função externa; prefira
definições globais quando precisar reutilizar funções.

## Recursão

```text
def fatorial(n) {
    if n <= 1 { return 1; }
    return n * fatorial(n - 1);
}
print(fatorial(5))
```

Saída:

```text
120
```

Recursão mútua também é permitida quando as definições estão disponíveis
antes da chamada inicial:

```text
def par(n) {
    if n == 0 { return True; }
    return impar(n - 1);
}
def impar(n) {
    if n == 0 { return False; }
    return par(n - 1);
}
print(par(4), impar(4))
```

Saída:

```text
True False
```

O executor admite até **1000 chamadas aninhadas**. A tentativa de ultrapassar
esse limite gera `limite de recursao excedido (1000 chamadas)`.

## Funções como valores

Uma função pode ser atribuída a uma variável, passada como argumento ou
devolvida como resultado. O valor guarda a referência à definição na AST,
sem capturar o quadro em que foi criado.

```text
def dobro(x) { return x * 2; }
def aplicar(f, valor) { return f(valor); }
alias = dobro
print(alias(3), aplicar(dobro, 4), dobro)
```

Saída:

```text
6 8 <funcao dobro>
```

Chamadas por variável ou parâmetro cuja aridade não é conhecida pela
semântica são verificadas novamente durante a execução. Chamar um valor
que não seja função gera, por exemplo, `'f' nao e uma funcao (tipo 'int')`.

## `print`

`print(a, b, ...)` avalia todos os argumentos antes de escrever a saída.
Os valores são separados por um espaço e a linha termina com `\n`.
`print()` escreve uma linha vazia; o resultado da chamada é `None`.

```text
print(1, 4 / 2, True, None, [1, "a"])
```

Saída:

```text
1 2.0 True None [1, 'a']
```

O formato completo dos valores está em [Interpretador](interpretador.md).

## `range`

`range` devolve uma lista de inteiros e aceita estas formas:

| Chamada | Início | Fim exclusivo | Passo |
| --- | --- | --- | --- |
| `range(fim)` | `0` | `fim` | `1` |
| `range(inicio, fim)` | `inicio` | `fim` | `1` |
| `range(inicio, fim, passo)` | `inicio` | `fim` | `passo` |

```text
print(range(3), range(1, 8, 3), range(5, 0, -2))
```

Saída:

```text
[0, 1, 2] [1, 4, 7] [5, 3, 1]
```

Uma direção incompatível com o intervalo produz lista vazia. O passo zero
gera `passo de range nao pode ser zero`. Todos os argumentos precisam
produzir valores do tipo inteiro; booleanos, textos, listas, `None` e
reais geram `argumentos de range devem ser int`.

`range` materializa a lista inteira antes de devolver o resultado, portanto
intervalos grandes consomem memória proporcional à quantidade de elementos.
`print` e `range` são reconhecidos como builtins pelo nome da chamada.

## Verificações e organização

A análise semântica verifica nomes de função, chamadas globais anteriores à
definição, aridade conhecida, parâmetros repetidos, redefinições e `return`
fora de função. Problemas que dependem do valor recebido são diagnosticados
pelo executor. A semântica protege os nomes de funções e builtins contra
atribuições inválidas.

Na [AST](ast.md), uma definição usa `AST_FUNCAO`, com parâmetros em `a` e
corpo em `b`; `AST_CHAMADA` guarda os argumentos em `a`; `AST_RETURN`
guarda a expressão em `a`, ou `NULL`. As sequências usam `prox`.
O executor das chamadas está em `Interpretador/exec_funcoes.c`, e os quadros
ficam em `Interpretador/ambiente.c`.

A cobertura de recursão, escopos, argumentos, builtins, retorno e limites,
junto dos resultados de validação, está em [Testes](testes.md).
