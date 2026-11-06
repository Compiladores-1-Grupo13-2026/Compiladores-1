"""Confere historicos e casos novos B sem escrever arquivos de referencia."""
from pathlib import Path
import subprocess
import difflib

ROOT = Path(__file__).resolve().parents[1]
HISTORICOS = {
    "validos": (
        "p1_original", "p1_parenteses_unario", "p3_break_continue_em_if",
        "p3_lacos", "p3_lacos_completos", "base_catalogo_tokens",
        "base_comentarios", "base_programa_referencia", "base_valores_sem_execucao",
    ),
    "invalidos": (
        "p3_bloco_aberto", "p3_break_em_def_dentro_de_laco", "p3_break_fora",
        "p3_continue_fora", "p3_for_iteravel_numero", "p3_for_sem_in",
        "p3_for_sem_variavel", "p3_while_condicao_invalida", "p3_while_sem_condicao",
        "base_colunas", "base_crlf", "base_lexico", "base_multiplos_erros",
    ),
}


def main():
    binary = ROOT / ("parser.exe" if (ROOT / "parser.exe").is_file() else "parser")
    tokens = ROOT / ("tokens.exe" if (ROOT / "tokens.exe").is_file() else "tokens")
    results = []
    for folder, names in HISTORICOS.items():
        cases = [ROOT / "testes" / folder / (name + ".txt") for name in names]
        # Os dez historicos P5 sao conferidos, preservando seu conteudo.
        cases.extend(sorted(case for case in (ROOT / "testes" / folder).glob("p5_*.txt")
                            if "_exec_" not in case.stem and case.with_suffix(".esperado").exists()))
        for prefix in ("p1", "p3", "p5"):
            cases.extend(sorted((ROOT / "testes" / folder).glob(prefix + "_exec_*.txt")))
        for case in cases:
            mode = [] if "_exec_" in case.stem else ["--ast"]
            result = subprocess.run([str(binary), *mode], cwd=ROOT,
                                    input=case.read_bytes(), stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT, timeout=5)
            output = result.stdout.replace(b"\r\n", b"\n")
            expected_status = 0 if folder == "validos" else 1
            if result.returncode != expected_status or (folder == "invalidos" and b"[ERRO " not in output):
                raise SystemExit(f"Revisar {case.name}: status {result.returncode}\n" + output.decode("utf-8"))
            results.append((case.with_suffix(".esperado"), output))
    case = ROOT / "testes/validos/base_catalogo_tokens.txt"
    result = subprocess.run([str(tokens)], cwd=ROOT, input=case.read_bytes(),
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=5)
    if result.returncode or result.stderr:
        raise SystemExit("Falha ao validar catalogo de tokens")
    results.append((case.with_suffix(".tokens.esperado"), result.stdout.replace(b"\r\n", b"\n")))
    failures = 0
    for target, output in results:
        before = target.read_bytes().replace(b"\r\n", b"\n") if target.exists() else b""
        if before != output:
            failures += 1
            print("".join(difflib.unified_diff(before.decode("utf-8").splitlines(True),
                  output.decode("utf-8").splitlines(True), fromfile=str(target.relative_to(ROOT)), tofile="atual")), end="")
    print(f"{len(results)} expectativas B conferidas; {failures} divergencias.")
    # Verifica o comportamento observado: erros de analise impedem ate prints
    # anteriores, e tipos conhecidos sao rejeitados antes de entrar no runtime.
    invalidos = (
        (b'print("executou")\nx = "a"\nx - 1\n', b"[ERRO SEMANTICO]", b"tipos incompativeis"),
        (b'print("executou")\nx = 1\nfor i in x { }\n', b"[ERRO SEMANTICO]", b"'int'"),
        (b'print("executou")\nx = True\nfor i in x { }\n', b"[ERRO SEMANTICO]", b"'bool'"),
        (b'print("executou")\nx = 1.5\nfor i in x { }\n', b"[ERRO SEMANTICO]", b"'float'"),
        (b'print("executou")\nfor x in x { }\n', b"[ERRO SEMANTICO]", b"'x'"),
        (b'print("executou")\n@\n', b"[ERRO LEXICO]", b"Linha 2"),
        (b'print("executou")\nif { }\n', b"[ERRO SINTATICO]", b"Linha 2"),
    )
    for source, prefix, message in invalidos:
        result = subprocess.run([str(binary)], input=source, cwd=ROOT,
                                capture_output=True, timeout=5)
        if result.returncode != 1 or result.stdout or prefix not in result.stderr or message not in result.stderr:
            raise SystemExit(f"Falha de integracao: {source!r}\n{result.stdout!r}\n{result.stderr!r}")
    result = subprocess.run([str(binary)], cwd=ROOT, capture_output=True, timeout=5,
                            input=b'def soma(x) { return x + 1; }\nf = soma\nprint(f(4))\n')
    if result.returncode or result.stderr or result.stdout.replace(b"\r\n", b"\n") != b"5\n":
        raise SystemExit("Falha na chamada por alias de funcao")
    referencia = b'''def fatorial(n) {
    if n <= 1 { return 1; } else { return n * fatorial(n - 1); }
}
x = 5;
while x > 0 { print(x); x -= 1; }
for i in range(3) { print(i); }
lista = [1, 2, 3];
print(fatorial(5), lista[0], "fim")
'''
    result = subprocess.run([str(binary)], input=referencia, cwd=ROOT,
                            capture_output=True, timeout=5)
    if result.returncode or result.stderr or result.stdout.replace(b"\r\n", b"\n") != b"5\n4\n3\n2\n1\n0\n1\n2\n120 1 fim\n":
        raise SystemExit("Falha no programa de referencia da secao 2.4")
    print("9 verificacoes de tipos, bloqueio de execucao, alias e programa de referencia passaram.")
    raise SystemExit(bool(failures))


if __name__ == "__main__":
    main()
