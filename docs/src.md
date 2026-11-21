# Código-fonte e build

O projeto usa C, Flex e Bison. O `main` está em `Sintatica/parser.y` e
coordena o parser, a análise semântica, a execução e a liberação dos recursos.
`src/main.c` está vazio e não é o ponto de entrada do executável.

## Organização do código

| Caminho | Função |
| --- | --- |
| `Lexico/scanner.l` | Tokens, comentários, terminadores, localização e diagnóstico léxico |
| `Sintatica/parser.y` | Gramática, AST, recuperação sintática e modos de operação |
| `Semantica/Ast/ast.c` e `ast.h` | Nós, construtores, acesso numérico, impressão e liberação da AST |
| `Semantica/Tabela/tabela.c` e `tabela.h` | Símbolos, categorias, tipos e escopos estáticos |
| `Semantica/semantico.c` e `semantico.h` | Percurso semântico, nomes, tipos, chamadas e controle de fluxo |
| `Interpretador/interpretador.c` e `interpretador.h` | Despacho, blocos, sinais e interrupção de erros |
| `Interpretador/valor.c` e `valor.h` | Representação, cópia, liberação e impressão de valores |
| `Interpretador/exec_expressoes.c` | Literais, escapes, operações numéricas e de strings |
| `Interpretador/exec_condicoes.c` | Comparações, curto-circuito e condicionais |
| `Interpretador/exec_lacos.c` | Iteração, laços, `break` e `continue` |
| `Interpretador/ambiente.c` e `ambiente.h` | Quadros de variáveis da execução |
| `Interpretador/lista.c` | Vetor dinâmico de valores e liberação de listas |
| `Interpretador/exec_atribuicao.c` | Variáveis, atribuições, listas e indexação |
| `Interpretador/exec_funcoes.c` | Definições, chamadas, parâmetros, retorno, `print` e `range` |
| `testes/` | Entradas, referências e verificadores de unidades, integração e memória |
| `docs/`, `mkdocs.yml`, `requirements-docs.txt` | Conteúdo, navegação e dependências da documentação |

Veja a [AST](ast.md), a [análise semântica](analisador_semantico/analisador-semantico.md)
e o [interpretador](interpretador.md) para os contratos entre as etapas.

## Compilar em Linux

São necessários GCC, GNU Make, Flex, Bison e as bibliotecas de desenvolvimento
de Flex e de matemática. A suíte usa `sh`, `timeout`, `diff`, `grep` e `tr`;
os verificadores adicionais usam Python. Em Debian/Ubuntu:

```sh
sudo apt install build-essential flex bison libfl-dev python3 python3-venv valgrind
make clean
make
make tokens
make test
```

O Makefile usa alvos agrupados, disponíveis no GNU Make 4.3 ou posterior.
Bison usa `-Wall -Werror` e o compilador, `-Wall -Wextra`. Para tornar avisos
do compilador fatais também:

```sh
make CFLAGS='-Wall -Wextra -Werror'
```

Exemplo pelo stdin:

```sh
printf 'print(3 + 4 * 2)\n' | ./parser
```

Saída:

```text
11
```

## Alvos e modos

| Comando | Resultado |
| --- | --- |
| `make` | Gera e compila `parser` |
| `make tokens` | Compila o scanner isolado com `SO_TOKENS` |
| `./parser < programa.txt` | Analisa e executa se não houver erro de análise |
| `./parser --ast < programa.txt` | Mostra reconhecimentos e AST, sem executar |
| `./tokens < programa.txt` | Mostra os tokens |
| `make run ARQ=programa.txt` | Compila e executa o arquivo indicado |
| `make test` | Executa a suíte geral |
| `make memcheck` | Verifica memória nos válidos selecionados |
| `make clean` | Remove os artefatos previstos no Makefile |

`programa.txt` representa um arquivo criado pelo usuário. Os verificadores
estão detalhados em [Testes e validação](testes.md).

O build gera `parser.tab.c`, `parser.tab.h`, `parser.output`, `lex.yy.c` e
o executável. São arquivos derivados: altere a linguagem em `scanner.l` ou
`parser.y`, e não nas fontes C geradas.

O código de saída é 0 para sucesso e 1 para erro léxico, sintático, semântico
ou de execução. Diagnósticos vão para `stderr`. Um erro de execução preserva
a saída já impressa; erros de análise impedem a execução.

## Windows e MSYS2

Com as ferramentas disponíveis no MSYS2, o scanner pode dispensar `libfl`,
pois usa `noyywrap`:

```sh
make parser tokens LDLIBS=-lm
```

Para exercitar o limite de chamadas com executável Windows, reserve uma pilha
maior:

```sh
make parser tokens LDLIBS=-lm LDFLAGS='-Wl,--stack,8388608'
```

As expectativas usam LF. O stdin textual do Windows pode traduzir CRLF e
alterar colunas do scanner; a validação deve preservar os bytes de entrada.
Consulte o [teste CRLF](testes.md#resultados-historicos-e-crlf) antes de gerar
referências fora do Linux.

## Gerar a documentação

Na raiz do repositório:

```sh
python3 -m venv .venv-docs
.venv-docs/bin/python -m pip install -r requirements-docs.txt
.venv-docs/bin/mkdocs build --strict
```

No Windows, os executáveis do ambiente virtual ficam em `.venv-docs\Scripts\`.
O build produz `site/`. Para visualizar localmente:

```sh
.venv-docs/bin/mkdocs serve
```

Abra `http://127.0.0.1:8000/Compiladores-1/` e use `Ctrl+C` para encerrar.

O workflow `.github/workflows/gh-pages.yml` define a geração estrita e a
publicação de `site/`. Edição e build locais apenas preparam o conteúdo;
não publicam alterações no site remoto.
