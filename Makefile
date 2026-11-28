# FGA0003 - Compiladores 1
# Curso de Engenharia de Software
# Universidade de Brasília (UnB)
# Equipe 13 - MiniPy
#
# Arquivo: Makefile
# Descrição: regras para gerar e compilar o parser e o scanner,
#            executar o MiniPy, rodar testes e verificar memória.

CC = gcc
BISON = bison
FLEX = flex
CFLAGS ?= -Wall -Wextra
BISONFLAGS = -Wall -Werror -v
LDLIBS = -lfl -lm

# Remove alvos incompletos quando uma receita falha.
.DELETE_ON_ERROR:

# Todo o codigo-fonte fica em src/, uma pasta por fase do pipeline.
PARSER_SOURCE = src/sintatico/parser.y
SCANNER_SOURCE = src/lexico/scanner.l
TEST_SCRIPT ?= testes/run_tests.sh

.PHONY: all test clean
all: parser

parser.tab.c parser.tab.h &: $(PARSER_SOURCE) Makefile
	$(BISON) $(BISONFLAGS) -d -o parser.tab.c $(PARSER_SOURCE)

lex.yy.c: $(SCANNER_SOURCE) parser.tab.h
	$(FLEX) -o lex.yy.c $(SCANNER_SOURCE)

# libm fornece floor e pow para os operadores da P1.
# [P5] Fontes do interpretador integrado: AST, tabela de simbolos, semantica
# e otimizador. Os includes partem de src/ (ex.: "ast/ast.h").
FONTES_SEMANTICA = src/ast/ast.c src/semantico/tabela.c src/semantico/semantico.c \
    src/otimizador/otimizador.c
HEADERS_SEMANTICA = src/ast/ast.h src/semantico/tabela.h src/semantico/semantico.h \
    src/otimizador/otimizador.h
FONTES_INTERPRETADOR = src/interpretador/interpretador.c src/interpretador/valor.c \
    src/interpretador/exec_expressoes.c src/interpretador/exec_condicoes.c \
    src/interpretador/exec_lacos.c src/interpretador/ambiente.c src/interpretador/lista.c \
    src/interpretador/exec_atribuicao.c src/interpretador/exec_funcoes.c
HEADERS_INTERPRETADOR = src/interpretador/valor.h src/interpretador/ambiente.h src/interpretador/interpretador.h

parser: parser.tab.c parser.tab.h lex.yy.c $(FONTES_SEMANTICA) $(HEADERS_SEMANTICA) $(FONTES_INTERPRETADOR) $(HEADERS_INTERPRETADOR) Makefile
	$(CC) $(CPPFLAGS) -I. -Isrc $(CFLAGS) $(LDFLAGS) -o $@ parser.tab.c lex.yy.c $(FONTES_SEMANTICA) $(FONTES_INTERPRETADOR) $(LDLIBS)

tokens: parser.tab.h lex.yy.c Makefile
	$(CC) $(CPPFLAGS) -Isrc $(CFLAGS) -DSO_TOKENS $(LDFLAGS) -o $@ lex.yy.c $(LDLIBS)

# P1 fornece o alvo de build; o script de testes pertence a P3.
test:
	@test -f "$(TEST_SCRIPT)" || { printf '%s\n' 'Pendente: script de testes da P3 em $(TEST_SCRIPT).' >&2; exit 1; }
	$(MAKE) parser tokens
	sh "$(TEST_SCRIPT)"

# ===== [P4] Atribuicao, listas e comentarios =====
# make p4 ARQ=arquivo.py: gera o parser (valida a gramatica da P4) e mostra os
# tokens do tema (ASSIGN, PLUSEQ, MINUSEQ, TIMESEQ, DIVEQ, LBRACKET, RBRACKET).
.PHONY: p4

p4: parser.tab.c tokens
	@test -n "$(ARQ)" || { echo 'Uso: make p4 ARQ=arquivo.py' >&2; exit 1; }
	./tokens < "$(ARQ)"

# [P4] Sprint 3: verifica a fatia contra o contrato sem implementar a base
# pendente de P1/P2/P5. O script P3 continua responsavel pela suite anterior.
PYTHON ?= python3
.PHONY: test-p4 memcheck-p4
test-p4:
	$(PYTHON) testes/testar_p4.py

memcheck-p4:
	$(PYTHON) testes/testar_p4.py --memcheck
# ===== [P1] Execucao e verificacao de memoria =====
.PHONY: run
run: parser
	@test -n "$(ARQ)" || { printf '%s\n' 'Uso: make run ARQ=arquivo.txt' >&2; exit 1; }
	./parser < "$(ARQ)"

# Valgrind e preferido; usa AddressSanitizer com deteccao de leaks se ausente.
VALIDOS_MEMCHECK = $(sort $(wildcard testes/validos/p1_*.txt testes/validos/*_exec_*.txt testes/validos/*_ast_otimizada_*.txt))
.PHONY: memcheck parser-memcheck
memcheck: parser
	@if command -v valgrind >/dev/null 2>&1; then \
		for caso in $(VALIDOS_MEMCHECK); do \
			modo=--ast; case "$$caso" in *_exec_*) modo= ;; *_ast_otimizada_*) modo=--ast-otimizada ;; esac; \
			valgrind --quiet --leak-check=full --errors-for-leak-kinds=definite,indirect,possible --error-exitcode=1 ./parser $$modo < "$$caso" >/dev/null || exit $$?; \
		done; \
	else \
		printf '%s\n' 'valgrind indisponivel; usando AddressSanitizer com deteccao de leaks.'; \
		$(MAKE) --no-print-directory parser-memcheck || exit $$?; \
		for caso in $(VALIDOS_MEMCHECK); do \
			modo=--ast; case "$$caso" in *_exec_*) modo= ;; *_ast_otimizada_*) modo=--ast-otimizada ;; esac; \
			ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./parser-memcheck $$modo < "$$caso" >/dev/null || exit $$?; \
		done; \
	fi

parser-memcheck: parser.tab.c parser.tab.h lex.yy.c $(FONTES_SEMANTICA) $(HEADERS_SEMANTICA) $(FONTES_INTERPRETADOR) $(HEADERS_INTERPRETADOR) Makefile
	$(CC) $(CPPFLAGS) -I. -Isrc $(CFLAGS) -g -fsanitize=address -fno-omit-frame-pointer $(LDFLAGS) -o $@ parser.tab.c lex.yy.c $(FONTES_SEMANTICA) $(FONTES_INTERPRETADOR) $(LDLIBS) -fsanitize=address
# ===== FIM [P1] Verificacao de memoria dos casos validos de expressoes =====

clean:
	$(RM) parser.tab.c parser.tab.h lex.yy.c parser parser-memcheck tokens parser.output
