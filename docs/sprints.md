
# Estado do projeto

O MiniPy é um interpretador didático de um subconjunto de Python, implementado em C com Flex e Bison. O projeto recebe um programa, realiza análise léxica, sintática e semântica, constrói uma AST e executa os comandos válidos.

O pipeline atual é:

```text
Código-fonte
     ↓
Scanner (Flex)
     ↓
Parser (Bison)
     ↓
Construção da AST
     ↓
Análise semântica
     ↓
Dobramento de constantes
     ↓
Interpretador
     ↓
Saída ou diagnóstico
```

O modo padrão executa somente depois que as análises terminam sem erros. Os modos `--ast`, `--ast-otimizada` e `--ast-tipos` permitem inspecionar a estrutura do programa sem executá-lo. O executável `tokens` permite observar o scanner isoladamente.

## Recursos disponíveis

| Área | Comportamento implementado |
| --- | --- |
| Léxico | Identificadores, inteiros, decimais, textos, palavras reservadas, operadores, comentários e localização de erros |
| Sintaxe | Precedência de operadores, listas, atribuições, condicionais, laços, funções, chamadas e recuperação de erros |
| AST | Nós dos recursos da linguagem, linhas de origem, sequências, impressão e liberação |
| Semântica | Tabela de símbolos, tipos conhecidos, escopos, nomes, aridade e contexto de `return`, `break` e `continue` |
| Otimização | Dobramento em pós-ordem de operações `+`, `-` e `*` entre literais inteiros quando o resultado cabe em `int` |
| Valores e expressões | Inteiros, reais, booleanos, textos, listas, `None`, funções, coerção, aritmética e impressão |
| Textos | Concatenação, repetição por inteiro ou booleano e indexação para leitura |
| Condições | Comparações, conversão para verdadeiro/falso, curto-circuito e seleção de ramos |
| Laços | `while`, `for` em listas, textos e `range`, aninhamento, `break` e `continue` |
| Listas | Listas vazias, aninhadas e heterogêneas, referências compartilhadas, índices positivos e negativos, alteração de elementos, concatenação e repetição |
| Atribuições | Atribuição simples e composta, incluindo `+=`, `-=`, `*=` e `/=` |
| Funções e builtins | Definição, argumentos, retorno, recursão, aliases, `print`, `range` e `len` |
| Diagnósticos | Erros léxicos, sintáticos, semânticos e de execução com localização |

### Operações com listas

A concatenação `lista_a + lista_b` produz uma nova lista e preserva as listas originais. A repetição `lista * inteiro` e `inteiro * lista` produz uma lista com os elementos repetidos. As atribuições compostas `+=` e `*=` também estão disponíveis.

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

Uma operação entre tipos incompatíveis, como `[1] + 1`, produz erro de execução com indicação da linha.

### Indexação e tamanho

Listas e textos aceitam índices inteiros positivos e negativos. A indexação de textos é somente para leitura, pois textos são imutáveis. A função embutida `len()` retorna a quantidade de elementos de uma lista ou o tamanho do texto em bytes.

```text
lista = [10, 20, 30]
texto = "MiniPy"

print(lista[-1], len(lista))
print(texto[0], texto[-1], len(texto))
```

Saída:

```text
30 3
M y 6
```

A indexação e a iteração de textos percorrem bytes, sem decodificar caracteres UTF-8.

### Dobramento de constantes

Após a análise semântica, o otimizador percorre a AST em pós-ordem e simplifica operações `+`, `-` e `*` entre literais inteiros quando o resultado cabe em `int`.

Por exemplo:

```text
x = (1 + 2) * (3 + 4)
print(x)
```

A AST otimizada substitui as duas somas e a multiplicação por um único literal `21`. Operações com reais, divisão, resto, potência, menos unário e resultados fora do intervalo de `int` permanecem para o interpretador.

Consulte [Dobramento de constantes](otimizacao.md) para ver a AST antes e depois e os limites da transformação.

## Histórico das Sprints

O histórico abaixo reúne as entregas registradas no repositório e nas pull requests incorporadas. Nas Sprints 1 a 4, a organização é apresentada pelo conjunto de funcionalidades desenvolvido; não foram encontradas atas completas para todas as reuniões dessas etapas.

### Sprint 1 — Base léxica, sintática e expressões

- Estrutura inicial do projeto com Flex, Bison, GCC e Makefile.
- Criação do parser e integração com o scanner.
- Reconhecimento de literais inteiros, reais, textos, booleanos e `None`.
- Implementação de operadores aritméticos, precedência, divisão inteira, módulo e potência.
- Criação do modo `tokens` para inspeção dos tokens reconhecidos.
- Organização das primeiras regras e dos arquivos de compilação.

### Sprint 2 — Condições, lógica e composição da linguagem

- Ampliação do scanner e do parser para expressões condicionais.
- Integração de comparações e operadores lógicos.
- Estruturação de condicionais com `if`, `elif` e `else`.
- Avanço na integração das regras gramaticais entre as partes do projeto.
- Preparação da linguagem para avaliar expressões e selecionar blocos de comandos.

### Sprint 3 — Laços e automação de testes

- Estrutura geral de execução no parser, com programa, elementos, comandos e blocos.
- Regras para `while`, `for`, `break` e `continue`.
- Inclusão de verificações semânticas para impedir comandos de controle de fluxo fora de seus contextos válidos.
- Integração dos laços com as expressões da linguagem.
- Criação do script `testes/run_tests.sh` e do alvo `make test`.
- Ampliação dos casos válidos e inválidos para verificar os componentes da linguagem.

