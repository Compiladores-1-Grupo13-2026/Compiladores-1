# Decisões técnicas — seção P4

A contribuição P4 fica em seções dos arquivos compartilhados `ast.c`, `ast.h`,
`semantico.c` e `semantico.h`, identificadas por comentário com `P4` e título.
A tabela comum fica em `Semantica/Tabela/tabela.c` e `tabela.h`.

O [plano da equipe](https://docs.google.com/document/d/1oPxylxNLkBpUya7pTXNanqJy-Rh25Lyj43E3K4mtf30/edit)
define a responsabilidade de cada integrante e o contrato das estruturas.
O [guia do professor](https://github.com/sergioaafreitas/COMP1/blob/main/semana%2001/docs/Guia%20-%20Projeto%20de%20um%20interpretador.md)
situa AST e análise semântica inicial na Sprint 3. As escolhas específicas de
MiniPy abaixo adaptam esses materiais à linguagem da equipe.

## Semântica depois da construção da AST

As ações do parser registram a estrutura. A seção semântica da P4 consulta a
tabela e verifica somente seus nós. Expressões aritméticas, lógica, controle
de fluxo e funções dependem dos responsáveis por essas fatias.

O visitante P4 recebe callbacks para visitar expressões e emitir diagnósticos.
Os tipos são `int`, `str`, `bool`, `lista`, `none`, `funcao` e `?`. Este último
indica tipo desconhecido e permite continuar a análise. O marcador interno
`erro` (`P4_TIPO_ERRO`) indica um erro já diagnosticado e deve ser propagado
pelas outras fatias para impedir atualizações inválidas na tabela.

## Atribuição e leitura de nomes

A primeira atribuição simples válida cria uma variável no escopo atual.
A expressão à direita é visitada antes da inserção, para que `y = y + 1`
acuse a leitura de `y` ainda inexistente. Um valor de tipo `?` permite a
atribuição; `erro` impede a inserção e a atualização.

Reatribuição é permitida e atualiza o tipo da variável local, preservando a
linha da primeira atribuição válida. Atribuições compostas leem o valor anterior:
um identificador precisa existir no escopo atual. Uma variável apenas global
não satisfaz essa exigência dentro de função sem uma construção `global`.

| Operador | Tipos conhecidos aceitos | Resultado |
|---|---|---|
| `+=` | `int` com `int`; `str` com `str`; `lista` com `lista` | Tipo do alvo |
| `-=` | `int` com `int` | `int` |
| `*=` | `int` com `int`; alvo `str` ou `lista` com valor `int` | Tipo do alvo |
| `/=` | `int` com `int` | `?`, até acordar a representação de decimais |

Se algum operando tem tipo desconhecido, o resultado também é `?`. Outros pares
conhecidos geram erro. A P4 detecta divisor literal `0` em `/=`; divisores
obtidos de valores em execução ficam para a etapa de interpretação.

Funções e builtins são categorias protegidas contra atribuição. `print` e
`range` entram no escopo global como builtins de tipo `funcao`. A tabela não
implementa chamadas; sua aridade começa desconhecida para a integração de funções.

## Tabela conforme o professor

A [Semana 6](https://github.com/sergioaafreitas/COMP1/blob/main/semana%2006/src/tabela.c)
apresenta lista encadeada. A [Semana 7](https://github.com/sergioaafreitas/COMP1/blob/main/semana%2007/src/tabela.c)
evolui para hash com 211 posições e colisões por encadeamento. A tabela da
equipe segue essa segunda estrutura e a função que acumula
`h = (h << 4) + caractere` antes do módulo 211. O caractere é lido como
`unsigned char` para evitar depender do sinal de `char`.

Cada escopo tem um índice privado. Entradas privadas ligam os símbolos às
cadeias de colisão, preservando a estrutura pública `Simbolo` do contrato.
Seu `prox` mantém a ordem de inserção para a impressão. Na saída de escopo,
são liberados índice, símbolos e nomes.

Os exemplos C do professor usam apenas o escopo global. Pilha de escopos,
categorias, builtins, linha e liberação são adaptações previstas no plano MiniPy.
A prática da Semana 7 usa outra linguagem; suas declarações `int nome;` e
regras de redeclaração não são transplantadas para MiniPy.

## Escopos, listas e índices

Somente funções criam novos escopos. A busca local verifica o escopo atual;
a busca geral percorre os ativos do mais interno até o global. Uma primeira
atribuição simples em função cria uma variável local mesmo se houver global
de mesmo nome. Ao sair, os locais são liberados. A tabela imprime os escopos
ativos, sem guardar histórico dos encerrados.

Listas vazias, aninhadas e heterogêneas são permitidas. O tipo externo é `lista`;
os elementos são visitados individualmente para verificar nomes. Quando os
tipos são conhecidos, a base de indexação deve ser lista e o índice, inteiro.
O resultado tem tipo `?`, pois o contrato não descreve o tipo de cada elemento.
Atribuir a uma posição conserva o tipo externo `lista`.

Limites, tamanhos e índices negativos dependem dos valores da execução. A P4
não acrescenta indexação de strings. Comentários continuam descartados pelo
scanner existente e não geram símbolos nem nós.

## Organização e pendências

Os construtores e o impressor P4 ocupam sua seção de `Semantica/Ast/ast.c`;
os protótipos ficam em `ast.h`. Os tipos e a API do núcleo comum aguardam a P1.
O visitante ocupa a seção P4 de `Semantica/semantico.c`. As ações futuras de
AST ficam comentadas na seção P4 de `Sintatica/parser.y`, aguardando a mudança
de `expr` para nós pelo grupo. O parser ativo mantém a entrega da Semana 5.

| Responsável | Integração que permanece com ele |
|---|---|
| P1 | Tipos, alocação, impressão e liberação gerais da AST; domínio numérico |
| P2 | Despacho semântico e diagnósticos gerais |
| P3 | Suíte geral e verificação global de memória |
| P4 | Seções de atribuições/listas/indexação/leitura, tabela, testes próprios e documentação |
| P5 | Parser, raiz, blocos, main, funções e chamadas |

O runner `testes/testar_p4.py` executa somente dois testes unitários, com o
contrato declarativo `testes/contrato_ast_teste.h` e os dublês `dubles_ast.c/h`.
O alvo geral `make test` e a suíte da P3 permanecem com seu responsável.

Os casos `.mpy` e `.esperado` em `testes/validos/` e `testes/invalidos/` estão
preparados para a integração e não são executados pelo runner atual. Os unitários
não certificam parser, comentários, recuperação, conflitos Bison ou memória do
núcleo da P1. Valgrind para os unitários permanece pendente neste Windows.

O contrato `int valor` ainda precisa ser conciliado pela P1 e pela equipe com
os decimais da entrega anterior. A seção P4 do header declara suas funções e
preserva essa decisão para os responsáveis pela base.
