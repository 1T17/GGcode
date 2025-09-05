#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token_utils.h"



/// @brief Create a new token instance




Token make_token(Token_Type type, const char *value, int line, int column) {
    Token token;
    token.type = type;
    token.value = strdup(value);  // Duplicate the string for safety
    token.line = line;
    token.column = column;

    return token;
}

void token_free(Token token) {
    if (token.value) {
        free(token.value);
    }
}





