# AST — seção P4

A AST representa a estrutura do programa para análise posterior. A contribuição
P4 ocupa uma seção de `Semantica/Ast/ast.c`, identificada com o comentário:

```c
/* ===== [P4] Atribuição, listas e indexação ===== */
```

Os protótipos ficam na seção P4 de `ast.h`. Os tipos `NoAST` e `TipoNo`, a API
de alocação, a impressão geral e a liberação comuns aguardam a P1, conforme o
[plano da equipe](https://docs.google.com/document/d/1oPxylxNLkBpUya7pTXNanqJy-Rh25Lyj43E3K4mtf30/edit).
O [exemplo da Semana 6](https://github.com/sergioaafreitas/COMP1/blob/main/semana%2006/src/README.md)
é a referência didática para AST e tabela de símbolos.

## Filhos dos nós P4

O contrato define `NoAST` com tipo, linha, `int valor`, `char *texto`, filhos
`a`, `b`, `c` e encadeamento `prox`. A seção P4 utiliza os filhos assim:

| Tipo | Filho `a` | Filho `b` | Filho `c` |
|---|---|---|---|
| `AST_ATRIBUICAO` | Alvo de `=` | Valor | `NULL` |
| `AST_ATRIB_SOMA` | Alvo de `+=` | Valor | `NULL` |
| `AST_ATRIB_SUB` | Alvo de `-=` | Valor | `NULL` |
| `AST_ATRIB_MULT` | Alvo de `*=` | Valor | `NULL` |
| `AST_ATRIB_DIV` | Alvo de `/=` | Valor | `NULL` |
| `AST_LISTA` | Primeiro elemento; demais por `prox` | `NULL` | `NULL` |
| `AST_INDEXACAO` | Expressão base | Índice | `NULL` |

Uma lista vazia tem `a == NULL`. Em `itens = [1, 2]`, a atribuição aponta para
o identificador em `a` e para a lista em `b`. A lista aponta para o literal `1`,
cujo `prox` aponta para `2`. Em `itens[0] += 3`, o alvo é uma indexação. Os
identificadores e literais são nós fornecidos pela base da P1.

`ast_p4_atribuicao`, `ast_p4_lista` e `ast_p4_indexacao` usam `ast_criar` para
preencher esses filhos. As funções pertencem à seção P4 do arquivo comum;
não existe um módulo de produção separado por integrante.

## Linhas, impressão e memória

A atribuição recebe a linha do alvo; a lista, a linha do `[`; a indexação,
a linha inicial da expressão base. Cada elemento preserva sua localização.

Os filhos passam a pertencer à árvore. Um nó deve ter um único dono e `prox`
deve formar uma sequência sem ciclos. A liberação comum da P1 deverá visitar
`a`, `b`, `c`, `prox` e os textos. As ações de recuperação da P4 precisam liberar
subárvores abandonadas durante a integração do parser.

`ast_p4_nome_tipo` retorna o nome dos sete tipos P4 e `NULL` para os demais.
`ast_p4_imprimir` imprime o nó P4 com dois espaços por nível e delega os filhos
`a`, `b`, `c` ao impressor comum. O responsável pela sequência percorre `prox`
uma única vez. O retorno é `1` quando o nó foi impresso e `0` nos demais casos.

A tabela copia os nomes recebidos; seu ciclo de vida é independente dos textos
armazenados na AST.

## Semântica e parser

O visitante da seção P4 de `Semantica/semantico.c` trata atribuições, listas,
indexações e leitura de identificadores. Ele recebe callbacks para as expressões
das outras fatias e para os diagnósticos gerais. `NULL` indica tipo de nó de
outra fatia; `?` indica tipo desconhecido; `P4_TIPO_ERRO` indica erro já relatado
no filho. As políticas estão em [Decisões](decisoes.md).

As ações futuras de AST estão comentadas na própria seção P4 de
`Sintatica/parser.y`. Elas aguardam a base que fará `expr` carregar nós AST.
A forma preparada aceita `expr op_atrib expr` e `expr [ expr ]`, permitindo
que alvos como `3 = 1` e bases como `5[0]` cheguem à verificação semântica.
O parser ativo mantém o comportamento da Semana 5.

## Alcance da validação

Os testes unitários verificam filhos, tipos, linhas e encaminhamento de chamadas
da seção P4. `testes/contrato_ast_teste.h` é uma interface declarativa de teste;
`testes/dubles_ast.c` fornece nós pré-montados e registra chamadas às dependências.
Esses arquivos ficam fora do build de produção.

A validação atual não comprova a impressão completa da árvore, liberação do
núcleo P1, comentários, recuperação sintática ou conflitos do parser. Os casos
`.mpy` em `testes/validos/` e `testes/invalidos/` aguardam integração e não são
executados pelo runner unitário.

O campo `int valor` do contrato ainda precisa ser conciliado pela P1 e pela
equipe com os números decimais da linguagem.
