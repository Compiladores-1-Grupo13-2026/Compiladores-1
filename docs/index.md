# MiniPy — Equipe 13

MiniPy é um interpretador didático de um subconjunto de Python, implementado em
**C, Flex e Bison**. O programa lê o código, constrói uma árvore sintática,
verifica nomes e tipos e executa os comandos válidos.

A linguagem usa chaves para delimitar blocos e aceita comandos terminados por
quebra de linha ou ponto e vírgula. Há expressões, condicionais, laços, listas,
funções, recursão e as funções embutidas `print` e `range`.

## Um primeiro programa

```text
def fatorial(n) {
    if n <= 1 { return 1; }
    return n * fatorial(n - 1);
}

lista = [1, 2, 3]
lista[0] += 5
print(fatorial(5), lista)
for i in range(3) { print(i); }
```

Saída:

```text
120 [6, 2, 3]
0
1
2
```

Salve o código em `programa.txt` e execute, na raiz do projeto:

```sh
make
./parser < programa.txt
```

## Recursos da linguagem

| Recurso | Comportamento |
| --- | --- |
| Valores | Inteiros, reais, booleanos, textos, `None`, listas e funções |
| Expressões | `+`, `-`, `*`, `/`, `//`, `%`, `**` e menos unário |
| Condições | Comparações, `and` e `or` com curto-circuito, `not`, `if`/`elif`/`else` |
| Repetição | `while`, `for` em listas, textos e `range`, `break` e `continue` |
| Dados | Atribuição simples e composta, listas compartilhadas, leitura e alteração por índice |
| Funções | Parâmetros, chamadas, retorno, recursão e escopo local/global |
| Diagnósticos | Erros léxicos, sintáticos, semânticos e de execução com localização |

Identificadores distinguem maiúsculas e minúsculas e usam letras ASCII, números
e `_`, sem número inicial. Comentários começam com `#`. Os blocos não dependem
da indentação. `elif` e `else` ficam na mesma linha da chave `}` anterior.

## Executar e inspecionar

| Comando | Finalidade |
| --- | --- |
| `./parser < programa.txt` | Analisar e executar; mostrar os `print` e os erros |
| `./parser --ast < programa.txt` | Analisar e mostrar a AST, sem executar o programa |
| `./tokens < programa.txt` | Mostrar os tokens, após compilar com `make tokens` |
| `make test` | Executar a suíte de programas e as verificações de tokens |
| `make memcheck` | Verificar a memória dos casos selecionados pelo alvo |

A execução começa somente se as análises não encontrarem erros. Sucesso retorna
0; erro retorna 1. Um erro de execução preserva o que já foi impresso e interrompe
o programa no ponto da falha.

## Conhecer o projeto

- [Código-fonte e compilação](src.md): dependências, organização e comandos.
- [Interpretador](interpretador.md): valores, operações, controle de fluxo e memória.
- [Funções e chamadas](p5.md): parâmetros, retorno, recursão e funções embutidas.
- [Analisador léxico](analisador_lexico/analisador-lexico.md),
  [sintático](analisador_sintatico/analisador-sintatico.md) e
  [semântico](analisador_semantico/analisador-semantico.md): as etapas de análise.
- [AST](ast.md) e [decisões técnicas](decisoes.md): representação e arquitetura.
- [Testes](testes.md) e [estado do projeto](sprints.md): cobertura, resultados e limites.

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
