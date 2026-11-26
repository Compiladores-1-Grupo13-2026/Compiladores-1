
# MiniPy — Equipe 13

Interpretador didático de um subconjunto de Python, implementado em **C + Flex + Bison**. O MiniPy executa expressões, condicionais, laços, listas e funções com recursão. Também oferece indexação de textos, funções embutidas e dobramento de constantes na AST. Blocos usam `{ }`; comandos terminam com `;` ou quebra de linha.

[Documentação do projeto](https://compiladores-1-grupo13-2026.github.io/Compiladores-1/)

## Compilar e executar

São necessários GCC, GNU Make, Flex, Bison e as bibliotecas de desenvolvimento `libfl` e `libm`. Os testes também usam `sh`, `timeout`, `diff`, `grep`, Python 3 e GCC. Valgrind é utilizado na verificação de memória em Linux.

Em Debian/Ubuntu:

```sh
sudo apt install build-essential flex bison libfl-dev python3 python3-venv valgrind
make clean
make
make tokens
```

O executável recebe o programa pelo stdin:

```sh
./parser < programa.txt
./parser --ast < programa.txt
./parser --ast-otimizada < programa.txt
./parser --ast-tipos < programa.txt
./tokens < programa.txt
make run ARQ=programa.txt
```

O modo padrão executa depois das análises. `--ast` mostra os reconhecimentos do parser e a árvore sintática original, sem executar. `--ast-otimizada` mostra a árvore após o dobramento de constantes, também sem executar. `--ast-tipos` exibe a AST com os tipos inferidos pela análise semântica. O modo `tokens` mostra o resultado do scanner.

Expressões soltas são avaliadas e descartadas; use `print` para mostrar valores.

## Exemplo

```text
def fatorial(n) {
    if n <= 1 { return 1; }
    return n * fatorial(n - 1);
}

lista = [1, 2, 3]
lista[0] += 5

print(fatorial(5), lista)
print(lista + [4])
print(2 * lista)
print(len(lista))

nome = "MiniPy"
print(nome[0], nome[-1], len(nome))
```

Saída:

```text
120 [6, 2, 3]
[6, 2, 3, 4]
[6, 2, 3, 6, 2, 3]
3
M y 6
```

## Otimização da AST

Depois da análise semântica, o interpretador realiza dobramento de constantes: expressões formadas por literais inteiros podem ser calculadas antes da execução.

A otimização percorre a árvore em pós-ordem e considera apenas `+`, `-` e `*` entre inteiros cujo resultado cabe em `int`. Divisão, resto, potência, menos unário, reais e operações cujo resultado excede esse intervalo permanecem na AST.

Use `--ast` para visualizar a árvore original e `--ast-otimizada` para visualizar a versão transformada. O modo padrão executa a árvore otimizada.

A implementação e seus limites estão detalhados em [Dobramento de constantes](docs/otimizacao.md).

## Linguagem e diagnósticos

| Recurso | Suporte |
| --- | --- |
| Valores e contas | Inteiros, reais, booleanos, textos, `None`; aritmética, potência e menos unário |
| Textos | Concatenação com `+`, repetição com `*` por inteiro ou booleano e indexação para leitura |
| Condições | Comparações, lógica com curto-circuito, `if`, `elif` e `else` |
| Laços | `while`, `for`, `break`, `continue`; iteração em listas, textos e `range` |
| Listas | Literais, listas aninhadas e heterogêneas, compartilhamento por referência, índices positivos e negativos, alteração de elementos, concatenação com `+` e repetição com `*` por inteiro |
| Atribuições | Atribuição simples e composta: `+=`, `-=`, `*=`, `/=`; operações compostas também podem atualizar elementos de listas |
| Funções | `def`, parâmetros, `return`, recursão, chamadas por alias e escopos local/global |
| Funções embutidas | `print`, `range` e `len` |
| Inspeção | Tokens, AST original, AST otimizada e AST anotada com tipos inferidos |

### Operações com listas

O operador `+` concatena duas listas, criando uma nova lista sem modificar os operandos originais. O operador `*` repete uma lista quando combinado com um inteiro, nos dois sentidos:

```text
a = [1, 2]
b = [3]

print(a + b)
print(a * 2)
print(3 * b)
```

Saída:

```text
[1, 2, 3]
[1, 2, 1, 2]
[3, 3, 3]
```

As atribuições compostas `+=` e `*=` também aceitam essas operações.

### Indexação e tamanho

Listas e textos aceitam índices positivos e negativos. A indexação de textos é somente para leitura, pois textos são imutáveis. `len(lista)` retorna a quantidade de elementos; `len(texto)` retorna o tamanho do texto em bytes.

```text
texto = "MiniPy"
lista = [10, 20, 30]

print(texto[0], texto[-1])
print(lista[-1], len(lista), len(texto))
```

Saída:

```text
M y
30 3 6
```

A indexação e a iteração de textos operam sobre bytes e não decodificam caracteres UTF-8.

Identificadores usam letras ASCII, dígitos e `_`, sem dígito inicial; são sensíveis a maiúsculas e minúsculas. Comentários usam `#`. `elif` e `else` devem ficar na mesma linha do `}` anterior. Listas podem ocupar várias linhas.

Sucesso retorna 0; qualquer erro retorna 1. Erros de análise impedem a execução inteira; erros de execução interrompem o programa e preservam a saída anterior.

| Etapa | Formato |
| --- | --- |
| Léxico | `[ERRO LEXICO] Linha N: mensagem` |
| Sintático | `[ERRO SINTATICO] Linha N, coluna C: mensagem (proximo a 'lexema')` |
| Semântico | `[ERRO SEMANTICO] Linha N: mensagem` |
| Execução | `[ERRO EXECUCAO] Linha N: mensagem` |

## Testes

Para executar a suíte geral:

```sh
make test
```

Verificações adicionais:

```sh
python3 testes/testar_p4.py --runtime --memcheck
python3 testes/testar_valores.py --memcheck
python3 testes/verificar_b.py
make memcheck
```

Na validação final realizada em Linux em **26 de novembro de 2026**, foram aprovados:

- **125 programas:** 56 válidos e 69 inválidos.
- **3 verificações do scanner.**
- **67 expectativas** do verificador B, sem divergências, além de 9 verificações adicionais.
- Testes P4 com runtime e verificação de memória.
- Testes de valores P1 com verificação de memória.
- `make memcheck`, com código de saída 0.

Os resultados detalhados, critérios e limites das ferramentas estão em [Testes e validação](docs/testes.md).

No Windows/MSYS2, o build pode usar `LDLIBS=-lm`; o teste CRLF precisa preservar os bytes de entrada. As expectativas canônicas seguem a leitura do Linux. Os detalhes de ambiente estão no [guia de compilação](docs/src.md).

## Documentação

O site MkDocs apresenta a [linguagem](docs/index.md), os [exemplos e o manual de uso](docs/exemplos.md), a [AST](docs/ast.md), a [otimização](docs/otimizacao.md), as [decisões técnicas](docs/decisoes.md), o [interpretador](docs/interpretador.md) e as [funções](docs/funcoes.md).

A organização dos fontes está em [Código-fonte](docs/src.md); os resultados e limites dos testes estão em [Testes](docs/testes.md); os recursos disponíveis e o histórico das entregas estão em [Estado do projeto](docs/sprints.md).

Para gerar e visualizar a documentação em um ambiente Python isolado:

```sh
python3 -m venv .venv-docs
. .venv-docs/bin/activate
python -m pip install -r requirements-docs.txt
python -m mkdocs build --strict
python -m mkdocs serve
```

No Windows, ative o ambiente com:

```powershell
.\.venv-docs\Scripts\Activate.ps1
```

O servidor local mostra a documentação em <http://127.0.0.1:8000/Compiladores-1/>.

## Limites do subconjunto

MiniPy não executa Python padrão. Ainda não há fatiamento, indexação encadeada nem métodos de listas como `append`. A indexação e a iteração de textos percorrem bytes, sem decodificação de caracteres UTF-8.

Listas compartilham referências e a contagem de referências não coleta ciclos. A comparação de estruturas cíclicas também possui limitações. Funções consultam o quadro atual e o global, sem closures. Há limites de 1.000.000 de iterações por laço e 1000 chamadas aninhadas.

Consulte os detalhes em [Interpretador](docs/interpretador.md) e [Decisões técnicas](docs/decisoes.md).

## Membros

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/navicg">
        <img src="https://github.com/navicg.png" width="100" alt="Ana Victória"/><br />
        <sub><b>Ana Victória</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/Gabriel-Sayd">
        <img src="https://github.com/Gabriel-Sayd.png" width="100" alt="Gabriel Sayd"/><br />
        <sub><b>Gabriel Sayd</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/Jauzimm">
        <img src="https://github.com/Jauzimm.png" width="100" alt="João Vitor"/><br />
        <sub><b>João Vitor</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/KarolineLuz">
        <img src="https://github.com/KarolineLuz.png" width="100" alt="Karoline Luz"/><br />
        <sub><b>Karoline Luz</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/Antedeguemon21">
        <img src="https://github.com/LeonardoRodriguesMarrtinns99.png" width="100" alt="Leonardo"/><br />
        <sub><b>Leonardo</b></sub>
      </a>
    </td>
  </tr>
</table>
