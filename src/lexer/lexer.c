#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "../utils/compat.h"
#include "../error/error.h"
#include "lexer.h"
#include "token_utils.h"



Lexer *lexer_new(const char *source) {
     //printf("[lexer_new]\n");
    if (!source) {
        report_error("[Lexer] ERROR: source is NULL!");
        return NULL;
    }

    // Safe defensive copy
    char *copy = strdup(source);
    if (!copy) {
        report_error("[Lexer] ERROR: strdup failed");
        return NULL;
    }

    //printf("[Lexer] Copied string to internal buffer:\n---\n%s\n---\n", copy);

    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    if (!lexer) {
        report_error("[Lexer] ERROR: malloc failed for lexer struct");
        free(copy);
        return NULL;
    }

    lexer->source = copy;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;

    //printf("[Lexer] Lexer successfully created. Starting at line 1, column 1\n");

    return lexer;
}

/// @brief Frees the memory for a lexer
/// @param lexer The lexer to free
void lexer_free(Lexer *lexer)
{
    if (lexer) {
        if (lexer->source) {
            free((char*)lexer->source);
            lexer->source = NULL;
        }
        free(lexer);
    }
}

/// @brief Peeks at the current character
static char peek(Lexer *lexer)
{
    if (!lexer || !lexer->source)
        return '\0';

    return lexer->source[lexer->pos];
}

/// @brief Peeks at the next character
static char peek_next(Lexer *lexer)
{
    if (!lexer || !lexer->source)
        return '\0';

    return lexer->source[lexer->pos + 1];
}

/// @brief Peeks ahead by a fixed offset
static char peek_ahead(Lexer *lexer, int offset)
{
    if (!lexer || !lexer->source)
        return '\0';

    int pos = lexer->pos + offset;
    return lexer->source[pos] != '\0' ? lexer->source[pos] : '\0';
}

/// @brief Advances the lexer by one character
static char advance(Lexer *lexer)
{
    char c = lexer->source[lexer->pos++];
    lexer->column++;

    //printf("[advance] Char: '%c' (0x%02X), Pos: %d, Line: %d, Column: %d\n",
       //    (c >= 32 && c <= 126) ? c : '.', // printable char or dot
       //    (unsigned char)c,
        //   lexer->pos - 1,
         //  lexer->line,
         //  lexer->column - 1);

    if (c == '\n')
    {
        lexer->line++;
        lexer->column = 1;
        //printf("[advance] Newline encountered → Line: %d, Reset Column to 1\n", lexer->line);
    }

    return c;
}

/// @brief Skips whitespace, comments, and semicolons
static void skip_whitespace(Lexer *lexer)
{
    while (1)
    {
        char c = peek(lexer);

 if (c == '\n') {
    advance(lexer);
} else if (isspace(c)) {
    advance(lexer);
}
        // Skip semicolons - they're optional in GGcode
        else if (c == ';')
        {
            advance(lexer);
        }
        else if (c == '/' && peek_next(lexer) == '/')
        {
            while (peek(lexer) != '\n' && peek(lexer) != '\0')
                advance(lexer);
        }
        else if (c == '/' && peek_next(lexer) == '*')
        {
            advance(lexer);
            advance(lexer);
            while (!(peek(lexer) == '*' && peek_next(lexer) == '/') && peek(lexer) != '\0')
            {
                if (peek(lexer) == '\n')
                {
                    lexer->line++;
                    lexer->column = 1;
                }
                advance(lexer);
            }
            if (peek(lexer) == '*' && peek_next(lexer) == '/')
            {
                advance(lexer);
                advance(lexer);
            }
        }
        else
        {
            break;
        }
    }
}

/// @brief Lookup keyword and return its token type
static Token_Type keyword_lookup(const char *word)
{
#define X(name, type) if (strcmp(word, name) == 0) return type;
    KEYWORD_LIST
    TOKEN_FUNCTION_LIST
#undef X
    return TOKEN_IDENTIFIER;
}

