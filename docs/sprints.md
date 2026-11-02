# Sprints e quadro de acompanhamento

O plano interno usa Sprint 0 (base), 1 (léxico), 2 (parser) e 3 (erros da
Semana 5). Essa **Sprint 3 interna não é a Sprint 3 do guia do professor**,
que trata de AST/semântica a partir da Semana 6. O guia está no fechamento
das Sprints 1 e 2.

| Sprint interna | Planejado por fatia | Realizado / evidência local |
|---|---|---|
| 0 | Todas: base, tokens e precedência; P1: build | build sem warnings, terminadores e contrato registrados |
| 1 | P1 literais; P2 lógica; P3 laços; P4 listas/atribuição; P5 funções | scanner completo e `make tokens`, catálogo por fatia |
| 2 | Cada fatia acrescenta suas produções; P4 documenta gramática | cinco fatias reconhecidas; referência oficial aceita |
| 3 | Cada fatia recupera seus erros; P2 integra diagnósticos; P3 testes; P4 docs; P5 coordena PC1 | suíte verde, saídas esperadas, documentação e roteiro preparados; revisão humana e evidências externas pendentes |

## Responsáveis por fatia

| Fatia | Responsável | Escopo reconhecido | Estado na Semana 5 | Responsabilidade transversal |
|---|---|---|---|---|
| P1 | Ana | expressões, literais, aritmética e erros numéricos | ✅ | build e flags |
| P2 | Leonardo | comparações, lógica, if/elif/else e recuperação | ✅ | diagnóstico global, linha/coluna e status |
| P3 | Gabriel | while, for, break, continue e recuperação | ✅ | testes automatizados |
| P4 | Karoline | atribuição, listas, indexação e comentários | ✅ | documentação |
| P5 | João Vitor | def, return, chamadas e recuperação | ✅ | integração, quadro e formulário |

## Sprint 3 do professor: AST e semântica

Divisão mantida por recurso da linguagem, conforme o
[plano da equipe](https://docs.google.com/document/d/1oPxylxNLkBpUya7pTXNanqJy-Rh25Lyj43E3K4mtf30/edit).
Esta atualização implementa exclusivamente a P4; as linhas dos demais
integrantes registram as responsabilidades e dependências, sem declarar suas
entregas concluídas.

| Fatia | Base / responsabilidade transversal | Trabalho na fatia | Estado nesta branch |
|---|---|---|---|
| P1 — Ana | Núcleo comum da AST e build | Literais, identificadores e aritmética | Depende da entrega da responsável |
| P2 — Leonardo | Visitante comum e diagnósticos | Comparações, lógica e condicionais | Depende da entrega do responsável |
| P3 — Gabriel | Runner geral e Valgrind | Laços, contexto de break/continue | Depende da entrega do responsável |
| P4 — Karoline | Tabela de símbolos e documentação | Atribuições, listas, indexação e leitura de nomes | Módulos e testes isolados implementados; integração depende da base |
| P5 — João Vitor | Raiz/blocos do parser e fluxo do main | Funções, parâmetros, chamadas e return | Depende da entrega do responsável |

A contribuição P4 está nas seções comentadas de `Semantica/Ast/ast.c` e `ast.h`,
`Semantica/semantico.c` e `semantico.h`, além de `Semantica/Tabela/`.
A preparação das ações AST fica comentada na seção P4 do próprio
`Sintatica/parser.y`; os testes ficam em `testes/`, com casos nas pastas comuns
`validos/` e `invalidos/`. Não há pastas nem módulos de produção separados por P.
Os tipos e o núcleo da AST aguardam a P1. O contrato provisório dos testes está
em `testes/contrato_ast_teste.h`.
O parser principal continua na Semana 5. Não há tag de sprint concluída nem
integração final anunciada. O [guia da P4](p4.md) registra os passos e limites.

