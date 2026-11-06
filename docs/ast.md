# Árvore sintática abstrata

A AST registra a estrutura do programa que será analisado e executado. O parser
constrói a árvore em `Semantica/Ast/ast.c`; a análise semântica a percorre antes
de o interpretador avaliar os nós. Parênteses e terminadores organizam a
gramática, mas não precisam de nós próprios.

~~~sh
./parser --ast < programa.txt
~~~

O modo `--ast` mostra reconhecimentos e a árvore, realiza a análise semântica e
encerra sem executar o programa. O modo padrão executa a mesma árvore somente
quando não há erros léxicos, sintáticos ou semânticos.

## Estrutura comum

`Semantica/Ast/ast.h` define todos os tipos de nó e a estrutura compartilhada:

~~~c
typedef struct NoAST {
    TipoNo tipo;
    int linha;
    int valor;
    char *texto;
    struct NoAST *a, *b, *c;
    struct NoAST *prox;
} NoAST;
~~~

`tipo` determina o significado dos filhos `a`, `b` e `c`. `linha` preserva a
localização usada nos diagnósticos. `texto` guarda uma cópia de identificadores,
strings e nomes de funções ou variáveis de controle.

`prox` encadeia comandos, elementos de lista, parâmetros e argumentos. O primeiro
item representa a sequência inteira; uma sequência vazia é `NULL`. A travessia
dos filhos tem significado diferente da travessia dos próximos itens.

## Formato dos nós

Os nomes abaixo omitem o prefixo `AST_` do enum `TipoNo`.

| Nó | Dados e filhos |
| --- | --- |
| `PROGRAMA`, `BLOCO` | `a`: primeiro comando; os demais seguem por `prox` |
| `NUMERO` | Valor obtido com `ast_numero_valor` |
| `STRING` | `texto`: conteúdo sem as aspas delimitadoras |
| `BOOL` | `valor`: 0 ou 1 |
| `NONE` | Sem filhos |
| `IDENTIFICADOR` | `texto`: nome da variável ou função |
| `SOMA`, `SUBTRACAO`, `MULTIPLICACAO`, `DIVISAO`, `DIVINT`, `MODULO`, `POTENCIA` | `a`: operando esquerdo; `b`: operando direito |
| `NEGATIVO` | `a`: operando |
| `IGUAL`, `DIFERENTE`, `MENOR`, `MAIOR`, `MENOR_IGUAL`, `MAIOR_IGUAL`, `E`, `OU` | `a`: operando esquerdo; `b`: operando direito |
| `NAO` | `a`: operando |
| `IF` | `a`: condição; `b`: bloco quando verdadeira; `c`: bloco `else`, outro `IF` para `elif`, ou `NULL` |
| `WHILE` | `a`: condição; `b`: corpo |
| `FOR` | `texto`: variável de controle; `a`: iterável; `b`: corpo |
| `BREAK`, `CONTINUE` | Sem filhos |
| `ATRIBUICAO`, `ATRIB_SOMA`, `ATRIB_SUB`, `ATRIB_MULT`, `ATRIB_DIV` | `a`: alvo; `b`: expressão do valor |
| `LISTA` | `a`: primeiro elemento; os demais seguem por `prox` |
| `INDEXACAO` | `a`: identificador da lista; `b`: índice |
| `FUNCAO` | `texto`: nome; `a`: primeiro parâmetro; `b`: bloco do corpo |
| `PARAMETRO` | `texto`: nome do parâmetro; os demais seguem por `prox` |
| `RETURN` | `a`: expressão devolvida, ou `NULL` para `return;` |
| `CHAMADA` | `texto`: nome chamado; `a`: primeiro argumento; os demais seguem por `prox` |

Uma expressão usada como comando, como `3 + 4;`, aparece diretamente na
sequência de comandos. Na execução, seu valor é calculado e descartado.

## Exemplo de construção

~~~text
x = 3 + 4 * 2;
~~~

A precedência está representada pela posição dos nós:

~~~text
PROGRAMA [linha 1]
  ATRIBUICAO [linha 1]
    IDENTIFICADOR 'x' [linha 1]
    SOMA [linha 1]
      NUMERO 3 [linha 1]
      MULTIPLICACAO [linha 1]
        NUMERO 4 [linha 1]
        NUMERO 2 [linha 1]
~~~

Em `itens = [1, 2]`, o filho `b` da atribuição é uma `LISTA`. Seu filho `a`
aponta para o número `1`, cujo `prox` aponta para `2`. Em `itens[0] += 3`, o
alvo da atribuição composta é uma `INDEXACAO`.

A gramática aceita indexação diretamente por nome, como `itens[i]`. Para
trabalhar com uma sublista, atribua-a a outro nome antes de indexá-la:

~~~text
matriz = [[1, 2], [3, 4]];
linha = matriz[0];
linha[1] = 9;
~~~

## Números, strings e memória

O campo `valor` permanece inteiro no contrato de `NoAST`. Literais reais usam
um registro auxiliar associado ao endereço do nó. A função
`ast_numero_valor(no, &numero)` recupera o `double` correto; ler `no->valor`
diretamente não recupera necessariamente o valor de um literal numérico.

As strings guardam os escapes ainda escritos como barra e letra. A execução
converte os escapes de quebra de linha, tabulação, barra e aspas ao construir
o valor de texto.

Cada nó é dono de seus filhos e da cópia de `texto`. `ast_liberar` percorre
`a`, `b`, `c` e `prox`, remove o registro numérico auxiliar e libera o nó. A
árvore não deve compartilhar um mesmo nó entre pais nem conter ciclos.
As ações de recuperação sintática liberam subárvores que foram descartadas.

As listas da AST representam a estrutura do código; as listas de valores do
interpretador são objetos separados, compartilhados por referência. Uma função
em execução conserva um ponteiro para seu nó `FUNCAO`, que continua pertencendo
à AST até o encerramento do programa.

Veja também a [análise semântica](analisador_semantico/analisador-semantico.md),
as [decisões técnicas](decisoes.md) e o [interpretador](interpretador.md).
