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

Rule table[11][12];
FILE *salida;
Token currentToken;

Symbol tokenToSymbol(Token t) {
    switch (t) {
        case L_LLAVE: return T_L_LLAVE;
        case R_LLAVE: return T_R_LLAVE;
        case L_CORCHETE: return T_L_CORCHETE;
        case R_CORCHETE: return T_R_CORCHETE;
        case STRING: return T_STRING;
        case NUMBER: return T_NUMBER;
        case PR_TRUE: return T_PR_TRUE;
        case PR_FALSE: return T_PR_FALSE;
        case PR_NULL: return T_PR_NULL;
        case DOS_PUNTOS: return T_DOS_PUNTOS;
        case COMA: return T_COMA;
        case EOF_TOKEN: return T_EOF;
        default: return -3;
    }
}

int symbolToColumn(Symbol s) {
    return s - T_L_LLAVE;
}

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

void parseSymbol(Symbol sym);

void parseSymbol(Symbol sym) {
    Symbol tokSym = tokenToSymbol(currentToken);

    if (sym >= T_L_LLAVE) {
        if (sym == tokSym) {
            if (sym == T_STRING || sym == T_NUMBER || sym == T_PR_TRUE || sym == T_PR_FALSE || sym == T_PR_NULL) {
                fprintf(salida, "%s", lexema);
            }
            currentToken = getToken();
        } else {
            fprintf(stderr, "[ERROR] Linea %d: se esperaba '%d' pero se encontro '%d'\n", numLinea, sym, tokSym);
            currentToken = getToken();
        }
    } else {
        Rule r = table[sym][symbolToColumn(tokSym)];
        if (r.length == 0) {
            fprintf(stderr, "[ERROR] Linea %d: entrada invalida para '%d' con '%d'\n", numLinea, sym, tokSym);
            currentToken = getToken();
        } else {
            if (sym == ATTRIBUTE) {
                char etiqueta[256];
                strcpy(etiqueta, lexema);
                currentToken = getToken();
                currentToken = getToken(); // saltar ':'
                fprintf(salida, "<%s>", etiqueta);
                parseSymbol(ATTRIBUTE_VALUE);
                fprintf(salida, "</%s>\n", etiqueta);
                return;
            } else if (sym == ELEMENTS_LIST || sym == ELEMENTS_LIST_PRIMA) {
                if (sym == ELEMENTS_LIST || (sym == ELEMENTS_LIST_PRIMA && tokSym == T_COMA))
                    fprintf(salida, "<item>\n");
                int i;
                for (i = 0; i < r.length; i++) {
                    if (r.rhs[i] != EPSILON)
                        parseSymbol(r.rhs[i]);
                }
                if (sym == ELEMENTS_LIST || (sym == ELEMENTS_LIST_PRIMA && tokSym == T_COMA))
                    fprintf(salida, "</item>\n");
                return;
            }
            int i;
            for ( i = 0; i < r.length; i++) {
                if (r.rhs[i] != EPSILON)
                    parseSymbol(r.rhs[i]);
            }
        }
    }
}

int main() {
    initLexer("fuente.txt");
    initTable();
    salida = fopen("traducido.xml", "w");
    if (!salida) {
        fprintf(stderr, "No se pudo abrir archivo XML\n");
        return 1;
    }
    currentToken = getToken();
    parseSymbol(JSON);
    fclose(salida);
    return 0;
}

