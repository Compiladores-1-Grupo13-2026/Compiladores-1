# Testes da P4

Execute `python testes/testar_p4.py` na raiz. Os testes ficam junto aos demais
arquivos de `testes/`, sem uma pasta separada por integrante. Comentários
`[P4]` identificam a contribuição.

- `tabela_test.c`: inserção, busca, colisões de hash, duplicatas, escopos e
  reinicialização após liberar.
- `semantico_test.c`: construção/impressão dos nós P4, leitura de nomes,
  atribuições, listas, indexação, tipos e interação com a tabela.

O runner compila a AST e a semântica reais da Sprint 3, além da tabela.
Na Sprint 4, `semantico_test.c` também cobre identificadores tipados da P2.
São dois executáveis de teste, compilados
com `-std=c11 -Wall -Wextra -Werror`.

`contrato_ast_teste.h` apenas inclui o header real, por compatibilidade.
O runner não o injeta com `-include` nem redefine `NoAST`.

Os dublês antigos `dubles_ast.c/.h` não são ligados. Os testes usam os
construtores reais, liberam cada árvore e não substituem o núcleo da equipe.

Os 15 pares `.mpy`/`.esperado` da P4 estão nas pastas compartilhadas `validos/`
e `invalidos/`, preparados para a integração futura. O runner atual **não os
executa** e não certifica gramática completa, comentários, recuperação de parser
ou memória do núcleo AST.

A suíte geral da Sprint 4 usa `--ast` nos casos históricos e executa os
`p2_exec_*`/`p4_exec_*`. Os novos testes e as pendências estão no
[documentação do interpretador](../docs/interpretador.md).

`python testes/testar_p4.py --runtime` verifica os contratos P2/P4 com os módulos
reais da dupla. `--memcheck` usa Valgrind. Sem os arquivos B ou sem GCC, o
runner relata a dependência ausente. A compilação e a execução ainda não
foram validadas neste ambiente.
