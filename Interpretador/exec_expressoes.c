#include "Interpretador/interpretador.h"

#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int numerico(Valor v) {
    return v.tipo == VAL_INT || v.tipo == VAL_REAL || v.tipo == VAL_BOOL;
}

static long long inteiro(Valor v) {
    return v.tipo == VAL_BOOL ? v.u.booleano : v.u.inteiro;
}

static double real(Valor v) {
    return v.tipo == VAL_REAL ? v.u.real : (double)inteiro(v);
}

static const char *simbolo(TipoNo op) {
    switch (op) {
        case AST_SOMA: return "+";
        case AST_SUBTRACAO: return "-";
        case AST_MULTIPLICACAO: return "*";
        case AST_DIVISAO: return "/";
        case AST_DIVINT: return "//";
        case AST_MODULO: return "%";
        case AST_POTENCIA: return "**";
        default: return "?";
    }
}

static char *alocar_texto(size_t tamanho, int linha) {
    char *texto = malloc(tamanho);
    if (!texto) erro_execucao(linha, "memoria insuficiente");
    return texto;
}

/* Assume a posse do buffer recem-alocado, sem uma segunda copia. */
static Valor texto_proprio(char *texto) {
    Valor v = { .tipo = VAL_STR };
    v.u.texto = texto;
    return v;
}

static Valor concatenar(const char *a, const char *b, int linha) {
    size_t na = strlen(a), nb = strlen(b);
    if (na > SIZE_MAX - nb - 1) erro_execucao(linha, "texto excedeu o tamanho suportado");
    char *texto = alocar_texto(na + nb + 1, linha);
    memcpy(texto, a, na);
    memcpy(texto + na, b, nb + 1);
    return texto_proprio(texto);
}

static Valor repetir(const char *origem, long long vezes, int linha) {
    size_t tamanho = strlen(origem);
    if (vezes <= 0 || tamanho == 0) return valor_str("");
    if ((unsigned long long)vezes > (SIZE_MAX - 1) / tamanho)
        erro_execucao(linha, "texto excedeu o tamanho suportado");
    size_t total = tamanho * (size_t)vezes;
    char *texto = alocar_texto(total + 1, linha);
    for (size_t posicao = 0; posicao < total; posicao += tamanho)
        memcpy(texto + posicao, origem, tamanho);
    texto[total] = '\0';
    return texto_proprio(texto);
}

static long long multiplicar_inteiros(long long a, long long b, int linha) {
    int excedeu = 0;
    if (a > 0) {
        if (b > 0) excedeu = a > LLONG_MAX / b;
        else if (b < 0) excedeu = b < LLONG_MIN / a;
    } else if (a < 0) {
        if (b > 0) excedeu = a < LLONG_MIN / b;
        else if (b < 0) excedeu = a < LLONG_MAX / b;
    }
    if (excedeu) erro_execucao(linha, "resultado inteiro fora do intervalo");
    return a * b;
}

static Valor operar_inteiros(TipoNo op, long long a, long long b, int linha) {
    switch (op) {
        case AST_SOMA:
            if ((b > 0 && a > LLONG_MAX - b) || (b < 0 && a < LLONG_MIN - b))
                erro_execucao(linha, "resultado inteiro fora do intervalo");
            return valor_int(a + b);
        case AST_SUBTRACAO:
            if ((b > 0 && a < LLONG_MIN + b) || (b < 0 && a > LLONG_MAX + b))
                erro_execucao(linha, "resultado inteiro fora do intervalo");
            return valor_int(a - b);
        case AST_MULTIPLICACAO:
            return valor_int(multiplicar_inteiros(a, b, linha));
        case AST_DIVISAO:
            return valor_real((double)a / (double)b);
        case AST_DIVINT: {
            if (a == LLONG_MIN && b == -1)
                erro_execucao(linha, "resultado inteiro fora do intervalo");
            long long quociente = a / b, resto = a % b;
            if (resto && ((resto < 0) != (b < 0))) quociente--;
            return valor_int(quociente);
        }
        case AST_MODULO: {
            if (a == LLONG_MIN && b == -1) return valor_int(0);
            long long resto = a % b;
            if (resto && ((resto < 0) != (b < 0))) resto += b;
            return valor_int(resto);
        }
        case AST_POTENCIA: {
            if (b < 0) return valor_real(pow((double)a, (double)b));
            long long resultado = 1;
            /* Exponenciacao inteira evita a perda de precisao de pow/double. */
            while (b) {
                if (b % 2) resultado = multiplicar_inteiros(resultado, a, linha);
                b /= 2;
                if (b) a = multiplicar_inteiros(a, a, linha);
            }
            return valor_int(resultado);
        }
        default:
            erro_execucao(linha, "operador aritmetico desconhecido");
            return valor_none();
    }
}

