# Decisões técnicas

O MiniPy separa a leitura do código, a construção da árvore, a análise
semântica e a execução. Essa organização permite inspecionar um programa com
`--ast` e executar a mesma representação no modo padrão.

## Uma árvore para analisar e executar

O Flex produz tokens; o Bison constrói a [AST](ast.md). A análise semântica
consulta a tabela de símbolos sem substituir a árvore por resultados
calculados. O interpretador percorre os nós para avaliar expressões, escolher
ramos, repetir blocos e chamar funções.

A entrada passa pelas verificações léxicas, sintáticas e semânticas antes de
ser executada. Mesmo um `print` escrito antes de um erro de análise não chega
a ser executado. Durante a execução, a saída já produzida é preservada quando
ocorre um erro.

## Tipos estáticos e valores de execução

A tabela semântica guarda nomes, categorias e tipos conhecidos. O ambiente de
execução guarda valores concretos. São estruturas independentes: a tabela é
liberada ao terminar a análise, e o ambiente é criado ao iniciar a execução.

`Valor` distingue `None`, inteiros, reais, booleanos, textos, listas e funções.
A tag do valor determina qual campo da união pode ser lido. Parâmetros,
retornos de chamadas e elementos de listas podem ter tipo estático desconhecido;
nesses casos, o executor valida os valores recebidos.

| Operação | Comportamento |
| --- | --- |
| Aritmética com inteiros | Usa `long long`; verifica estouro nas operações inteiras |
| Aritmética com um real | Produz `double` |
| Booleanos em contas | `False` vale 0 e `True` vale 1 |
| `/` | Sempre produz real, mesmo em `4 / 2` |
| `//` | Arredonda para baixo: `-7 // 2` é `-4` |
| `%` | O resto não nulo tem o sinal do divisor |
| `**` com inteiros | Resultado inteiro para expoente não negativo; real para expoente negativo |
| Textos | `+` concatena; multiplicação por inteiro ou booleano repete |

O scanner lê literais numéricos como `double`. O executor converte um literal
integral para `long long` quando ele cabe nesse tipo. Assim, a aritmética
inteira pode preservar precisão que já não seria preservada ao escrever um
literal decimal muito grande. Não há inteiros de precisão arbitrária.

## Comparação e lógica

Igualdade e diferença aceitam tipos distintos. Números e booleanos são
comparados pelo valor numérico; textos pelo conteúdo; listas recursivamente
pelos elementos; funções pela identidade do nó de definição. Tipos distintos
não numéricos são diferentes.

As comparações de ordem aceitam dois números ou dois textos. Textos são
comparados com `strcmp`, pela ordem dos bytes. Listas não implementam ordenação.

`and` e `or` usam curto-circuito e devolvem um dos operandos. `not` devolve
booleano. `None`, zero, `False`, texto vazio e lista vazia são falsos; os
demais valores são verdadeiros.

## Nomes, quadros e funções

A tabela semântica usa um hash de 211 posições por escopo, com colisões por
encadeamento. O encadeamento público dos símbolos conserva a ordem de inserção,
separado das cadeias do hash. A busca semântica percorre os escopos ativos do
mais interno até o global.

Na execução, somente chamadas de funções criam quadros. A busca do ambiente vê
o quadro atual e depois o global; não procura os quadros de quem chamou.
Atribuições escrevem no quadro atual. Blocos de `if`, `while` e `for` não
criam outro quadro.

Uma função conserva seu nó da AST, sem capturar o ambiente onde foi definida.
Definições aninhadas, portanto, não oferecem fechamentos léxicos. Os argumentos
são avaliados no quadro do chamador antes de criar o quadro da chamada; os
valores são então associados aos parâmetros. O quadro é liberado ao terminar,
e uma função sem retorno explícito de valor produz `None`.

`print` e `range` são nomes de funções embutidas, não palavras reservadas do
scanner. O executor reconhece essas chamadas pelo nome. A análise semântica
protege nomes de funções e embutidas contra atribuição e redefinição inválidas.

## Atribuição, listas e propriedade

Na atribuição simples, o valor à direita é avaliado antes do alvo. Na composta,
o alvo e seu valor anterior são fixados antes da avaliação da expressão à
direita. Isso preserva o destino quando chamadas envolvidas alteram listas.

Textos têm cópias próprias. Listas são vetores dinâmicos com contagem de
referências: `b = a` compartilha a mesma lista, e uma alteração por `b` é
visível por `a`. A cópia de uma lista não duplica seus elementos.

~~~text
a = [1, 2];
b = a;
b[0] += 5;
print(a);
~~~

A saída é `[6, 2]`. Índices precisam ser inteiros e podem ser negativos.
A gramática aceita a forma `nome[indice]`; sublistas podem ser acessadas
atribuindo uma delas a outro identificador. Textos não são indexáveis.

Cada valor devolvido pela avaliação pertence a quem o recebeu.
`ambiente_definir` e `lista_adicionar` assumem o valor recebido; buscas no
ambiente emprestam um ponteiro, e uma leitura que será guardada ou devolvida
precisa fazer uma cópia. Funções emprestam o nó da AST, que permanece vivo
durante toda a execução.

## Interrupção de blocos e erros

Um sinal compartilhado distingue execução normal, `break`, `continue` e
`return`. Blocos param ao encontrar um sinal. Laços consomem `break` e
`continue`, preservando `return` para a chamada da função.

Cada laço aceita até 1.000.000 de iterações; a profundidade de chamadas é
limitada a 1000. Esses limites interrompem programas que não terminariam ou
que consumiriam a pilha indefinidamente.

Erros de execução usam `[ERRO EXECUCAO] Linha N: mensagem` no stderr.
`erro_execucao` descarrega stdout e interrompe o programa com `longjmp`; o
ambiente é liberado e o código de saída é 1. Sucesso retorna 0.

## Limites da implementação

- A inferência semântica não determina todos os tipos e não acompanha todos
  os caminhos de execução. Verificações de valores continuam necessárias.
- A análise aceita algumas combinações de listas em `+=` e `*=`, mas o
  executor não implementa concatenação ou repetição de listas e rejeita
  essas operações.
- A contagem de referências não coleta ciclos entre listas. A impressão
  reconhece ciclos; a igualdade entre ciclos distintos pode continuar
  recursivamente.
- Em erros de execução, valores temporários abandonados pelo `longjmp` podem
  permanecer alocados até o encerramento do processo.

O [interpretador](interpretador.md) detalha a execução e os formatos de
saída. A página de [testes](testes.md) registra a validação e a cobertura.
