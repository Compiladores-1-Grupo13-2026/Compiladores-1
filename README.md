# MiniPy — Equipe 13

Interpretador didático de um subconjunto de Python, implementado em **C + Flex +
Bison**. O MiniPy executa expressões, condicionais, laços, listas e funções com
recursão. Blocos usam `{ }`; comandos terminam com `;` ou quebra de linha.

[Documentação do projeto](https://compiladores-1-grupo13-2026.github.io/Compiladores-1/)

## Compilar e executar

São necessários GCC, GNU Make, Flex, Bison e as bibliotecas de desenvolvimento
libfl e libm. Os testes também usam `sh`, `timeout`, `diff` e `grep`; os testes
unitários usam Python 3. Valgrind é usado na verificação de memória em Linux.

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
./tokens < programa.txt
make run ARQ=programa.txt
```

O modo padrão executa depois das análises. `--ast` mostra reconhecimentos e a
árvore sintática, sem executar. O modo tokens mostra o resultado do scanner.
Expressões soltas são descartadas; use `print` para mostrar valores.

```text
def fatorial(n) {
    if n <= 1 { return 1; }
    return n * fatorial(n - 1);
}
lista = [1, 2, 3]
lista[0] += 5
print(fatorial(5), lista)
```

Saída: `120 [6, 2, 3]`.

## Linguagem e diagnósticos

| Recurso | Suporte |
| --- | --- |
| Valores e contas | Inteiros, reais, booleanos, textos, `None`; aritmética, potência, concatenação e repetição de textos |
| Condições | Comparações, lógica com curto-circuito, `if`, `elif` e `else` |
| Laços | `while`, `for`, `break`, `continue`; iteração em listas, textos e `range` |
| Listas | Literais, compartilhamento por referência, índices positivos e negativos, alteração de elementos |
| Funções | `def`, parâmetros, `return`, recursão, chamadas por alias, `print` e `range` |

Identificadores usam letras ASCII, dígitos e `_`, sem dígito inicial; são sensíveis
a maiúsculas e minúsculas. Comentários usam `#`. `elif` e `else` devem ficar na
mesma linha do `}` anterior. Listas podem ocupar várias linhas.

Sucesso retorna 0; qualquer erro retorna 1. Erros de análise impedem a execução
inteira; erros de execução interrompem o programa e preservam a saída anterior.

| Etapa | Formato |
| --- | --- |
| Léxico | `[ERRO LEXICO] Linha N: mensagem` |
| Sintático | `[ERRO SINTATICO] Linha N, coluna C: mensagem (proximo a 'lexema')` |
| Semântico | `[ERRO SEMANTICO] Linha N: mensagem` |
| Execução | `[ERRO EXECUCAO] Linha N: mensagem` |

## Testes

```sh
make test
python3 testes/testar_p4.py --runtime
python3 testes/testar_valores.py
python3 testes/verificar_b.py
make memcheck
```

Na validação local de **6 de novembro de 2026**, passaram os **108 casos**
(46 válidos e 62 inválidos) e as três verificações de tokens, tanto em Windows
quanto em Linux. Os quatro executáveis unitários passaram. Valgrind verificou
os 46 casos válidos e os quatro unitários; `make memcheck` terminou com sucesso.
Veja [critérios e limites da validação](docs/testes.md).

No Windows/MSYS2, o build pode usar `LDLIBS=-lm`; o teste CRLF precisa preservar
os bytes de entrada. As expectativas canônicas seguem a leitura do Linux.
Os detalhes de ambiente estão no [guia de compilação](docs/src.md).

## Documentação

O site MkDocs apresenta a [linguagem](docs/index.md), a [AST](docs/ast.md), as
[decisões técnicas](docs/decisoes.md), o [interpretador](docs/interpretador.md)
e as [funções](docs/p5.md). A organização dos fontes está em
[Código-fonte](docs/src.md); recursos e limites estão em
[Estado do projeto](docs/sprints.md).

Para gerar e visualizar a documentação em um ambiente Python isolado:

```sh
python3 -m venv .venv-docs
. .venv-docs/bin/activate
python -m pip install -r requirements-docs.txt
python -m mkdocs build --strict
python -m mkdocs serve
```

No Windows, ative o ambiente com `.\.venv-docs\Scripts\Activate.ps1`.
O servidor local mostra a documentação em
<http://127.0.0.1:8000/Compiladores-1/>.

## Limites do subconjunto

MiniPy não executa Python padrão. Não há fatiamento, indexação de textos,
indexação encadeada ou coleta de ciclos entre listas. Operações aritméticas
entre listas não são implementadas. Funções veem o quadro atual e o global,
sem closures. Há limites de 1.000.000 de iterações por laço e 1000 chamadas
aninhadas. Consulte os detalhes no [interpretador](docs/interpretador.md).

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
        <img src="https://github.com/Antedeguemon21.png" width="100" alt="Leonardo"/><br />
        <sub><b>Leonardo</b></sub>
      </a>
    </td>
  </tr>
</table>
