# Analisador semântico

A contribuição de Karoline fica na seção
`/* ===== [P4] Atribuição, listas e indexação ===== */` do arquivo compartilhado
`Semantica/semantico.c`, com declarações em `semantico.h`. Ela verifica
atribuições, listas, indexações e identificadores de leitura. As outras fatias
acrescentarão suas contribuições nos mesmos arquivos, em suas seções.

A tabela comum está em `Semantica/Tabela/tabela.c` e `tabela.h`. O despacho
semântico geral e os diagnósticos são dependências da P2; o visitante P4 recebe
os callbacks correspondentes.

O parser principal ainda aguarda a base que construirá a AST completa. Os
testes unitários exercitam a contribuição P4 dos arquivos compartilhados com
nós pré-montados e simulações das dependências. Consulte a [AST](../ast.md) e
as [decisões](../decisoes.md).
