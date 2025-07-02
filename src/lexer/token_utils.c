#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token_utils.h"

// Optional debug flag (set externally)
extern int LEXER_DEBUG;

/// @brief Create a new token instance
Token make_token(Token_Type type, const char *value, int line, int column) {
    Token token;
    token.type = type;
    token.value = strdup(value);  // Duplicate the string for safety
    token.line = line;
    token.column = column;

    print_token(token);
    return token;
}

/// @brief Free memory allocated for a token's value
void token_free(Token token) {
    if (token.value) {
        free(token.value);
    }
}

/// @brief Print token information (only if LEXER_DEBUG is enabled)
void print_token(Token token) {
    if (LEXER_DEBUG) {
        printf("[Token] Type: %d, Value: '%s', Line: %d, Column: %d\n",
               token.type, token.value, token.line, token.column);
    }
}
