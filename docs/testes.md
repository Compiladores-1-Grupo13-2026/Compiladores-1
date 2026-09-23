# Testes

`make test` compila parser/tokens e chama `sh testes/run_tests.sh`.
São 18 entradas válidas, 38 inválidas e 3 verificações de tokens. Os cinco
testes antigos foram preservados em `validos/`, com prefixos de fatia;
`p1_original.txt` conserva o antigo teste de expressão atribuída a uma variável.

| Grupo | Critério |
|---|---|
| `validos/*.txt` | status 0 e stderr vazio |
| `invalidos/*.txt` | status diferente de 0, sem timeout/sinal, diagnóstico com tipo e `Linha N` |
| `*.esperado` | diff exato de stdout + stderr, preservando a ordem |
| modo tokens | catálogo, referência oficial e rejeição de caractere inválido |

Cada caso possui `.esperado`. O runner não atualiza expectativas: se o comportamento
for alterado intencionalmente, revise o resultado antes de editar esse arquivo.
Uma segunda execução por caso captura os dois descritores juntos; concatenar
os arquivos stdout e stderr separados não preservaria a ordem da demonstração.
Cada execução tem limite de cinco segundos. Falhas exibem o diff e produzem status 1;
pasta vazia também falha. Não são exigidas ferramentas Python para os testes.

## Cobertura por fatia

- P1: precedência, unários, números, strings, booleanos/None, parênteses, operandos,
  divisão inteira/resto por zero, potência fora do domínio, overflow e EOF sem newline.
- P2: if/elif/else, lógica, condição vazia/inválida, bloco ausente, else órfão e aninhamento.
- P3: while/for, break/continue com ambos os terminadores, falta de condição, variável,
  `in`, e bloco aberto no EOF.
- P4: atribuições simples/compostas, indexação, listas em várias linhas, alvo literal,
  falta de `]`/vírgula e continuação com expressão ou atribuição válida.
- P5: def, parâmetros, chamadas, return vazio/com expressão, cabeçalhos malformados,
  falta de `)` com `;` e NEWLINE, múltiplos erros no mesmo arquivo.
- Base: programa oficial, exemplo exato do §9.3, erro léxico, linha/coluna com
  espaços/tabs, comentários, CRLF e ausência de resultados falsos para IDs/chamadas.

## Demonstração de múltiplos erros

```sh
./parser < testes/invalidos/base_multiplos_erros.txt 2>&1
echo "$?"
```

```text
Resultado: 7
[ERRO SINTATICO] Linha 2, coluna 5: syntax error (proximo a ';')
Resultado: 42
[ERRO SINTATICO] Linha 4, coluna 7: syntax error (proximo a ';')
Resultado: 8
[ERRO SEMANTICO] Linha 6: divisao por zero
```

O código de saída é 1. Os erros estão nas linhas 2, 4 e 6; os resultados 7, 42 e 8
intercalados provam a recuperação.

[Entradas e expectativas no repositório](https://github.com/Compiladores-1-Grupo13-2026/Compiladores-1/tree/main/testes).
