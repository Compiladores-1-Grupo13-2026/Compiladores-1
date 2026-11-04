CC = gcc
BISON = bison
FLEX = flex
CFLAGS ?= -Wall -Wextra
BISONFLAGS = -Wall -Werror -v
LDLIBS = -lfl -lm

# Remove alvos incompletos quando uma receita falha.
.DELETE_ON_ERROR:

# Preserva os caminhos ja existentes neste repositorio.
PARSER_SOURCE = Sintatica/parser.y
SCANNER_SOURCE = Lexico/scanner.l
TEST_SCRIPT ?= testes/run_tests.sh

.PHONY: all test clean
all: parser

parser.tab.c parser.tab.h &: $(PARSER_SOURCE) Makefile
	$(BISON) $(BISONFLAGS) -d -o parser.tab.c $(PARSER_SOURCE)

lex.yy.c: $(SCANNER_SOURCE) parser.tab.h
	$(FLEX) -o lex.yy.c $(SCANNER_SOURCE)

# libm fornece floor e pow para os operadores da P1.
parser: parser.tab.c parser.tab.h lex.yy.c Semantica/Ast/ast.c Semantica/Ast/ast.h Semantica/semantico.c Semantica/semantico.h Makefile
	$(CC) $(CPPFLAGS) -I. $(CFLAGS) $(LDFLAGS) -o $@ parser.tab.c lex.yy.c Semantica/Ast/ast.c Semantica/semantico.c $(LDLIBS)

tokens: parser.tab.h lex.yy.c Makefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -DSO_TOKENS $(LDFLAGS) -o $@ lex.yy.c $(LDLIBS)

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

# ===== [P1] Verificacao de memoria dos casos validos de expressoes =====
# Valgrind e preferido; usa AddressSanitizer com deteccao de leaks se ausente.
VALIDOS_P1 = $(wildcard testes/validos/p1_*.txt)
.PHONY: memcheck parser-memcheck
memcheck: parser
	@if command -v valgrind >/dev/null 2>&1; then \
		for caso in $(VALIDOS_P1); do \
			valgrind --quiet --leak-check=full --errors-for-leak-kinds=all --error-exitcode=1 ./parser < "$$caso" >/dev/null || exit $$?; \
		done; \
	else \
		printf '%s\n' 'valgrind indisponivel; usando AddressSanitizer com deteccao de leaks.'; \
		$(MAKE) --no-print-directory parser-memcheck || exit $$?; \
		for caso in $(VALIDOS_P1); do \
			ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./parser-memcheck < "$$caso" >/dev/null || exit $$?; \
		done; \
	fi

parser-memcheck: parser.tab.c parser.tab.h lex.yy.c Semantica/Ast/ast.c Semantica/Ast/ast.h Semantica/semantico.c Semantica/semantico.h Makefile
	$(CC) $(CPPFLAGS) -I. $(CFLAGS) -g -fsanitize=address -fno-omit-frame-pointer $(LDFLAGS) -o $@ parser.tab.c lex.yy.c Semantica/Ast/ast.c Semantica/semantico.c $(LDLIBS) -fsanitize=address
# ===== FIM [P1] Verificacao de memoria dos casos validos de expressoes =====

clean:
	$(RM) parser.tab.c parser.tab.h lex.yy.c parser parser-memcheck tokens parser.output
