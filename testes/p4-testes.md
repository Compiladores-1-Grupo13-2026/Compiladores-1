# Testes da P4

Execute `python testes/testar_p4.py` na raiz. Os testes ficam junto aos demais
arquivos de `testes/`, sem uma pasta separada por integrante. Comentários
`[P4]` identificam a contribuição.

- `tabela_test.c`: inserção, busca, colisões de hash, duplicatas, escopos e
  reinicialização após liberar.
- `semantico_test.c`: construção/impressão dos nós P4, leitura de nomes,
  atribuições, listas, indexação, tipos e interação com a tabela.

O runner compila as seções P4 dos arquivos compartilhados
`Semantica/Ast/ast.c` e `Semantica/semantico.c`, além de
`Semantica/Tabela/tabela.c`. A implementação comum da AST e o despacho
semântico geral ainda dependem da base do grupo.

`contrato_ast_teste.h` contém somente a interface esperada dessas dependências.
Os nomes de todas as fatias no enum preservam os valores do contrato; não há
implementação dessas fatias nesse header.

`dubles_ast.c` fornece nós pré-montados em memória estática e registra chamadas
ao impressor. Não implementa núcleo de AST, parser, aritmética ou visitante
geral. Esses dublês são exclusivos do teste e não entram no build de produção.

Os 15 pares `.mpy`/`.esperado` da P4 estão nas pastas compartilhadas `validos/`
e `invalidos/`, preparados para a integração futura. O runner atual **não os
executa** e não certifica gramática completa, comentários, recuperação de parser
ou memória do núcleo AST.

A suíte geral e sua integração continuam com a P3. O runner da P4 não reproduz
essa suíte nem altera o alvo geral `make test`. `--memcheck` é opcional e verifica
somente os módulos exercitados pelos testes unitários P4, quando houver Valgrind.
