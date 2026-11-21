# ===== [P4] Testes de atribuicao, listas, indexacao e tabela =====
"""Testa tabela/P2/P4 com a AST real; --runtime inclui os modulos da dupla."""
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
    parser.add_argument("--memcheck", action="store_true", help="executa os testes com Valgrind")
    parser.add_argument("--runtime", action="store_true", help="inclui testes de execucao; exige os modulos da Pessoa B")
    args = parser.parse_args()
    runtime_sources = [ROOT / "Interpretador" / name for name in (
        "interpretador.c", "valor.c", "exec_expressoes.c", "exec_condicoes.c",
        "exec_lacos.c", "ambiente.c", "lista.c", "exec_atribuicao.c", "exec_funcoes.c",
    )]
    if args.runtime:
        missing = [str(p.relative_to(ROOT)) for p in runtime_sources if not p.is_file()]
        if missing:
            raise SystemExit("Integracao pendente: " + ", ".join(missing))
    if not shutil.which("gcc"):
        raise SystemExit("Dependencia ausente: gcc")
    if args.memcheck and not shutil.which("valgrind"):
        raise SystemExit("Valgrind ausente; execute --memcheck em Linux com Valgrind instalado.")
    extension = ".exe" if os.name == "nt" else ""
    flags = ["gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-g", "-I", ROOT]
    tabela = ROOT / "Semantica/Tabela/tabela.c"
    jobs = [
        ("tabela_test", [tabela]),
        ("semantico_test", [
            tabela, ROOT / "Semantica/Ast/ast.c",
            ROOT / "Semantica/semantico.c",
        ]),
    ]
    if args.runtime:
        jobs.append(("runtime_a_test", [ROOT / "Semantica/Ast/ast.c", *runtime_sources]))
    valgrind = [
        "valgrind", "--quiet", "--leak-check=full", "--show-leak-kinds=all",
        "--errors-for-leak-kinds=all", "--error-exitcode=99",
    ] if args.memcheck else []
    with tempfile.TemporaryDirectory(prefix="minipy-p4-") as temporary:
        for name, sources in jobs:
            binary = Path(temporary) / (name + extension)
            execute(flags + sources + [HERE / (name + ".c"), "-o", binary, "-lm"])
            result = execute(valgrind + [binary], timeout=10)
            print(text(result.stdout), end="")
    print(f"Pessoa A: {len(jobs)} executaveis de teste passaram com dependencias reais.")
    if args.memcheck:
        print("Valgrind: nenhum erro nos modulos exercitados pelos testes.")


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as error:
        raise SystemExit(text(error.stdout) + text(error.stderr)) from error