### Sprint 4 — Atribuições, listas e integração do interpretador

- Reconhecimento de atribuições simples e compostas: `=`, `+=`, `-=`, `*=` e `/=`.
- Suporte léxico a delimitadores de listas e comentários com `#`.
- Integração das regras de laços, expressões, atribuições e listas.
- Desenvolvimento das operações de leitura e alteração de elementos por índice.
- Ajustes na validação de índices e na divisão composta por valores reais.
- Organização dos módulos de AST, semântica e execução para integrar os componentes do interpretador.

### Sprint 5 — Recursos adicionais e testes integrados

A Sprint 5 ampliou o comportamento do interpretador e a cobertura da suíte.

- **Dobramento de constantes — #23:** otimização da AST após a análise semântica e criação do modo `--ast-otimizada`.
- **Operações com listas — #24:** concatenação com `+`, repetição com `*`, suporte a `+=` e `*=`, preservação dos operandos e erros para tipos incompatíveis.
- **Indexação de textos — #25:** leitura de caracteres por índices positivos e negativos, mantendo a imutabilidade dos textos.
- **Função `len()` — #26:** tamanho de listas e textos, com verificação de argumentos e tipos.
- **Testes integrados — #27:** programas completos que combinam funções, laços, listas, condições e expressões aritméticas.

As entregas de recursos foram incorporadas à `main` por pull requests. A suíte integrada passou a verificar o funcionamento de funcionalidades em conjunto, além dos casos isolados.

### Sprint 6 — Documentação, exemplos e validação final

A Sprint 6 foi planejada para consolidar a documentação, facilitar o uso do interpretador e preparar a entrega do projeto.

- **Documentação da otimização — #28:** criação da página sobre dobramento de constantes, com exemplos de AST original e otimizada.
- **README e estado do projeto — #29:** atualização das funcionalidades, limites, números de testes e histórico das Sprints 1 a 6.
- **Exemplos e manual de uso — #30:** criação de cinco programas comentados e de uma página com instruções de compilação, execução, modos do programa e interpretação de erros.
- **Validação final — #31:** execução das verificações em Linux e registro dos resultados em `docs/testes.md`.
- **Cabeçalhos institucionais — #33:** padronização dos comentários de identificação e descrição dos arquivos-fonte.
- **Entrega final — #32:** limpeza do repositório, resolução de pendências, atualização do GitHub Pages e publicação da versão final.

Na data de **26 de novembro de 2026**, as issues #28, #30 e #31 estavam fechadas. A atualização desta página corresponde à issue #29. As issues #32 e #33 ainda apareciam abertas no GitHub.

## Validação registrada

A validação final foi registrada em **26 de novembro de 2026**, em ambiente Linux.

| Verificação | Resultado |
| --- | --- |
| Compilação com GCC, Flex e Bison | Sucesso |
| Suíte geral | 125/125 programas aprovados |
| Programas válidos | 56/56 aprovados |
| Programas inválidos | 69/69 rejeitados corretamente |
| Verificações do scanner | 3 aprovadas |
| Verificador B | 67 expectativas conferidas sem divergências |
| Verificações adicionais do verificador B | 9 aprovadas |
| Testes P4 com runtime e memcheck | Aprovados |
| Testes de valores P1 com memcheck | Aprovados |
| `make memcheck` | Código de saída 0 |

Os testes de memória foram realizados com Valgrind no Linux. O registro completo dos critérios, comandos e limitações está em [Testes e validação](testes.md).

## Limites do produto

- O MiniPy implementa um subconjunto didático de Python. Os blocos usam `{ }` e não dependem de indentação significativa.
- Inteiros de execução usam `long long`; reais usam `double`. Não há inteiros de precisão arbitrária.
- O dobramento de constantes é limitado a `+`, `-` e `*` entre literais inteiros cujo resultado cabe em `int`.
- Textos são imutáveis. A indexação e a iteração percorrem bytes, sem decodificação de caracteres UTF-8.
- Não há fatiamento, indexação encadeada nem métodos de listas como `append`.
- Listas compartilham referências. A contagem de referências não coleta ciclos entre listas.
- A comparação de estruturas cíclicas também possui limitações.
- A busca de variáveis durante a execução consulta o quadro atual e o global. Funções não implementam closures que preservem variáveis locais de chamadas encerradas.
- Cada laço admite até 1.000.000 de iterações; chamadas aninhadas são limitadas a 1000 níveis.
- `range` produz uma lista, exige argumentos inteiros e recusa passo zero.

Os detalhes de valores, memória e execução estão em [Interpretador](interpretador.md). As escolhas da arquitetura estão em [Decisões técnicas](decisoes.md).

## Documentação e automação

As fontes do site ficam em `docs/`, com navegação definida em `mkdocs.yml`. A geração local usa:

```sh
python -m mkdocs build --strict
```

O workflow de testes define compilação, execução e verificações de memória. O workflow de documentação define a geração e publicação do site.

A geração local confirma a documentação no ambiente utilizado, mas não altera o conteúdo publicado nem comprova uma execução remota.

A compilação e os comandos de execução estão em [Código-fonte e build](src.md); os resultados da suíte estão em [Testes e validação](testes.md).
