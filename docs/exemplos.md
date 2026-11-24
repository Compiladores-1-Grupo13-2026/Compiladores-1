# Exemplos e manual de uso

Este manual ensina a compilar o interpretador MiniPy, executar programas,
escolher o modo de saída e ler os erros. A pasta
[`exemplos/`](https://github.com/Compiladores-1-Grupo13-2026/Compiladores-1/tree/main/exemplos)
traz cinco programas curtos e comentados, um por tema.

## Início rápido

Na raiz do projeto, em Linux:

```sh
make
./parser < exemplos/01_expressoes.txt
```

A primeira linha impressa deve ser `Ana 20 1.75 True None`.

## 1. Compilar

### Linux

São necessários GCC, GNU Make, Flex, Bison e as bibliotecas de desenvolvimento
libfl e libm. Em Debian/Ubuntu:

```sh
sudo apt install build-essential flex bison libfl-dev
make clean
make
make tokens
```

`make` gera o executável `parser`; `make tokens` gera `tokens`, que mostra
apenas o resultado do analisador léxico. O Makefile usa alvos agrupados, que
exigem GNU Make 4.3 ou posterior.

### Windows e MSYS2

Com as ferramentas do MSYS2, o scanner dispensa a `libfl`, pois usa
`noyywrap`:

```sh
make parser tokens LDLIBS=-lm
```

Para programas com recursão profunda (perto do limite de 1000 chamadas),
reserve uma pilha maior:

```sh
make parser tokens LDLIBS=-lm LDFLAGS='-Wl,--stack,8388608'
```

Os comandos das próximas seções valem no terminal do MSYS2. Mais detalhes do
ambiente estão em [Código-fonte e build](src.md#windows-e-msys2).

## 2. Executar um programa

O executável lê o programa pela entrada padrão (stdin):

```sh
./parser < programa.txt
make run ARQ=programa.txt
```

`make run` compila `parser`, se for preciso, e depois executa o arquivo.
A extensão do arquivo não importa; os exemplos usam `.txt`, como os testes.

O programa passa pelas análises léxica, sintática e semântica antes de
executar. Se alguma análise encontra erro, nada é executado. Somente `print`
mostra valores; uma expressão solta, como `1 + 2`, é calculada e descartada.

| Resultado | Código de saída |
| --- | --- |
| Sucesso | `0` |
| Qualquer erro (léxico, sintático, semântico ou de execução) | `1` |

Para ver o código de saída depois de executar: `echo $?`.

## 3. Modos do executável

| Comando | O que mostra | Executa o programa? |
| --- | --- | --- |
| `./parser < arq` | A saída dos `print` e os erros | Sim |
| `./parser --ast < arq` | Os reconhecimentos do parser e a AST original | Não |
| `./parser --ast-otimizada < arq` | A AST depois do dobramento de constantes | Não |
| `./parser --ast-tipos < arq` | A AST com o tipo que a análise semântica inferiu para cada expressão | Não |
| `./tokens < arq` | Os tokens do analisador léxico | Não |

Exemplo com o programa abaixo, salvo como `programa.txt`:

```text
x = (1 + 2) * (3 + 4)
print(x)
```

`./parser < programa.txt` imprime `21`.

`./parser --ast-otimizada < programa.txt` mostra que as contas com
constantes já foram feitas antes da execução:

```text
PROGRAMA [linha 1]
  ATRIBUICAO [linha 1]
    IDENTIFICADOR 'x' [linha 1]
    NUMERO 21 [linha 1]
  CHAMADA 'print' [linha 2]
    IDENTIFICADOR 'x' [linha 2]
```

Com `--ast`, a mesma atribuição aparece com os nós `MULTIPLICACAO` e `SOMA`
originais, precedidos por linhas `[OK] ... reconhecida`. A comparação
completa e os limites da otimização estão em [Otimização](otimizacao.md).

`./parser --ast-tipos` mostra a "AST anotada": a árvore original, com
`: tipo` ao lado das expressões cujo tipo a análise semântica conhece. Comandos,
como a atribuição, e expressões de tipo desconhecido, como `l[0]` (o tipo do
elemento de uma lista só é conhecido na execução), ficam sem anotação. Para o
programa:

```text
x = 2 + 3
y = x * 1.5
```

a saída é:

```text
PROGRAMA [linha 1]
  ATRIBUICAO [linha 1]
    IDENTIFICADOR 'x' [linha 1]
    SOMA : int [linha 1]
      NUMERO 2 : int [linha 1]
      NUMERO 3 : int [linha 1]
  ATRIBUICAO [linha 2]
    IDENTIFICADOR 'y' [linha 2]
    MULTIPLICACAO : float [linha 2]
      IDENTIFICADOR 'x' : int [linha 2]
      NUMERO 1.5 : float [linha 2]
```

`./tokens` lista um token por linha, no formato `NOME(lexema)`. Para
`def fatorial(n) {`, a saída começa assim:

```text
DEF(def)
ID(fatorial)
LPAREN(()
ID(n)
RPAREN())
LBRACE({)
NEWLINE(\n)
```

## 4. Exemplos

| Arquivo | Conteúdo |
| --- | --- |
| `01_expressoes.txt` | Tipos básicos, aritmética, precedência, textos e atribuições compostas |
| `02_condicoes.txt` | `if`/`elif`/`else`, comparações, `and`/`or`/`not` e curto-circuito |
| `03_lacos.txt` | `while`, `for` com `range`, `for` em listas, `break`, `continue` e laços aninhados |
| `04_listas_e_textos.txt` | Listas, índices negativos, compartilhamento por referência, indexação de textos e `len()` |
| `05_funcoes_recursao.txt` | `def`, `return`, escopo local, recursão e listas passadas a funções |

Execute cada um com `./parser < exemplos/<arquivo>` e compare com a saída
esperada. O código abaixo é o mesmo dos arquivos, sem o cabeçalho.

### 01 — Expressões

```text
# Valores básicos: inteiro, real, texto, booleano e None.
idade = 20
altura = 1.75
nome = "Ana"
estudante = True
nada = None
# print aceita vários argumentos e os separa por espaço.
print(nome, idade, altura, estudante, nada)

# Inteiros: soma, subtração, multiplicação, divisão inteira (//),
# resto (%) e potência (**).
a = 17
b = 5
print(a + b, a - b, a * b)
print(a // b, a % b, b ** 3)

# A divisão com / sempre produz um número real.
print(a / b, 10 / 2)

# Misturar inteiro e real também produz real.
print(altura * 2, idade + 0.5)

# Precedência: ** vem antes de * e /, que vêm antes de + e -.
# A potência associa à direita: 2 ** 3 ** 2 é 2 ** 9.
print(2 + 3 * 4, (2 + 3) * 4, 2 ** 3 ** 2)

# Textos: + concatena e * repete.
saudacao = "Ola, " + nome
print(saudacao)
print("-" * 10)

# Atribuições compostas atualizam a própria variável.
pontos = 10
pontos += 5
pontos -= 3
pontos *= 2
print(pontos)
# /= usa a divisão real, então o resultado passa a ser real.
pontos /= 4
print(pontos)
```

Saída esperada:

```text
Ana 20 1.75 True None
22 12 85
3 2 125
3.4 5.0
3.5 20.5
14 20 512
Ola, Ana
----------
24
6.0
```

A divisão `/` sempre produz real (`10 / 2` é `5.0`), e `pontos /= 4` também
transforma `pontos` em real.

### 02 — Condições

```text
# Blocos usam chaves. elif e else ficam na mesma linha do } anterior.
nota = 7.5
if nota >= 9 {
    print("Excelente")
} elif nota >= 7 {
    print("Aprovado")
} else {
    print("Reprovado")
}

# Comparações produzem True ou False. Textos são comparados em ordem
# alfabética.
x = 4
print(x == 4, x != 4, x < 10, "abc" < "abd")

# and, or e not. As comparações são avaliadas antes de and/or.
tem_ingresso = True
idade = 15
print(tem_ingresso and idade >= 18)
print(tem_ingresso or idade >= 18)
print(not tem_ingresso)

# Curto-circuito: se o lado esquerdo do and for falso, o lado direito
# nem é avaliado. Por isso, a divisão por zero abaixo nunca acontece.
divisor = 0
if divisor != 0 and 10 / divisor > 1 {
    print("nao aparece")
} else {
    print("divisao evitada")
}

# or devolve o primeiro valor verdadeiro. Texto vazio conta como falso,
# então o valor padrão é usado.
apelido = ""
print(apelido or "sem apelido")
```

Saída esperada:

```text
Aprovado
True False True True
False
True
False
divisao evitada
sem apelido
```

Em `divisor != 0 and 10 / divisor > 1`, o lado esquerdo é falso; por isso a
divisão por zero não é avaliada.

### 03 — Laços

```text
# while repete enquanto a condição for verdadeira.
contador = 3
while contador > 0 {
    print("faltam", contador)
    contador -= 1
}
print("fim!")

# range(inicio, fim) vai de inicio até fim - 1.
soma = 0
for i in range(1, 11) { soma += i }
print("soma de 1 a 10:", soma)

# range(inicio, fim, passo) também aceita passo negativo.
for i in range(10, 0, -3) { print(i) }

# for percorre os elementos de uma lista, em ordem.
frutas = ["maca", "banana", "uva"]
for fruta in frutas { print(fruta) }

# continue pula para a próxima volta; break encerra o laço.
# Resultado: os ímpares de 1 a 7.
for n in range(1, 10) {
    if n % 2 == 0 { continue; }
    if n > 7 { break; }
    print(n)
}

# Laços aninhados: uma pequena tabuada.
for i in range(1, 3) {
    for j in range(1, 3) {
        print(i, "x", j, "=", i * j)
    }
}
```

Saída esperada:

```text
faltam 3
faltam 2
faltam 1
fim!
soma de 1 a 10: 55
10
7
4
1
maca
banana
uva
1
3
5
7
1 x 1 = 1
1 x 2 = 2
2 x 1 = 2
2 x 2 = 4
```

### 04 — Listas e textos

```text
# ---------- Listas ----------
notas = [7, 9, 5, 10]
print(notas)
# O índice 0 é o primeiro elemento; -1 é o último. len() conta os itens.
print(notas[0], notas[-1], len(notas))

# Elementos podem ser trocados ou atualizados pelo índice.
notas[2] = 6
notas[0] += 1
print(notas)

# Média: soma com for e divide pela quantidade.
soma = 0
for n in notas { soma += n }
print("media:", soma / len(notas))

# Atribuir uma lista a outra variável não copia: as duas apontam para a
# mesma lista. Alterar copia também altera notas.
copia = notas
copia[0] = 0
print(notas)

# Listas podem conter listas. Não há indexação encadeada (m[1][0]),
# então use uma variável intermediária.
matriz = [[1, 2], [3, 4]]
linha = matriz[1]
print(linha[0], len(matriz))

# Uma lista pode ocupar várias linhas.
alunos = [
    "Ana",
    "Bruno",
    "Carla"
]
for i in range(len(alunos)) { print(i, alunos[i]) }

# ---------- Textos ----------
palavra = "MiniPy"
# Textos também aceitam índices positivos e negativos, e len().
print(palavra[0], palavra[-1], len(palavra))

# for percorre um texto letra por letra.
vogais = 0
for letra in palavra {
    if letra == "a" or letra == "e" or letra == "i" or letra == "o" or letra == "u" {
        vogais += 1
    }
}
print("vogais:", vogais)

# Inverter um texto com índices negativos.
invertido = ""
for i in range(1, len(palavra) + 1) { invertido += palavra[-i] }
print(invertido)

# Primeira letra de cada nome da lista.
iniciais = ""
for nome in alunos { iniciais += nome[0] }
print(iniciais)

# Textos não podem ser alterados por índice: palavra[0] = "m" gera erro.
```

Saída esperada:

```text
[7, 9, 5, 10]
7 10 4
[8, 9, 6, 10]
media: 8.25
[0, 9, 6, 10]
3 2
0 Ana
1 Bruno
2 Carla
M y 6
vogais: 2
yPiniM
ABC
```

A linha `[0, 9, 6, 10]` mostra o compartilhamento: `copia = notas` não cria
uma lista nova, então `copia[0] = 0` também altera `notas`.

### 05 — Funções e recursão

```text
# Uma função deve ser definida antes de ser chamada.
def quadrado(x) {
    return x * x
}
print(quadrado(4), quadrado(1.5))

# Variáveis criadas dentro da função são locais.
def maior(lista) {
    resultado = lista[0]
    for item in lista {
        if item > resultado { resultado = item }
    }
    return resultado
}
print(maior([3, 8, 2, 7]))

# Funções que percorrem um texto.
def contar(texto, alvo) {
    total = 0
    for c in texto {
        if c == alvo { total += 1 }
    }
    return total
}
print(contar("banana", "a"))

# Recursão: a função chama a si mesma até chegar ao caso base.
def fatorial(n) {
    if n <= 1 { return 1; }
    return n * fatorial(n - 1)
}
def fibonacci(n) {
    if n <= 1 { return n; }
    return fibonacci(n - 1) + fibonacci(n - 2)
}
print(fatorial(5), fatorial(10))
print(fibonacci(10))

# Listas são passadas por referência: a função altera a lista original.
# Sem return, a função devolve None; a chamada solta descarta o valor.
def dobrar_todos(lista) {
    for i in range(len(lista)) { lista[i] *= 2 }
}
valores = [1, 2, 3]
dobrar_todos(valores)
print(valores)
```

Saída esperada:

```text
16 2.25
8
3
120 3628800
55
[2, 4, 6]
```

### Executar todos os exemplos

```sh
for f in exemplos/*.txt; do
    echo "== $f"
    ./parser < "$f" || echo "[FALHOU] $f"
done
```

Nenhuma linha `[FALHOU]` deve aparecer.

## 5. Erros

### Como ler uma mensagem

Os diagnósticos vão para a saída de erro (stderr). Toda mensagem começa com a
etapa que encontrou o problema e a linha do programa:

```text
[ERRO SINTATICO] Linha 1, coluna 7: syntax error (proximo a ';')
```

| Parte | Significado |
| --- | --- |
| `[ERRO SINTATICO]` | A etapa: `LEXICO`, `SINTATICO`, `SEMANTICO` ou `EXECUCAO` |
| `Linha 1` | A linha do programa onde o problema foi detectado |
| `coluna 7` | A coluna do token, só em erros sintáticos |
| `syntax error` | A descrição do problema |
| `(proximo a ';')` | O token em que o parser parou; o erro costuma estar nele ou logo antes |

Um erro pode causar outros em seguida, como o erro léxico abaixo. Corrija
primeiro a mensagem de menor linha e execute de novo.

### Erro léxico

Um caractere que não pertence à linguagem. O parser continua lendo e também
indica o erro sintático causado por ele.

```text
3 + @ 4
2 + 2
```

```text
[ERRO LEXICO] Linha 1: caractere invalido '@'
[ERRO SINTATICO] Linha 1, coluna 5: syntax error (proximo a '@')
```

### Erro sintático

Um comando mal formado, como um parêntese aberto e não fechado. Na linha 1, o
`;` aparece na coluna 7, onde o parser esperava o `)`:

```text
(3 + 2;
2 + 2;
```

```text
[ERRO SINTATICO] Linha 1, coluna 7: syntax error (proximo a ';')
```

### Erro semântico

O programa está bem formado, mas não faz sentido. Por exemplo, chamar uma
função antes do `def` ou uma função que não existe. A análise semântica
mostra todos os problemas encontrados, não só o primeiro:

```text
print(dobro(2))
def dobro(x) { return x * 2 }
print(dobra(3))
y = triplo(1)
```

```text
[ERRO SEMANTICO] Linha 1: funcao 'dobro' chamada antes de ser definida (def na linha 2)
[ERRO SEMANTICO] Linha 3: funcao 'dobra' nao definida
[ERRO SEMANTICO] Linha 4: funcao 'triplo' nao definida
```

### Erro de execução

O problema só aparece com os valores em execução. O programa para na linha do
erro, mas o que já foi impresso continua na saída:

```text
nome = "abc"
print("antes")
print(nome[3])
print("depois")
```

```text
antes
[ERRO EXECUCAO] Linha 3: indice 3 fora do intervalo (texto de tamanho 3)
```

O texto `"abc"` tem as posições 0, 1 e 2; por isso o índice 3 está fora. A
lista completa das mensagens de execução está em
[Interpretador](interpretador.md).

## 6. Regras rápidas para escrever programas

- Blocos usam chaves: `if x > 0 { print(x) }`. O bloco pode ocupar uma ou
  várias linhas.
- `elif` e `else` ficam na mesma linha do `}` anterior: `} else {`.
- Cada comando termina no fim da linha ou em `;`.
- Comentários começam com `#` e vão até o fim da linha.
- Textos usam aspas simples ou duplas e não podem ser alterados por índice.
- Uma função deve ser definida com `def` antes de ser chamada.
- Não há fatiamento (`l[1:3]`) nem indexação encadeada (`m[1][0]`); use uma
  variável intermediária.
- Laços têm limite de 1.000.000 de iterações, e chamadas aninhadas têm limite
  de 1000.

A descrição completa da linguagem está em [Início](index.md) e no
[Interpretador](interpretador.md).
