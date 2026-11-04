/* ============================================================================
 * P3 (Gabriel) - LACOS - trecho de Semantica/Ast/ast.c
 *
 * Sao 2 pedacos de "case", para colar nos switches que P1 criou, dentro da
 * secao [P3]. Os 4 membros do enum (AST_WHILE, AST_FOR, AST_BREAK,
 * AST_CONTINUE) ja fazem parte do contrato (secao 3.1) e ficam no ast.h do P1.
 *
 * Suposicao sobre o ast_imprimir do P1: o switch so imprime o CABECALHO do no
 * (indentacao ja foi feita antes do switch) e depois uma parte comum recursa em
 * a, b e c com nivel + 1 e segue pelo prox. Se o seu switch recursa dentro de
 * cada case, acrescente em WHILE e FOR:
 *     ast_imprimir(no->a, nivel + 1);  ast_imprimir(no->b, nivel + 1);
 * Se o ponteiro do no tem outro nome (raiz, n...), troque "no".
 * ========================================================================== */


/* ---------- PEDACO 1: dentro do switch de ast_nome_tipo() ------------------ */
    /* ===== [P3] LACOS ===== */
    case AST_WHILE:    return "WHILE";
    case AST_FOR:      return "FOR";
    case AST_BREAK:    return "BREAK";
    case AST_CONTINUE: return "CONTINUE";
    /* (se P1 usou um vetor de nomes em vez de switch, acrescente "WHILE",
     *  "FOR", "BREAK", "CONTINUE" na posicao da secao [P3] do enum) */


/* ===== [P3] LAÇOS ===== */
case AST_WHILE:
    printf("%*sWHILE [linha %d]\n", nivel * 2, "", raiz->linha);
    if (raiz->a) ast_imprimir(raiz->a, nivel + 1);
    if (raiz->b) ast_imprimir(raiz->b, nivel + 1);
    break;

case AST_FOR:
    printf("%*sFOR '%s' [linha %d]\n", nivel * 2, "", raiz->texto ? raiz->texto : "", raiz->linha);
    if (raiz->a) ast_imprimir(raiz->a, nivel + 1);
    if (raiz->b) ast_imprimir(raiz->b, nivel + 1);
    break;

case AST_BREAK:
    printf("%*sBREAK [linha %d]\n", nivel * 2, "", raiz->linha);
    break;

case AST_CONTINUE:
    printf("%*sCONTINUE [linha %d]\n", nivel * 2, "", raiz->linha);
    break;