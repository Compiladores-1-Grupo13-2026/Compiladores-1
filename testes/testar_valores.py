"""Verifica precisao, limites e memoria de Valor com os modulos reais."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--memcheck", action="store_true")
    args = parser.parse_args()
    if not shutil.which("gcc"):
        raise SystemExit("Dependencia ausente: gcc")
    if args.memcheck and not shutil.which("valgrind"):
        raise SystemExit("Valgrind ausente; execute --memcheck em Linux.")
    sources = [ROOT / "Semantica/Ast/ast.c", *sorted((ROOT / "Interpretador").glob("*.c"))]
    with tempfile.TemporaryDirectory(prefix="minipy-valores-") as temporary:
        binary = Path(temporary) / ("valores_test.exe" if os.name == "nt" else "valores_test")
        command = ["gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-g", "-I", str(ROOT)]
        subprocess.run(command + [str(s) for s in sources] + [
            str(ROOT / "testes/runtime_b_valores_test.c"), "-o", str(binary), "-lm",
        ], cwd=ROOT, check=True)
        prefix = ["valgrind", "--quiet", "--leak-check=full", "--show-leak-kinds=all",
                  "--errors-for-leak-kinds=all", "--error-exitcode=99"] if args.memcheck else []
        result = subprocess.run(prefix + [str(binary)], cwd=ROOT, check=True,
                                capture_output=True, timeout=10)
        print(result.stdout.decode("utf-8").replace("\r\n", "\n"), end="")
        # Os tres erros sao intencionais; confirmar que foram controlados.
        esperado = "[ERRO EXECUCAO] Linha 7: resultado inteiro fora do intervalo\n" * 3
        if result.stderr.decode("utf-8").replace("\r\n", "\n") != esperado:
            raise SystemExit("Diagnosticos inesperados:\n" + result.stderr.decode("utf-8"))


if __name__ == "__main__":
    main()