Valor operar_aritmetica(TipoNo op, Valor a, Valor b, int linha) {
    if (op == AST_SOMA && a.tipo == VAL_STR && b.tipo == VAL_STR)
        return concatenar(a.u.texto, b.u.texto, linha);
    if (op == AST_MULTIPLICACAO) {
        if (a.tipo == VAL_STR && (b.tipo == VAL_INT || b.tipo == VAL_BOOL))
            return repetir(a.u.texto, inteiro(b), linha);
        if (b.tipo == VAL_STR && (a.tipo == VAL_INT || a.tipo == VAL_BOOL))
            return repetir(b.u.texto, inteiro(a), linha);
    }
    if (!numerico(a) || !numerico(b)) {
        erro_execucao(linha, "operacao '%s' entre tipos incompativeis ('%s' e '%s')",
                      simbolo(op), valor_nome_tipo(a), valor_nome_tipo(b));
        return valor_none();
    }
    double x = real(a), y = real(b);
    if ((op == AST_DIVISAO || op == AST_DIVINT || op == AST_MODULO) && y == 0.0)
        erro_execucao(linha, "divisao por zero");
    if (op == AST_POTENCIA && x == 0.0 && y < 0.0)
        erro_execucao(linha, "potencia de zero com expoente negativo");
    if (a.tipo != VAL_REAL && b.tipo != VAL_REAL)
        return operar_inteiros(op, inteiro(a), inteiro(b), linha);
    switch (op) {
        case AST_SOMA: return valor_real(x + y);
        case AST_SUBTRACAO: return valor_real(x - y);
        case AST_MULTIPLICACAO: return valor_real(x * y);
        case AST_DIVISAO: return valor_real(x / y);
        case AST_DIVINT: return valor_real(floor(x / y));
        case AST_MODULO: {
            double resto = fmod(x, y);
            if (resto != 0.0 && ((resto < 0.0) != (y < 0.0))) resto += y;
            if (resto == 0.0) resto = copysign(0.0, y);
            return valor_real(resto);
        }
        case AST_POTENCIA: return valor_real(pow(x, y));
        default:
            erro_execucao(linha, "operador aritmetico desconhecido");
            return valor_none();
    }
}

static Valor interpretar_texto(const char *origem, int linha) {
    char *texto = alocar_texto(strlen(origem) + 1, linha);
    size_t destino = 0;
    for (size_t i = 0; origem[i]; i++) {
        if (origem[i] == '\\' && origem[i + 1]) {
            switch (origem[i + 1]) {
                case 'n': texto[destino++] = '\n'; i++; continue;
                case 't': texto[destino++] = '\t'; i++; continue;
                case '\\': case '\'': case '"':
                    texto[destino++] = origem[++i];
                    continue;
                default: break; /* Escape desconhecido preserva barra e letra. */
            }
        }
        texto[destino++] = origem[i];
    }
    texto[destino] = '\0';
    return texto_proprio(texto);
}

Valor executar_expressao(NoAST *no) {
    switch (no->tipo) {
        case AST_NUMERO: {
            double numero;
            if (!ast_numero_valor(no, &numero)) {
                erro_execucao(no->linha, "literal numerico invalido");
                return valor_none();
            }
            /* LLONG_MAX arredonda para 2^63 em double: limite superior estrito. */
            if (isfinite(numero) && numero == floor(numero) &&
                numero >= (double)LLONG_MIN && numero < -(double)LLONG_MIN)
                return valor_int((long long)numero);
            return valor_real(numero);
        }
        case AST_STRING: return interpretar_texto(no->texto, no->linha);
        case AST_BOOL: return valor_bool(no->valor);
        case AST_NONE: return valor_none();
        case AST_NEGATIVO: {
            Valor operando = interpretar_no(no->a);
            Valor resultado = valor_none();
            if (operando.tipo == VAL_REAL) resultado = valor_real(-operando.u.real);
            else if (operando.tipo == VAL_INT || operando.tipo == VAL_BOOL) {
                long long numero = inteiro(operando);
                if (numero == LLONG_MIN)
                    erro_execucao(no->linha, "resultado inteiro fora do intervalo");
                resultado = valor_int(-numero);
            } else erro_execucao(no->linha, "operacao '-' unaria com tipo incompativel ('%s')",
                                 valor_nome_tipo(operando));
            valor_liberar(operando);
            return resultado;
        }
        case AST_SOMA: case AST_SUBTRACAO: case AST_MULTIPLICACAO:
        case AST_DIVISAO: case AST_DIVINT: case AST_MODULO: case AST_POTENCIA: {
            Valor a = interpretar_no(no->a);
            Valor b = interpretar_no(no->b);
            Valor resultado = operar_aritmetica(no->tipo, a, b, no->linha);
            valor_liberar(a);
            valor_liberar(b);
            return resultado;
        }
        default:
            erro_execucao(no->linha, "expressao desconhecida");
            return valor_none();
    }
}
