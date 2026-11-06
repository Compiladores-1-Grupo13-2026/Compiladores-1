"""Regenera somente os 21 casos historicos A, usando o parser real em --ast."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
CASES = {
    "validos": (
        "p2_condicionais", "p2_condicionais_completos", "p2_logica",
        "p4_atribuicao", "p4_lista_seguida_atribuicao", "p4_listas_multilinha",
    ),
    "invalidos": (
        "p2_condicao_invalida", "p2_condicao_vazia", "p2_elif_invalido",
        "p2_elif_sem_bloco", "p2_elif_vazio", "p2_else_nova_linha",
        "p2_else_sem_if", "p2_if_sem_bloco", "p4_alvo_invalido",
        "p4_lista_eof", "p4_lista_sem_fecho", "p4_lista_sem_fecho_atribuicao",
        "p4_lista_sem_fecho_newline", "p4_lista_sem_virgula", "p4_lista_vazia_invalida",
    ),
}


def main():
    binary = ROOT / "parser"
    if not binary.is_file():
        binary = ROOT / "parser.exe"
    if not binary.is_file():
        raise SystemExit("Parser ausente. Integre a base e compile com make primeiro.")
    results = []
    # Confere status e diagnosticos de todos os casos antes de gravar qualquer um.
    for folder, names in CASES.items():
        for name in names:
            case = ROOT / "testes" / folder / (name + ".txt")
            result = subprocess.run(
                [str(binary), "--ast"], input=case.read_bytes(),
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=5, cwd=ROOT,
            )
            expected_status = 0 if folder == "validos" else 1
            output = result.stdout.replace(b"\r\n", b"\n")
            if result.returncode != expected_status or (
                folder == "invalidos" and b"[ERRO " not in output
            ):
                raise SystemExit(f"Revisar {case.name}: status {result.returncode}\n"
                                 + output.decode("utf-8", errors="replace"))
            results.append((case.with_suffix(".esperado"), output))
    for target, output in results:
        target.write_bytes(output)
        print(target.relative_to(ROOT))
    print("21 expectativas A atualizadas. Revise o diff antes de integrar.")


if __name__ == "__main__":
    main()
