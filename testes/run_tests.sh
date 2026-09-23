#!/bin/bash

PASSOU=0
FALHOU=0

echo "========================================"
echo " Rodando os testes "
echo "========================================"

for arquivo in testes/*.txt; do
    [ -f "$arquivo" ] || continue

    echo -n "Testando $arquivo ... "
    ./parser < "$arquivo" > /dev/null 2>&1
    STATUS=$?

    if [ $STATUS -eq 0 ]; then
        echo "[OK]"
        PASSOU=$((PASSOU + 1))
    else
        echo "[FALHOU]"
        FALHOU=$((FALHOU + 1))
    fi
done

echo "========================================"
echo "Resultado: $PASSOU passaram, $FALHOU falharam."
echo "========================================"

if [ $FALHOU -ne 0 ]; then
    exit 1
fi