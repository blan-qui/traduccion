
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

#define STACK_SIZE 1000
#define MAX_RULE_LEN 10

typedef enum {
    // No terminales
    JSON, ELEMENT, OBJECT, ARRAY,
    ATTRIBUTES_LIST, ATTRIBUTES_LIST_PRIMA,
    ATTRIBUTE, ATTRIBUTE_NAME, ATTRIBUTE_VALUE,
    ELEMENTS_LIST, ELEMENTS_LIST_PRIMA,

    // Terminales
    T_L_LLAVE, T_R_LLAVE, T_L_CORCHETE, T_R_CORCHETE,
    T_STRING, T_NUMBER, T_PR_TRUE, T_PR_FALSE, T_PR_NULL,
    T_DOS_PUNTOS, T_COMA, T_EOF,

    EPSILON = -1, DOLLAR = -2
} Symbol;

typedef struct {
    Symbol rhs[MAX_RULE_LEN];
    int length;
} Rule;

typedef struct {
    Symbol stack[STACK_SIZE];
    int top;
} Stack;

void push(Stack *s, Symbol sym) {
    if (s->top < STACK_SIZE - 1) s->stack[++s->top] = sym;
}
Symbol pop(Stack *s) {
    return (s->top >= 0) ? s->stack[s->top--] : DOLLAR;
}
Symbol peek(Stack *s) {
    return (s->top >= 0) ? s->stack[s->top] : DOLLAR;
}

const char* symbolToString(Symbol sym) {
    switch (sym) {
        case JSON: return "JSON"; case ELEMENT: return "ELEMENT";
        case OBJECT: return "OBJECT"; case ARRAY: return "ARRAY";
        case ATTRIBUTES_LIST: return "ATTRIBUTES_LIST";
        case ATTRIBUTES_LIST_PRIMA: return "ATTRIBUTES_LIST_PRIMA";
        case ATTRIBUTE: return "ATTRIBUTE"; case ATTRIBUTE_NAME: return "ATTRIBUTE_NAME";
        case ATTRIBUTE_VALUE: return "ATTRIBUTE_VALUE"; case ELEMENTS_LIST: return "ELEMENTS_LIST";
        case ELEMENTS_LIST_PRIMA: return "ELEMENTS_LIST_PRIMA";
        case T_L_LLAVE: return "{"; case T_R_LLAVE: return "}";
        case T_L_CORCHETE: return "["; case T_R_CORCHETE: return "]";
        case T_STRING: return "STRING"; case T_NUMBER: return "NUMBER";
        case T_PR_TRUE: return "PR_TRUE"; case T_PR_FALSE: return "PR_FALSE";
        case T_PR_NULL: return "PR_NULL"; case T_DOS_PUNTOS: return ":";
        case T_COMA: return ","; case T_EOF: return "EOF";
        case EPSILON: return "e"; case DOLLAR: return "$";
        default: return "SYM?";
    }
}

Symbol tokenToSymbol(Token t) {
    switch (t) {
        case L_LLAVE: return T_L_LLAVE; case R_LLAVE: return T_R_LLAVE;
        case L_CORCHETE: return T_L_CORCHETE; case R_CORCHETE: return T_R_CORCHETE;
        case STRING: return T_STRING; case NUMBER: return T_NUMBER;
        case PR_TRUE: return T_PR_TRUE; case PR_FALSE: return T_PR_FALSE;
        case PR_NULL: return T_PR_NULL; case DOS_PUNTOS: return T_DOS_PUNTOS;
        case COMA: return T_COMA; case EOF_TOKEN: return T_EOF;
        default: return -3;
    }
}

int symbolToColumn(Symbol s) {
    return s - T_L_LLAVE;
}

// Tabla de parsing
Rule table[11][12]; // 11 no terminales × 12 terminales

