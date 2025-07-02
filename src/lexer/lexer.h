#ifndef LEXER_H
#define LEXER_H

#include "token_types.h"
#include "keyword_table.h"

typedef struct {
    Token_Type type;
    char* value;
    int line;
    int column;
} Token;

typedef struct {
    const char* source;
    int pos;
    int line;
    int column;
} Lexer;

Lexer* lexer_new(const char* source);
void lexer_free(Lexer* lexer);     
Token lexer_next_token(Lexer* lexer);

#endif // LEXER_H
