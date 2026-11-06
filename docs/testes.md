# Testes e validação

A suíte verifica o MiniPy desde os tokens até a execução da AST. São **108
programas: 46 válidos e 62 inválidos**, além de três verificações do scanner.

```sh
make test
```

O alvo compila `parser` e `tokens` e chama `testes/run_tests.sh`. O resumo
esperado da suíte atual é:

```text
validos: 46/46 passaram.
invalidos: 62/62 passaram.
Total de falhas: 0
```

## Modos e critérios

O nome do arquivo determina o modo do parser usado pelo runner:

| Conjunto | Quantidade | Modo | Verificação |
| --- | ---: | --- | --- |
| Históricos | 66: 25 válidos e 41 inválidos | `--ast` | Reconhecimento, AST, semântica e recuperação |
| Nomes contendo `_exec_` | 42: 21 válidos e 21 inválidos | Execução padrão | Saída, operações, fluxo e erros dinâmicos |
| Scanner isolado | 3 verificações | `tokens` | Catálogo, programa de referência e caractere inválido |

| Caso | Critério de aprovação |
| --- | --- |
| `testes/validos/*.txt` | Código 0 e `stderr` vazio |
| `testes/invalidos/*.txt` | Código não zero, sem timeout ou falha de inicialização, e diagnóstico com tipo e linha |
| Inválido com `_exec_` | Código de saída exatamente 1 |
| Caso com `.esperado` | Correspondência exata da saída combinada, após normalizar CRLF |

Cada execução tem limite de cinco segundos. As referências são comparadas
quando existem; alguns históricos de laços verificam apenas status e
diagnósticos. As entradas antigas `.mpy` não entram na varredura geral `.txt`.

O runner não atualiza expectativas. Uma segunda execução captura `stdout` e
`stderr` juntos para conservar a ordem: concatená-los depois não preservaria
a sequência dos diagnósticos e das saídas.

## Cobertura por recurso

| Recurso | Cenários cobertos |
| --- | --- |
| Expressões e valores | Precedência, unários, inteiros, reais, booleanos, `None`, escapes, operações numéricas e de strings |
| Condições | Comparações, igualdade de listas, curto-circuito, verdade e `if`/`elif`/`else` |
| Laços | Listas, textos, `range`, avaliação única do iterável, aninhamento, `break`, `continue` e retorno |
| Variáveis e listas | Reatribuição, compostas, compartilhamento, índices negativos, listas aninhadas e escopos |
| Funções | Argumentos, retorno, ausência de retorno, recursão simples/mútua, aliases, `print` e `range` |
| Limites dinâmicos | Divisor zero, tipos incompatíveis, estouro inteiro, índices, passo zero, iterações e chamadas |
| Análise e recuperação | Léxico inválido, construções incompletas, nomes, aridade e contexto de controle de fluxo |

## Testes unitários e de integração

Os verificadores abaixo usam dependências reais e exigem Python e GCC:

```sh
python3 testes/testar_p4.py --runtime
python3 testes/testar_valores.py
python3 testes/verificar_b.py
```

O primeiro comando compila três executáveis para tabela, semântica e contratos
de execução. O segundo acrescenta o quarto executável, com precisão acima de
`2**53`, fronteiras de `long long`, estouros controlados e posse de valores.
O último confere 62 resultados e nove verificações de integração, incluindo
o bloqueio da execução após erros de análise e o programa de referência.

Para observar a recuperação sem executar:

```sh
./parser --ast < testes/invalidos/base_multiplos_erros.txt 2>&1
echo "$?"  # 1
```

Nesse modo, os comandos recuperados aparecem na AST. No modo padrão, qualquer
erro léxico, sintático ou semântico impede toda a execução, inclusive um
`print` escrito antes do erro no arquivo.

## Memória

```sh
make memcheck
python3 testes/testar_p4.py --runtime --memcheck
python3 testes/testar_valores.py --memcheck
```

`make memcheck` cobre os válidos de expressões e todos os válidos contendo
`_exec_`, respeitando AST e execução. Prefere Valgrind e usa AddressSanitizer
como alternativa quando compilador e sistema oferecem o runtime necessário.
Os verificadores Python com `--memcheck` exigem Valgrind.

Na validação local de **6 de novembro de 2026**:

| Verificação | Resultado |
| --- | --- |
| Compilação GCC, Flex e Bison | Sucesso com `-Wall -Wextra -Werror` no compilador e `-Wall -Werror` no Bison |
| Suíte no Windows e no Linux | 108/108 programas e três verificações de tokens passaram |
| Testes unitários | Quatro executáveis passaram |
| AddressSanitizer Windows | 108 programas e quatro unitários passaram sem diagnóstico de acesso inválido |
| Valgrind 3.25.1 Linux | 46/46 válidos e quatro unitários passaram |
| `make memcheck` Linux | Código de saída 0 |

O Valgrind verificou acessos inválidos e blocos definitivamente, indiretamente
ou possivelmente perdidos. Buffers globais do Flex permanecem alcançáveis até
o encerramento; eles não são blocos perdidos. Os unitários verificam todos
os tipos de vazamento.

O AddressSanitizer usado no Windows não oferece LeakSanitizer; os vazamentos
foram verificados no Linux. O contrato admite alocações temporárias não
liberadas ao interromper um erro de execução com `longjmp`. As limitações
de ciclos em listas estão em [Decisões técnicas](decisoes.md).

## Resultados históricos e CRLF

As referências históricas registram reconhecimentos, AST e diagnósticos de
`--ast`. O script `testes/atualizar_esperados_a.py` regenera somente o conjunto
histórico predefinido nele; não é executado automaticamente por `make test`.

As expectativas usam LF. O teste `base_crlf.txt` conserva CRLF na entrada e
tem diagnóstico na **coluna 5**, confirmado no Linux. O stdin textual de um
executável Windows pode descartar `\r` e produzir coluna 4. Remover CR da
saída não corrige essa diferença: a validação deve preservar os bytes de
entrada. Gere referências no Linux ou em um ambiente com essa preservação.

## Automação

O workflow `.github/workflows/testes.yml` define compilação, suíte geral,
verificadores Python e memória em Ubuntu. Este registro descreve execuções
locais e não afirma o estado de uma execução remota.

Os requisitos e comandos de build estão em [Código-fonte e build](src.md).
