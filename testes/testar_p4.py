# ===== [P4] Testes de atribuicao, listas, indexacao e tabela =====
"""Testa somente os modulos P4, sem implementar a base nem a suite dos colegas.

O contrato declarativo e os dubles sao exclusivos deste teste. Os arquivos
.mpy/.esperado estao preparados para integracao quando o grupo entregar a base.
"""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HERE = Path(__file__).resolve().parent


def execute(command, **kwargs):
    return subprocess.run(
        [str(item) for item in command], cwd=ROOT, check=True,
        capture_output=True, **kwargs,
    )


def text(data: bytes) -> str:
    return data.decode("utf-8").replace("\r\n", "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--memcheck", action="store_true", help="verifica somente os testes unitarios P4 com Valgrind")
    args = parser.parse_args()
    if not shutil.which("gcc"):
        raise SystemExit("Dependencia ausente: gcc")
    if args.memcheck and not shutil.which("valgrind"):
        raise SystemExit("Valgrind ausente; execute --memcheck em Linux com Valgrind instalado.")
    extension = ".exe" if os.name == "nt" else ""
    flags = ["gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-I", ROOT]
    tabela = ROOT / "Semantica/Tabela/tabela.c"
    jobs = (
        ("tabela_test", [tabela]),
        ("semantico_test", [
            tabela, ROOT / "Semantica/Ast/ast.c",
            ROOT / "Semantica/semantico.c", HERE / "dubles_ast.c",
        ]),
    )
    valgrind = [
        "valgrind", "--quiet", "--leak-check=full", "--show-leak-kinds=all",
        "--errors-for-leak-kinds=all", "--error-exitcode=99",
    ] if args.memcheck else []
    with tempfile.TemporaryDirectory(prefix="minipy-p4-") as temporary:
        for name, sources in jobs:
            binary = Path(temporary) / (name + extension)
            contract = ["-include", HERE / "contrato_ast_teste.h"] if name == "semantico_test" else []
            execute(flags + contract + sources + [HERE / (name + ".c"), "-o", binary])
            result = execute(valgrind + [binary], timeout=10)
            print(text(result.stdout), end="")
    print("P4: 2 testes unitarios passaram. Casos .mpy aguardam a base do grupo.")
    if args.memcheck:
        print("Valgrind: nenhum erro nos modulos exercitados pelos testes unitarios P4.")


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as error:
        raise SystemExit(text(error.stdout) + text(error.stderr)) from error
