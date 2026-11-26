# Testes e validação

A suíte verifica o MiniPy desde os tokens até a execução da AST. São 125
programas: 56 válidos e 69 inválidos, além de três verificações do scanner.

```sh
make test
```

O alvo compila `parser` e `tokens` e chama `testes/run_tests.sh`. O resumo
esperado da suíte atual é:

```text
validos: 56/56 passaram.
invalidos: 69/69 passaram.
Total de falhas: 0
```

## Modos e critérios

O nome do arquivo determina o modo do parser usado pelo runner:

| Conjunto | Modo | Verificação |
| --- | --- | --- |
| Arquivos válidos | Conforme o nome do arquivo | Reconhecimento, AST, semântica e execução |
| Arquivos inválidos | Conforme o nome do arquivo | Diagnósticos léxicos, sintáticos e semânticos |
| Scanner isolado | `tokens` | Catálogo, programa de referência e caractere inválido |

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
python3 testes/testar_p4.py --runtime --memcheck
python3 testes/testar_valores.py --memcheck
python3 testes/verificar_b.py
```

O primeiro comando verifica tabela, semântica, AST e contratos de execução
com dependências reais, incluindo testes com Valgrind. O segundo verifica
precisão inteira, limites e posse de valores. O último confere 67 expectativas
e nove verificações de integração, incluindo o bloqueio da execução após
erros de análise e o programa de referência.

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

## Validação final no Linux — 26 de novembro de 2026

| Verificação | Resultado |
| --- | --- |
| Compilação com GCC, Flex e Bison | Sucesso |
| Suíte geral | 125/125 programas aprovados e três verificações de tokens aprovadas |
| Verificador B | 67 expectativas conferidas, sem divergências; nove verificações adicionais aprovadas |
| Testes P4 com runtime e memcheck | Aprovados; Valgrind sem erros nos módulos exercitados |
| Testes de valores P1 com memcheck | Aprovados |
| `make memcheck` | Código de saída 0; execução dos casos selecionados com Valgrind |

Todas as verificações acima foram executadas localmente em Linux. O alvo
`make memcheck` utiliza Valgrind quando disponível e mantém a saída silenciosa
quando não há erros.

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