/// @brief Main lexer function to get the next token
Token lexer_next_token(Lexer *lexer)
{

    //printf("[lexer_next_token] peek: '%c' (0x%02X) at pos: %d\n", peek(lexer), peek(lexer), lexer->pos);

    skip_whitespace(lexer);



    int start_col = lexer->column;
    char c = peek(lexer);

    if (c == '\0')
        return make_token(TOKEN_EOF, "EOF", lexer->line, lexer->column);

    // Identifiers, keywords, G-code words
if (isalpha(c) || c == '_') {
    int start = lexer->pos; // mark start before any advance
    advance(lexer); // consume the first character

    while (isalnum(peek(lexer)) || peek(lexer) == '_')
        advance(lexer);


int len = lexer->pos - start;
char *word = strndup_portable(&lexer->source[start], len);

Token_Type type = keyword_lookup(word);

if (type == TOKEN_IDENTIFIER && (word[0] == 'G' || word[0] == 'M' || word[0] == 'T'))
    type = TOKEN_GCODE_WORD;

Token token = make_token(type, word, lexer->line, start_col);
free(word);  // Free the temporary word buffer
return token;

}

    // String literals
    if (c == '"') {
        int start_line = lexer->line;
        int start_column = lexer->column;
        advance(lexer); // consume opening quote
        
        // Calculate buffer size needed (worst case: every char is escaped)
        int capacity = 64;
        char *buffer = malloc(capacity);
        if (!buffer) {
            report_error("[Lexer] ERROR: Memory allocation failed for string literal");
            return make_token(TOKEN_UNKNOWN, "", lexer->line, start_col);
        }
        
        int len = 0;
        
        while (peek(lexer) != '"' && peek(lexer) != '\0') {
            // Expand buffer if needed
            if (len >= capacity - 2) {
                capacity *= 2;
                char *new_buffer = realloc(buffer, capacity);
                if (!new_buffer) {
                    free(buffer);
                    report_error("[Lexer] ERROR: Memory allocation failed for string literal");
                    return make_token(TOKEN_UNKNOWN, "", lexer->line, start_col);
                }
                buffer = new_buffer;
            }
            
            char ch = peek(lexer);
            if (ch == '\\') {
                advance(lexer); // consume backslash
                char escaped = peek(lexer);
                if (escaped == '\0') {
                    free(buffer);
                    report_error("[Lexer] ERROR: Unterminated string literal at line %d, column %d", start_line, start_column);
                    return make_token(TOKEN_UNKNOWN, "", lexer->line, start_col);
                }
                
                switch (escaped) {
                    case '"':
                        buffer[len++] = '"';
                        break;
                    case 'n':
                        buffer[len++] = '\n';
                        break;
                    case 't':
                        buffer[len++] = '\t';
                        break;
                    case '\\':
                        buffer[len++] = '\\';
                        break;
                    default:
                        free(buffer);
                        report_error("[Lexer] ERROR: Invalid escape sequence '\\%c' at line %d, column %d", escaped, lexer->line, lexer->column);
                        // Consume the rest of the string to avoid further parsing errors
                        while (peek(lexer) != '"' && peek(lexer) != '\0') {
                            advance(lexer);
                        }
                        if (peek(lexer) == '"') {
                            advance(lexer); // consume closing quote
                        }
                        return make_token(TOKEN_UNKNOWN, "", lexer->line, start_col);
                }
                advance(lexer); // consume escaped character
            } else {
                buffer[len++] = ch;
                advance(lexer);
            }
        }
        
        if (peek(lexer) != '"') {
            free(buffer);
            report_error("[Lexer] ERROR: Unterminated string literal at line %d, column %d", start_line, start_column);
            return make_token(TOKEN_UNKNOWN, "", lexer->line, start_col);
        }
        
        advance(lexer); // consume closing quote
        buffer[len] = '\0';
        
        Token token = make_token(TOKEN_STRING, buffer, start_line, start_column);
        free(buffer);
        return token;
    }

    // Number literals (including negative and dot-prefixed)
    if ((c == '-' && (isdigit(peek_next(lexer)) || peek_next(lexer) == '.')) ||
        isdigit(c) || (c == '.' && isdigit(peek_next(lexer))))
    {
        int allow_unary = 0;
        if (c == '-') {
            int prev = lexer->pos - 1;
            while (prev > 0 && isspace(lexer->source[prev]))
                prev--;
            if (prev == 0 || lexer->source[prev] == '=' || lexer->source[prev] == '(' || lexer->source[prev] == '[')
                allow_unary = 1;
        } else {
            allow_unary = 1;
        }

        if (allow_unary) {
            int start = lexer->pos;
            int seen_dot = 0;
            advance(lexer);

            while (1) {
                char next = peek(lexer);
                char next2 = peek_next(lexer);
                if (isdigit(next)) {
                    advance(lexer);
                } else if (next == '.' && next2 == '.') {
                    break; // avoid consuming range operators
                } else if (next == '.') {
                    if (!seen_dot) {
                        seen_dot = 1;
                        advance(lexer);
                    } else {
                        advance(lexer);
                    }
                } else {
                    break;
                }
            }

            int len = lexer->pos - start;
         char *num = strndup_portable(&lexer->source[start], len);
            int j = 0, dot_seen = 0;
            for (int i = 0; i < len; i++) {
                if (num[i] == '.') {
                    if (!dot_seen) {
                        dot_seen = 1;
                        num[j++] = '.';
                    }
                } else {
                    num[j++] = num[i];
                }
            }
            num[j] = '\0';
            Token token = make_token(TOKEN_NUMBER, num, lexer->line, start_col);
            free(num);  // Free the temporary num buffer
            return token;
        }
    }

// Special check for '...', '..<', and '..'
if (c == '.' && peek_next(lexer) == '.' && peek_ahead(lexer, 2) == '.') {
    advance(lexer); advance(lexer); advance(lexer);
    return make_token(TOKEN_DOTDOTDOT, "...", lexer->line, start_col);
}
if (c == '.' && peek_next(lexer) == '.' && peek_ahead(lexer, 2) == '<') {
    advance(lexer); advance(lexer); advance(lexer);
    return make_token(TOKEN_DOTDOT_LT, "..<", lexer->line, start_col);
}
if (c == '.' && peek_next(lexer) == '.') {
    advance(lexer); advance(lexer);
    return make_token(TOKEN_DOTDOT, "..", lexer->line, start_col);
}

    // General operators, delimiters, dot
#define X(text, type, str, ch) \
    if (strncmp(&lexer->source[lexer->pos], text, strlen(text)) == 0) { \
        lexer->pos += strlen(text); \
        lexer->column += strlen(text); \
        return make_token(type, text, lexer->line, start_col); \
    }

    TOKEN_OPERATOR_LIST
    TOKEN_DELIMITER_LIST
    TOKEN_DOT_LIST

#undef X

    // Unknown character fallback
    advance(lexer);
    char unknown[2] = {c, '\0'};
    return make_token(TOKEN_UNKNOWN, unknown, lexer->line, start_col);
}
