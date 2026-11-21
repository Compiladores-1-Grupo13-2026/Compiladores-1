#include "Interpretador/interpretador.h"
#include "Interpretador/ambiente.h"

#include <stddef.h>

/* O laco consome apenas break/continue. O retorno pertence a chamada. */
static int encerrar_volta(void) {
    if (sinal_atual == SINAL_BREAK) {
        sinal_atual = SINAL_NORMAL;
        return 1;
    }
    if (sinal_atual == SINAL_CONTINUE) sinal_atual = SINAL_NORMAL;
    return sinal_atual == SINAL_RETURN;
}

static void contar_volta(long *voltas, int linha) {
    if (++*voltas > 1000000)
        erro_execucao(linha, "laco excedeu o limite de 1000000 iteracoes");
}

Valor executar_laco(NoAST *no) {
    if (no->tipo == AST_BREAK) {
        sinal_atual = SINAL_BREAK;
        return valor_none();
    }
    if (no->tipo == AST_CONTINUE) {
        sinal_atual = SINAL_CONTINUE;
        return valor_none();
    }

    long voltas = 0;
    if (no->tipo == AST_WHILE) {
        while (1) {
            Valor condicao = interpretar_no(no->a);
            int continuar = valor_verdadeiro(condicao);
            valor_liberar(condicao);
            if (!continuar) break;
            contar_volta(&voltas, no->linha);
            valor_liberar(interpretar_no(no->b));
            if (encerrar_volta()) break;
        }
        return valor_none();
    }
    if (no->tipo == AST_FOR) {
        /* Esta referencia permanece viva mesmo se o corpo redefinir o nome
         * original. A expressao iteravel e avaliada somente uma vez. */
        Valor iteravel = interpretar_no(no->a);
        if (iteravel.tipo == VAL_LISTA) {
            for (int i = 0; i < iteravel.u.lista->tamanho; i++) {
                contar_volta(&voltas, no->linha);
                ambiente_definir(no->texto, valor_copiar(iteravel.u.lista->itens[i]));
                valor_liberar(interpretar_no(no->b));
                if (encerrar_volta()) break;
            }
        } else if (iteravel.tipo == VAL_STR) {
            for (size_t i = 0; iteravel.u.texto[i]; i++) {
                contar_volta(&voltas, no->linha);
                char letra[2] = {iteravel.u.texto[i], '\0'};
                ambiente_definir(no->texto, valor_str(letra));
                valor_liberar(interpretar_no(no->b));
                if (encerrar_volta()) break;
            }
        } else {
            erro_execucao(no->linha, "'%s' nao e iteravel", valor_nome_tipo(iteravel));
        }
        valor_liberar(iteravel);
        return valor_none();
    }
    erro_execucao(no->linha, "no de laco desconhecido");
    return valor_none();
}
