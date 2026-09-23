# MiniPy — Equipe 13

MiniPy é um interpretador didático de um subconjunto de Python, com blocos
delimitados por `{ }`, implementado em **C + Flex + Bison** pela Equipe 13.
O objetivo é estudar a construção incremental do léxico e da gramática antes
de implementar AST, tabela de símbolos e execução.

Esta entrega fecha o escopo técnico da **Semana 5: parser e recuperação de erros**.
Expressões numéricas são calculadas; as cinco fatias reconhecem as demais
construções e mostram `[OK]`. PC1 significa Ponto de Controle 1; P1–P5
significam fatias, não pontos de controle.


## Tecnologias

C implementa o projeto; Flex gera o scanner e Bison gera o parser. Make automatiza o build. MkDocs com Material publica esta documentação no GitHub Pages.

- [Léxico](analisador_lexico/analisador-lexico.md)
- [Sintático](analisador_sintatico/analisador-sintatico.md)
- [Semântico: Semana 6 em diante](analisador_semantico/analisador-semantico.md)
- [Código-fonte e build](src.md)
- [Testes](testes.md)
- [Sprints](sprints.md)

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

O [README](https://github.com/Compiladores-1-Grupo13-2026/Compiladores-1/blob/main/README.md) apresenta o projeto e as instruções para compilar e executar.

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
