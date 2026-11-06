# MiniPy — Equipe 13

MiniPy é um interpretador didático de um subconjunto de Python, com blocos
delimitados por `{ }`, implementado em **C + Flex + Bison** pela Equipe 13.
O objetivo é estudar a construção incremental do léxico e da gramática antes
de implementar AST, tabela de símbolos e execução.

O parser principal fecha o escopo técnico da **Semana 5: parser e recuperação de erros**.
Expressões numéricas são calculadas; as cinco fatias reconhecem as demais
construções e mostram `[OK]`.

Na **Sprint 3 (AST e semântica)**, o parser monta uma AST única do programa,
imprime essa árvore e a análise semântica a percorre usando a tabela de símbolos
com escopos por função (P4 e P5). Veja [Funções e integração (P5)](docs/p5.md).

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
- `print` e `range` são identificadores comuns. Chamadas são reconhecidas, sem execução.
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

Compile e execute:

```sh
make clean
make
make tokens
./tokens < testes/validos/base_programa_referencia.txt
./parser < testes/validos/base_programa_referencia.txt
make test
```

`make` gera `parser.tab.c/.h`, `parser.output`, `lex.yy.c` e `parser`.
`make tokens` compila o mesmo scanner com `SO_TOKENS`; `make clean` remove os
artefatos. Eles são ignorados pelo Git. Para ver a recuperação e o código de saída:

```sh
./parser < testes/invalidos/base_multiplos_erros.txt 2>&1
echo "$?"  # 1
```

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

A entrega cobre léxico, parser e recuperação de erros da Semana 5 e, na Sprint 3,
a construção da AST completa, a tabela de símbolos e a análise semântica integradas
ao parser principal. O programa ainda não é executado (funções não rodam).
A execução e as fatias dos demais integrantes permanecem fora desta contribuição.
O parser lê stdin; arquivos de teste da entrega anterior usam `.txt`.

Para validar os módulos P4 no Windows (GCC e Python no PATH):

```powershell
python testes/testar_p4.py
```

Em Linux, use `python3` ou `make test-p4`. O comando executa somente os dois
testes unitários da P4. A suíte geral `make test` continua com a P3.
As novas entradas `.mpy` e saídas esperadas estão em `testes/validos/` e
`testes/invalidos/`, preparadas
para integração futura; elas não são executadas pelo runner unitário.
Para conferir memória dos módulos testados em Linux com Valgrind:
`make memcheck-p4`. Os testes usam nós pré-montados e simulações mínimas das
dependências; não fornecem o núcleo de AST, parser ou semântica dos colegas.
A contribuição fica identificada por comentários `[P4]` em `ast.c`, `ast.h`,
`semantico.c` e `semantico.h`. Os tipos e o núcleo comum da AST aguardam a P1.

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
