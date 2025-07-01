#ifndef KEYWORD_TABLE_H
#define KEYWORD_TABLE_H

// The full list of keywords and their token types
#define KEYWORD_LIST \
    X("let",     TOKEN_LET) \
    X("for",     TOKEN_FOR) \
    X("while",   TOKEN_WHILE) \
    X("if",      TOKEN_IF) \
    X("else",    TOKEN_ELSE) \
    X("note",    TOKEN_NOTE) \
    X("function", TOKEN_FUNCTION) \
    X("return", TOKEN_RETURN)

#define TOKEN_OPERATOR_LIST \
    X("==", TOKEN_EQUAL_EQUAL, "==", 0) \
    X("!=", TOKEN_BANG_EQUAL, "!=", 0) \
    X("<=", TOKEN_LESS_EQUAL, "<=", 0) \
    X(">=", TOKEN_GREATER_EQUAL, ">=", 0) \
    X("&&", TOKEN_AND, "&&", 0) \
    X("||", TOKEN_OR, "||", 0) \
    X("+", TOKEN_PLUS, "+", '+') \
    X("-", TOKEN_MINUS, "-", '-') \
    X("*", TOKEN_STAR, "*", '*') \
    X("/", TOKEN_SLASH, "/", '/') \
    X("=", TOKEN_EQUAL, "=", '=') \
    X("!", TOKEN_BANG, "!", '!') \
    X("<", TOKEN_LESS, "<", '<') \
    X(">", TOKEN_GREATER, ">", '>')

#define TOKEN_DELIMITER_LIST \
    X("(", TOKEN_LPAREN, "(", '(') \
    X(")", TOKEN_RPAREN, ")", ')') \
    X("{", TOKEN_LBRACE, "{", '{') \
    X("}", TOKEN_RBRACE, "}", '}') \
    X("[", TOKEN_LBRACKET, "[", '[') \
    X("]", TOKEN_RBRACKET, "]", ']') \
    X(",", TOKEN_COMMA, ",", ',') \
    X(";", TOKEN_SEMICOLON, ";", ';')

#define TOKEN_DOT_LIST \
    X("...", TOKEN_DOTDOTDOT, "...", 0) \
    X("..", TOKEN_DOTDOT, "..", 0) \
    X(".", TOKEN_DOT, ".", '.')

#endif // KEYWORD_TABLE_H
