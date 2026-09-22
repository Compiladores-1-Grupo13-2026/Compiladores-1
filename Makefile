CC = gcc
BISON = bison
FLEX = flex
CFLAGS ?= -Wall -Wextra
BISONFLAGS = -Wall -Wcounterexamples
LDLIBS = -lfl -lm

# Preserva os caminhos ja existentes neste repositorio.
PARSER_SOURCE = Sintatica/parser.y
SCANNER_SOURCE = Lexico/scanner.l
TEST_SCRIPT ?= testes/run_tests.sh

.PHONY: all tokens test clean
all: parser

parser.tab.c parser.tab.h &: $(PARSER_SOURCE)
	$(BISON) $(BISONFLAGS) -d -o parser.tab.c $(PARSER_SOURCE)

lex.yy.c: $(SCANNER_SOURCE) parser.tab.h
	$(FLEX) -o lex.yy.c $(SCANNER_SOURCE)

# libm fornece floor e pow para os operadores da P1.
parser: parser.tab.c parser.tab.h lex.yy.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ parser.tab.c lex.yy.c $(LDLIBS)

tokens: parser.tab.h lex.yy.c
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

clean:
	$(RM) parser.tab.c parser.tab.h lex.yy.c parser tokens