void initTable() {
    memset(table, 0, sizeof(table));

    table[JSON][symbolToColumn(T_L_LLAVE)] = (Rule){{ELEMENT, T_EOF}, 2};
    table[JSON][symbolToColumn(T_L_CORCHETE)] = (Rule){{ELEMENT, T_EOF}, 2};

    table[ELEMENT][symbolToColumn(T_L_LLAVE)] = (Rule){{OBJECT}, 1};
    table[ELEMENT][symbolToColumn(T_L_CORCHETE)] = (Rule){{ARRAY}, 1};

    table[OBJECT][symbolToColumn(T_L_LLAVE)] = (Rule){{T_L_LLAVE, ATTRIBUTES_LIST}, 2};

    table[ATTRIBUTES_LIST][symbolToColumn(T_STRING)] = (Rule){{ATTRIBUTE, ATTRIBUTES_LIST_PRIMA, T_R_LLAVE}, 3};
    table[ATTRIBUTES_LIST][symbolToColumn(T_R_LLAVE)] = (Rule){{T_R_LLAVE}, 1};

    table[ATTRIBUTES_LIST_PRIMA][symbolToColumn(T_COMA)] = (Rule){{T_COMA, ATTRIBUTE, ATTRIBUTES_LIST_PRIMA}, 3};
    table[ATTRIBUTES_LIST_PRIMA][symbolToColumn(T_R_LLAVE)] = (Rule){{EPSILON}, 1};

    table[ATTRIBUTE][symbolToColumn(T_STRING)] = (Rule){{ATTRIBUTE_NAME, T_DOS_PUNTOS, ATTRIBUTE_VALUE}, 3};
    table[ATTRIBUTE_NAME][symbolToColumn(T_STRING)] = (Rule){{T_STRING}, 1};

    table[ATTRIBUTE_VALUE][symbolToColumn(T_STRING)] = (Rule){{T_STRING}, 1};
    table[ATTRIBUTE_VALUE][symbolToColumn(T_NUMBER)] = (Rule){{T_NUMBER}, 1};
    table[ATTRIBUTE_VALUE][symbolToColumn(T_PR_TRUE)] = (Rule){{T_PR_TRUE}, 1};
    table[ATTRIBUTE_VALUE][symbolToColumn(T_PR_FALSE)] = (Rule){{T_PR_FALSE}, 1};
    table[ATTRIBUTE_VALUE][symbolToColumn(T_PR_NULL)] = (Rule){{T_PR_NULL}, 1};
    table[ATTRIBUTE_VALUE][symbolToColumn(T_L_LLAVE)] = (Rule){{ELEMENT}, 1};
    table[ATTRIBUTE_VALUE][symbolToColumn(T_L_CORCHETE)] = (Rule){{ELEMENT}, 1};

    table[ARRAY][symbolToColumn(T_L_CORCHETE)] = (Rule){{T_L_CORCHETE, ELEMENTS_LIST}, 2};

    table[ELEMENTS_LIST][symbolToColumn(T_R_CORCHETE)] = (Rule){{T_R_CORCHETE}, 1};
    table[ELEMENTS_LIST][symbolToColumn(T_L_LLAVE)] = (Rule){{ELEMENT, ELEMENTS_LIST_PRIMA, T_R_CORCHETE}, 3};
    table[ELEMENTS_LIST][symbolToColumn(T_L_CORCHETE)] = (Rule){{ELEMENT, ELEMENTS_LIST_PRIMA, T_R_CORCHETE}, 3};

    table[ELEMENTS_LIST_PRIMA][symbolToColumn(T_COMA)] = (Rule){{T_COMA, ELEMENT, ELEMENTS_LIST_PRIMA}, 3};
    table[ELEMENTS_LIST_PRIMA][symbolToColumn(T_R_CORCHETE)] = (Rule){{EPSILON}, 1};
}

void parse() {
    Stack s = {.top = -1};
    push(&s, DOLLAR);
    push(&s, JSON);

    Token tok = getToken();

    while (peek(&s) != DOLLAR) {
        Symbol top = peek(&s);
        Symbol tokSym = tokenToSymbol(tok);

        if (top >= T_L_LLAVE) {  // terminal
            if (top == tokSym) {
                pop(&s);
                tok = getToken();
            } else {
                printf("[ERROR] Linea %d: se esperaba '%s' pero se encontro '%s'\n",
                       numLinea, symbolToString(top), symbolToString(tokSym));
                tok = getToken(); // panic
            }
        } else { // no terminal
            Rule r = table[top][symbolToColumn(tokSym)];
            if (r.length == 0) {
                printf("[ERROR] Linea %d: entrada invalida para '%s' con '%s'\n",
                       numLinea, symbolToString(top), symbolToString(tokSym));
                tok = getToken(); // panic
            } else {
                pop(&s);
                int i;
                for (i = r.length - 1; i >= 0; i--) {
                    if (r.rhs[i] != EPSILON)
                        push(&s, r.rhs[i]);
                }
            }
        }
    }

    if (tok == EOF_TOKEN)
        printf("JSON sintacticamente correcto.\n");
    else
        printf("[ERROR] Tokens restantes tras el análisis.\n");
}

int main() {
    initLexer("fuente.txt");
    initTable();
    parse();
    return 0;
}

