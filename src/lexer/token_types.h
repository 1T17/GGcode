#ifndef TOKEN_TYPES_H
#define TOKEN_TYPES_H

#include "keyword_table.h"

typedef enum {

    // Keywords
    #define X(name, type) type,
    KEYWORD_LIST
    #undef X

    // Operators, delimiters, dot patterns
    #define X(text, type, str, ch) type,
    TOKEN_OPERATOR_LIST
    TOKEN_DELIMITER_LIST
    TOKEN_DOT_LIST
    #undef X

    // Built-in functions and constants
    #define X(name, type) type,
    TOKEN_FUNCTION_LIST
    #undef X

    // Identifiers and literals
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_GCODE_WORD,      // G0, G1, M3, etc.

    // Special
    TOKEN_BLOCK_COMMENT,   // /% %/
    TOKEN_NEWLINE,
    TOKEN_EOF,
    TOKEN_UNKNOWN,

} Token_Type;

#endif // TOKEN_TYPES_H
