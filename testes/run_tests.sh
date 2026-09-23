#!/bin/sh
# P3: execute de qualquer diretorio; expectativas sao revisadas e versionadas.
set -u
cd "$(dirname "$0")/.." || exit 1
temporario=$(mktemp -d) || exit 1
trap 'rm -rf "$temporario"' EXIT HUP INT TERM
falhas=0

for pasta in validos invalidos; do
    total=0
    passou=0
    for caso in "testes/$pasta/"*.txt; do
        [ -f "$caso" ] || continue
        total=$((total + 1))
        status=0
        timeout 5 ./parser < "$caso" > "$temporario/stdout" 2> "$temporario/stderr" || status=$?
        ok=1
        if [ "$pasta" = validos ]; then
            [ "$status" -eq 0 ] && [ ! -s "$temporario/stderr" ] || ok=0
        else
            # Timeout, sinal ou falha ao iniciar o processo nao contam como rejeicao.
            [ "$status" -gt 0 ] && [ "$status" -lt 124 ] || ok=0
            grep -Eq '^\[ERRO (SINTATICO|LEXICO|SEMANTICO)\] Linha [1-9][0-9]*' "$temporario/stderr" || ok=0
        fi
        esperado=${caso%.txt}.esperado
        if [ -f "$esperado" ]; then
            # Uma segunda execucao captura a ordem real dos dois descritores.
            # Concatenar stdout e stderr da primeira execucao perderia essa ordem.
            combinado=0
            timeout 5 ./parser < "$caso" > "$temporario/combinado" 2>&1 || combinado=$?
            [ "$combinado" -eq "$status" ] || ok=0
            diff -u "$esperado" "$temporario/combinado" > "$temporario/diff" || ok=0
        else
            : > "$temporario/diff"
        fi
        if [ "$ok" -eq 1 ]; then
            printf '[OK] %s\n' "$caso"
            passou=$((passou + 1))
        else
            printf '[FALHOU] %s (saida %s)\n' "$caso" "$status"
            cat "$temporario/stderr" "$temporario/diff"
            falhas=$((falhas + 1))
        fi
    done
    if [ "$total" -eq 0 ]; then
        printf '[FALHOU] nenhum caso em testes/%s\n' "$pasta"
        falhas=$((falhas + 1))
    fi
    printf '%s: %s/%s passaram.\n' "$pasta" "$passou" "$total"
done

# Semana 3: catalogo completo, referencia e erro lexico com status nao zero.
for nome in base_programa_referencia base_catalogo_tokens; do
    caso="testes/validos/$nome"
    status=0
    timeout 5 ./tokens < "$caso.txt" > "$temporario/tokens" 2> "$temporario/stderr" || status=$?
    if [ "$status" -eq 0 ] && [ ! -s "$temporario/stderr" ] &&
       diff -u "$caso.tokens.esperado" "$temporario/tokens"; then
        printf '[OK] tokens: %s\n' "$nome"
    else
        printf '[FALHOU] tokens: %s\n' "$nome"
        cat "$temporario/stderr"
        falhas=$((falhas + 1))
    fi
done
status=0
timeout 5 ./tokens < testes/invalidos/base_lexico.txt > "$temporario/tokens" 2> "$temporario/stderr" || status=$?
if [ "$status" -eq 1 ] && grep -q 'CARACTERE_INVALIDO(@)' "$temporario/tokens" &&
   grep -q '^\[ERRO LEXICO\] Linha 1:' "$temporario/stderr"; then
    printf '[OK] tokens: caractere invalido\n'
else
    printf '[FALHOU] tokens: caractere invalido\n'
    falhas=$((falhas + 1))
fi
printf 'Total de falhas: %s\n' "$falhas"
[ "$falhas" -eq 0 ]
