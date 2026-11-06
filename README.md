# MiniPy — Equipe 13

MiniPy é um interpretador didático de um subconjunto de Python, com blocos
delimitados por `{ }`, implementado em **C + Flex + Bison** pela Equipe 13.
O objetivo é estudar a construção incremental do léxico e da gramática antes
de implementar AST, tabela de símbolos e execução.

Na **Sprint 4**, a Pessoa A acrescenta infraestrutura, condições (P2), ambiente,
listas e atribuições (P4) à AST e à semântica da Sprint 3. Os quatro módulos de
execução da Pessoa B ainda precisam ser integrados. Consulte o
[interpretador](docs/interpretador.md) e [funções da Sprint 3](docs/p5.md).

[Documentação no GitHub Pages](https://compiladores-1-grupo13-2026.github.io/Compiladores-1/)

## Tecnologias

C implementa o projeto; Flex gera o scanner e Bison gera o parser. Make automatiza o build. MkDocs com Material publica esta documentação no GitHub Pages.

## Regras da linguagem

- MiniPy é sensível a maiúsculas e minúsculas. Identificadores usam letras ASCII, dígitos e `_`, sem dígito inicial.
- Blocos de `if`, `elif`, `else`, `while`, `for` e `def` usam sempre `{ }`.
- Comandos simples terminam com `;` ou quebra de linha. Antes de `}` e no EOF,
  o scanner fornece um terminador quando necessário, inclusive em `if x { y = 1 }`.
  Comandos de bloco não precisam de terminador; um `;` isolado não é comando vazio.
- `else` e `elif` devem estar na mesma linha do `}` anterior. A quebra de linha
  encerra o condicional; `else` na linha seguinte é diagnosticado como `else sem if`.
- Listas podem ocupar várias linhas dentro de `[ ]`; seus elementos precisam de vírgulas.
- `print` e `range` são identificadores comuns. A execução das chamadas depende do módulo da Pessoa B.
- Comentários começam com `#` e vão até o fim da linha.
- Números são inteiros ou decimais, sem notação exponencial; o sinal é operador unário.
  Strings aceitam escapes e uma única linha. Não há indentação significativa.

## Como rodar o projeto

Ambiente validado: GCC 13.3.0, Flex 2.6.4, Bison 3.8.2, GNU Make e GNU coreutils
(`timeout`, usado para detectar travamentos nos testes). É necessário o pacote de
desenvolvimento de libfl e libm. Em Debian/Ubuntu, instale as dependências e execute os comandos na raiz do repositório:

```sh
sudo apt install build-essential flex bison libfl-dev python3-venv
```

Se ainda não tiver o projeto na máquina:

```sh
git clone https://github.com/Compiladores-1-Grupo13-2026/Compiladores-1.git
cd Compiladores-1
```

Depois de integrar os módulos da Pessoa B, compile e execute:

```sh
make clean
make
make tokens
./tokens < testes/validos/base_programa_referencia.txt
./parser --ast < testes/validos/base_programa_referencia.txt
./parser < programa.txt
make test
```

`make` gera `parser.tab.c/.h`, `parser.output`, `lex.yy.c` e `parser`.
`make tokens` compila o mesmo scanner com `SO_TOKENS`; `make clean` remove os
artefatos. Eles são ignorados pelo Git. Para ver a recuperação e o código de saída:

```sh
./parser < testes/invalidos/base_multiplos_erros.txt 2>&1
echo "$?"  # 1
```

O modo padrão executa somente após as análises passarem e mostra os `print` e
os erros. `--ast` mostra reconhecimentos e a AST, sem executar. No Windows/MSYS2,
compile com `make parser tokens LDLIBS=-lm`. Erros vão para stderr; sucesso
retorna 0 e qualquer erro retorna 1. Erros de execução preservam a saída anterior.

| Tipo | Exemplo de diagnóstico | Código |
| --- | --- | --- |
| Léxico | `[ERRO LEXICO] Linha 1: caractere invalido '@'` | 1 |
| Sintático | `[ERRO SINTATICO] Linha 1, coluna 1: syntax error (proximo a 'EOF')` | 1 |
| Semântico | `[ERRO SEMANTICO] Linha 1: variável 'x' usada antes de ser atribuída` | 1 |
| Execução | `[ERRO EXECUCAO] Linha 3: indice 5 fora do intervalo (lista de tamanho 2)` | 1 |

Para construir a documentação:

```sh
python3 -m venv .venv-docs
.venv-docs/bin/pip install -r requirements-docs.txt
.venv-docs/bin/mkdocs build --strict
```

O workflow `.github/workflows/gh-pages.yml` constrói e publica `site/` quando
as mudanças chegam a `main`. O build local não confirma a publicação remota.

Para visualizar a documentação localmente:

```sh
.venv-docs/bin/mkdocs serve
```

Abra <http://127.0.0.1:8000/Compiladores-1/>. Use `Ctrl+C` para encerrar.

## Escopo e limitações

A implementação A preserva os headers e o despachante do contrato. P2 implementa
comparações, curto-circuito e condicionais; P4 implementa quadros atual/global,
listas compartilhadas, índices negativos e atribuições simples/compostas.

```sh
python3 testes/testar_p4.py
python3 testes/testar_p4.py --runtime
python3 testes/testar_p4.py --runtime --memcheck
python3 testes/atualizar_esperados_a.py
```

O primeiro comando testa tabela/P2/P4 com a AST real. `--runtime` inclui os
contratos de execução e exige os módulos B; `--memcheck` exige Valgrind.
O último comando regenera somente os 21 históricos A, após compilar e revisar.
O runner geral usa `--ast` nos históricos e execução em `_exec_`.
Os casos `.mpy` antigos permanecem fora da suíte geral `.txt`.

A validação C continua pendente: Make/GCC/Flex/Bison estão ausentes neste
ambiente e faltam os módulos B. Foram preparados 13 casos de execução A,
com os mesmos cenários dos 17 casos iniciais agrupados; as expectativas
seguem o contrato e ainda precisam de confirmação com o parser real.
Veja [arquitetura, mensagens e pendências](docs/interpretador.md).

A [página Início](docs/index.md) apresenta a linguagem e a equipe. O
[léxico](docs/analisador_lexico/analisador-lexico.md) documenta o catálogo de tokens,
e o [sintático](docs/analisador_sintatico/analisador-sintatico.md) detalha a
gramática EBNF, a precedência, o tratamento de erros e as limitações. O [registro das sprints](docs/sprints.md) detalha o planejamento,
as entregas e as pendências da equipe.

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
        <img src="https://github.com/Antedeguemon21.png" width="100" alt="Antedeguemon21"/><br />
        <sub><b>Antedeguemon21</b></sub>
      </a>
    </td>
  </tr>
</table>
