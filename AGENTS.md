# Memória do projeto — instrução da Karoline

- Trabalhar **somente na P4**: atribuição simples/composta, listas, indexação,
  comentários, tabela de símbolos e documentação atribuída à P4.
- Não desenvolver funcionalidades nem assumir a base de P1, P2, P3 ou P5.
- Usar os arquivos compartilhados `Semantica/Ast/ast.c`, `ast.h`,
  `Semantica/semantico.c` e `semantico.h`. Identificar a contribuição com
  `/* ===== [P4] Atribuição, listas e indexação ===== */`.
- Não criar pastas ou módulos de produção separados chamados `p4`,
  `p4_ast.*` ou `p4_semantico.*`. Somente a seção P4 dos arquivos comuns
  deve ser implementada; não assumir o núcleo P1.
- As declarações específicas da P4 ficam no header compartilhado. O contrato
  provisório dos testes fica em `testes/contrato_ast_teste.h`; não define
  a base de produção que cabe à P1.
- Não alterar o alvo geral `make test` nem reproduzir a suíte da P3. Manter
  apenas testes próprios da P4, com dublês mínimos das dependências.
- A tabela usa hash de 211 posições com encadeamento, conforme a Semana 7
  do professor; escopos e campos públicos seguem o contrato da equipe.
- Branch da Karoline: `Atribuicoes-Comentarios--Bison` (entrega sintática).
- Seguir o contrato e a divisão do
  [Plano de Trabalho — Sprint 3](https://docs.google.com/document/d/1oPxylxNLkBpUya7pTXNanqJy-Rh25Lyj43E3K4mtf30/edit).
- Explicar as alterações e distinguir implementação P4 testada de integração
  pendente dos colegas. Não anunciar a sprint completa sem essas dependências.
- A base compartilhada estava vazia em `c4502e6`: `ast.c`, `semantico.c` e
  `src/main.c`. Não substituir essas tarefas por implementações próprias.
- As seções P4 têm testes unitários em `testes/`, executados por
  `testes/testar_p4.py`. As entradas `.mpy` ficam nas pastas comuns
  `testes/validos/` e `testes/invalidos/`.
- A preparação da gramática AST está comentada na própria seção P4 de
  `Sintatica/parser.y`, aguardando a base para integração.
  Os dublês dos testes não implementam o parser nem o núcleo AST da equipe.
