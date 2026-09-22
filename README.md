# comp1-2026-02-grupo13

Interpretador de Python — um subconjunto da linguagem, apelidado de
"MiniPy" —, implementado em C com Flex (análise léxica) e Bison (análise
sintática). Equipe 13.

Você também pode ver a documentação completa no GitHub Pages: <!-- TODO: link do GitHub Pages -->

## Linguagens e ferramentas usadas

- **C** — linguagem de implementação do interpretador
- **Flex** — geração do analisador léxico (`scanner.l`)
- **Bison** — geração do analisador sintático (`parser.y`)
- **Make** — automação do build
- **Python (subconjunto)** — linguagem-alvo interpretada ("MiniPy")

## Requisitos

- `gcc` >= 16.2.0
- `flex` >= 2.6.4
- `bison` >= 3.8.2
- `make` >= 4.4.1

## Estrutura do projeto

- `Lexico/` — analisador léxico (Flex, `scanner.l`)
- `Sintatica/` — analisador sintático (Bison, `parser.y`)
- `Semantica/` — analisador semântico e AST (ainda não implementados)
- `src/` — ponto de entrada do interpretador (esqueleto, ainda não integrado)
- `testes/` — testes automatizados
- `docs/` — documentação complementar do projeto

## Status desta entrega

Esta primeira entrega do interpretador cobre apenas o **léxico**, o
**sintático** e o **Makefile** de build. Semântica, AST e execução ainda
não existem.

| Etapa | Conteúdo | Status |
|---|---|---|
| Expressões e literais | números, strings, `True`/`False`/`None`, aritmética | ✅ |
| Condições e lógica | comparações, `if`/`elif`/`else` | 🚧 reservado na gramática, não implementado |
| Laços | `while`, `for`, `break`, `continue` | ✅ (reconhecimento sintático; sem execução) |
| Atribuição, listas e comentários | `=`, `+=`/`-=`/`*=`/`/=`, `[...]`, `l[i]`, `#` | ✅ |
| Funções e chamadas | `def`, `return`, chamadas com argumentos | 🚧 reservado na gramática, não implementado |

## Como compilar

```sh
make parser   # gera o parser (bison + flex + gcc)
make tokens   # gera um binário que só imprime os tokens lidos
```

`make clean` remove os artefatos gerados (`parser.tab.c/.h`, `lex.yy.c`,
`parser`, `tokens`).

## Como executar

```sh
./parser < caminho/para/arquivo.py   # valida a gramática do arquivo
./tokens < caminho/para/arquivo.py   # imprime a lista de tokens lidos
```

## Como gerar um arquivo Python de teste

Como o interpretador ainda cobre só léxico e sintático, "gerar Python"
aqui significa escrever um arquivo `.py` de teste usando o subconjunto
já suportado (expressões, laços, atribuição, listas e comentários):

```python
# exemplo.py
x = 0
lista = [1, 2, 3]
while x < 3 {
    x += 1
}
```

## Como compilar e executar o Python gerado

Com o arquivo `.py` de teste em mãos, compile o interpretador e rode-o
sobre esse arquivo:

```sh
make parser
./parser < exemplo.py
```

## Documentação

- [Analisador Léxico](docs/analisador_lexico/analisador-lexico.md)
- [Analisador Sintático](docs/analisador_sintatico/analisador-sintatico.md)
- [Analisador Semântico](docs/analisador_semantico/analisador-semantico.md) (futuro)

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
  </tr>
</table>

