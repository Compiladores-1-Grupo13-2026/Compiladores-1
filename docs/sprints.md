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

