# Interpretador — Sprint 4, Pessoa A

A Pessoa A implementa a infraestrutura, condições (P2), ambiente, listas e
atribuições (P4). Os três headers e `Interpretador/interpretador.c` são cópias
literais do contrato original. A execução de expressões, valores, laços e
funções pertence à Pessoa B.

## Base e estado da entrega

`sprint4-base` parte do commit existente `214176d`, da integração
`p5-funcoes-semantico` da Sprint 3. Essa integração está no histórico da base
e não aparece como alteração própria da Sprint 4. A `main` não foi alterada;
não foram criados commits novos, PRs ou publicações.

Faltam `Interpretador/valor.c`, `exec_expressoes.c`, `exec_lacos.c` e
`exec_funcoes.c`. As fontes A estão implementadas, mas a base conjunta ainda
precisa de compilação, teste e revisão cruzada. A página `p5.md` restaura o
link ausente que veio da integração Sprint 3.

## Fluxo e modos

O Flex fornece tokens ao Bison, que monta a AST. A semântica percorre a árvore
e a tabela de símbolos antes da execução. O modo padrão só chama o interpretador
quando não há erro léxico, sintático ou semântico; expressões soltas são descartadas.

```sh
make clean
make
./parser < programa.txt
./parser --ast < programa.txt
make test
```

`--ast` mostra reconhecimentos e AST sem executar. O modo padrão mostra os
`print` e os diagnósticos. Sucesso retorna 0; erro retorna 1. O runner usa
`--ast` nos históricos e execução em `_exec_`, normalizando CRLF nas saídas.

## Valores, memória e ambiente

`Valor` distingue None, int, real, bool, str, lista e função. Strings possuem
memória própria; nós de função pertencem à AST. Cada resultado de `interpretar_no`
pertence ao chamador, que deve liberá-lo. Operações aritméticas/comparações
emprestam operandos. `ambiente_definir` e `lista_adicionar` assumem valores;
`ambiente_buscar` empresta um ponteiro. Leituras devolvem `valor_copiar`.

`Lista` é um vetor que dobra a capacidade. A cópia de um valor lista compartilha
a referência; a liberação final libera seus elementos e vetor. As operações
`valor_copiar`/`valor_liberar`, que atualizam o contador, pertencem à Pessoa B.
O contrato não especifica coleta de ciclos em listas que contenham a si mesmas.

O ambiente mantém quadros com entradas `(nome, Valor)`. O global tem profundidade
zero; chamadas empilham/desempilham quadros. A busca vê o atual e o global,
ignorando os intermediários. Definições substituem somente entradas atuais;
a saída de um quadro libera seus nomes e valores.

## Condições, sinais e atribuições

None, False, zero, texto vazio e lista vazia são falsos; os demais são verdadeiros.
`and`/`or` usam curto-circuito e devolvem operandos; `not` devolve bool. Igualdade
compara números por valor, textos por conteúdo e listas elemento a elemento.
Ordem aceita números ou dois textos.

`if` executa o ramo escolhido; `elif` é outro `AST_IF` em `c`. O bloco interrompe
quando `sinal_atual` muda. O `if` preserva break/continue/return; seus consumidores
são os laços e chamadas da Pessoa B.

`=`, `+=`, `-=`, `*=` e `/=` operam em variáveis e elementos de lista; as contas
são delegadas a `operar_aritmetica`. Índices exigem int e aceitam negativos.
A troca de elemento libera o anterior e mantém a lista compartilhada. Uma
referência própria mantém o alvo vivo durante a avaliação.

```text
l = [1, 2]
l[0] += 5
b = l
b[1] = 9
print(l, l[-1])
```

Saída contratada: `[6, 9] 9`. As melhorias semânticas ficam somente em P2/P4:
consultam tipos conhecidos em comparações/índices, permitem igualdade entre
tipos distintos e preservam tipos fracionários e o resultado float de `/=`.

## Erros

`erro_execucao` descarrega stdout, escreve em stderr e interrompe com `longjmp`.
O prefixo é `[ERRO EXECUCAO] Linha N:`; a saída anterior é preservada. Exemplos:

```text
comparacao '<' entre tipos incompativeis ('str' e 'int')
indice 5 fora do intervalo (lista de tamanho 2)
indice de lista deve ser int, recebido 'float'
'int' nao pode ser indexado
```

Tipos conhecidos incompatíveis geram erro semântico antes de executar. Os testes
dinâmicos usam elementos de lista, cujo tipo estático é desconhecido. Conforme
o contrato, vazamentos somente no caminho de erro são aceitos.

## Testes e validação

```sh
python3 testes/testar_p4.py
python3 testes/testar_p4.py --runtime
python3 testes/testar_p4.py --runtime --memcheck
python3 testes/atualizar_esperados_a.py
make test
```

O runner único compila tabela e semânticas P4/P2 com a AST real, sem injetar
outro NoAST nem ligar os dublês antigos. `--runtime` acrescenta o teste de verdade,
comparações, escopos, referências, crescimento de listas, curto-circuito e sinais.
Usa dependências reais; `--memcheck` exige Valgrind.

Há 13 pares `.txt`/`.esperado`: três válidos P2, dois inválidos P2, três válidos
P4 e cinco inválidos P4. Os cenários válidos dos 17 casos iniciais foram agrupados
sem retirar trechos ou saídas. As expectativas foram especificadas pelo contrato
e ainda não foram confirmadas por um binário compilado.

A correção `x = 10;` em `p4_listas_multilinha.txt` foi aplicada. O script de
regeneração limita-se aos 21 históricos A, valida os status antes de escrever
e normaliza CRLF. Sua execução ainda depende de um parser compilado.

| Verificação realizada | Resultado |
| --- | --- |
| `make` / `make test` | Não iniciaram: Make ausente |
| Testes unitários C | Interrompidos: GCC ausente |
| Testes de runtime | Interrompidos: módulos B ausentes |
| Sintaxe shell/Python e contrato literal | Conferidos |
| Gramática e limites P2/P4 | Preservados em relação à base Sprint 3 |

Não há confirmação de compilação sem avisos, memória, regressão ou CI. As 43
falhas citadas no plano são referência documental, não uma medição local.
Após integrar B, validar a base, B/P1, A/P2 e A/P4, depois B/P3 e B/P5; concluir
regressão, memória, programa de referência e revisão cruzada antes dos merges.
