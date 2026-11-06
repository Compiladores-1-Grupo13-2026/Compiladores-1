# Estado do projeto

O MiniPy constrói a AST, realiza análise semântica e executa o programa.
Scanner, parser e módulos do interpretador participam do mesmo pipeline.

## Recursos disponíveis

| Área | Comportamento implementado |
| --- | --- |
| Léxico | Identificadores, inteiros/decimais, strings, palavras reservadas, operadores, comentários e localização |
| Sintaxe | Precedência, listas, atribuições, condicionais, laços, funções, chamadas e recuperação |
| AST | Nós de todos os recursos, linhas, sequências, impressão e liberação |
| Semântica | Símbolos, tipos conhecidos, escopos, nomes, aridade e contexto de `return`, `break` e `continue` |
| Valores e expressões | Inteiros, reais, booleanos, strings, listas, `None`, funções, coerção, aritmética e impressão |
| Condições | Comparações, verdadeiro/falso, curto-circuito e seleção de ramos |
| Laços | `while`, `for` em listas/textos/`range`, aninhamento e sinais de controle |
| Ambiente e listas | Quadros de chamada, busca atual/global, referências compartilhadas, atribuição e índices negativos |
| Funções e builtins | Definição, argumentos, retorno, recursão, aliases, `print` e `range` |
| Erros dinâmicos | Linha e encerramento controlado para tipos inválidos, divisor zero, índices, limites e chamadas |

```text
entrada → scanner → parser → AST → análise semântica → interpretador
```

O modo padrão executa somente após as análises passarem. `--ast` mostra
reconhecimentos e árvore sem executar. O executável `tokens` inspeciona o
scanner isoladamente. Expressões soltas são avaliadas e descartadas; chamadas
como `print` produzem a saída do programa.

## Validação registrada

Em **6 de novembro de 2026**, a compilação passou sem avisos e a suíte
terminou com **108/108 programas aprovados**, mais três verificações de
tokens, no Windows e no Linux. Os quatro executáveis unitários passaram.

O Valgrind verificou os 46 válidos e os quatro unitários; `make memcheck`
retornou 0. O AddressSanitizer verificou 108 programas e quatro unitários
sem diagnóstico de acesso inválido. Os critérios e limites dessas ferramentas
estão em [Testes e validação](testes.md).

As referências históricas já refletem o modo AST. O teste CRLF foi alinhado
à entrada preservada no Linux; esses ajustes fazem parte da suíte atual.

## Limites do produto

- A linguagem é um subconjunto didático de Python, com blocos `{ }` e sem
  indentação significativa.
- Inteiros de execução usam `long long`; estouros suportados são interrompidos
  com erro controlado. Reais usam `double`.
- Cada laço permite até 1.000.000 de iterações; chamadas aninhadas são limitadas
  a 1000 níveis.
- `range` produz uma lista, exige argumentos de tipo inteiro e recusa passo zero.
- A indexação é de listas e exige `int`. Não há indexação de strings, fatiamento,
  atributos ou métodos como `append`.
- O ambiente procura no quadro atual e no global. Não implementa closures que
  conservem locais de uma chamada encerrada.
- Listas compartilham referências, mas não há coleta de ciclos. A comparação
  de estruturas cíclicas também tem limitações.
- Concatenação e repetição de listas não fazem parte da aritmética implementada.
  A aceitação de alguns tipos na análise estática não garante suporte à
  operação durante a execução.

Os valores e sinais são detalhados no [Interpretador](interpretador.md).
As escolhas entre análise e execução estão em [Decisões técnicas](decisoes.md).

## Documentação e automação

As fontes do site ficam em `docs/`, com navegação em `mkdocs.yml`. A geração
local usa `mkdocs build --strict`. O workflow de testes define compilação,
execução e memória; o de documentação define a geração e publicação do site.

Esta página descreve os arquivos e as validações locais. Gerar o site
localmente não altera o conteúdo publicado nem comprova uma execução remota.

