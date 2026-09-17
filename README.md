# comp1-2026-02-grupo13

Interpretador para um subconjunto da linguagem C, implementado em Python — Equipe 13.

## Estrutura do projeto

- `analisador_lexico/` — análise léxica (tokenização)
- `analisador_sintatico/` — análise sintática (AST)
- `analisador_semantico/` — análise semântica
- `src/` — ponto de entrada do interpretador
- `testes/` — testes automatizados
- `docs/` — documentação publicada via GitHub Pages

## Documentação

A documentação é gerada com [MkDocs Material](https://squidfunk.github.io/mkdocs-material/)
e publicada automaticamente no GitHub Pages a cada push na branch `main`.

Para rodar localmente:

```bash
pip install -r requirements-docs.txt
mkdocs serve
```