# Código-fonte e build

O build usa [Lexico/](https://github.com/Compiladores-1-Grupo13-2026/Compiladores-1/tree/main/Lexico) e [Sintatica/](https://github.com/Compiladores-1-Grupo13-2026/Compiladores-1/tree/main/Sintatica). `main` está em `parser.y`. [src/main.c](https://github.com/Compiladores-1-Grupo13-2026/Compiladores-1/blob/main/src/main.c) e [Semantica/](https://github.com/Compiladores-1-Grupo13-2026/Compiladores-1/tree/main/Semantica) são esqueletos futuros, sem integração.

Ambiente validado: GCC 13.3.0, Flex 2.6.4, Bison 3.8.2, GNU Make e GNU coreutils
(`timeout`, usado para detectar travamentos nos testes). É necessário o pacote de
desenvolvimento de libfl e libm. Em Debian/Ubuntu:

```sh
sudo apt install build-essential flex bison libfl-dev python3-venv
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
