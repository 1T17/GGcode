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
    X("return", TOKEN_RETURN) \
    X("step",    TOKEN_STEP)

#define TOKEN_OPERATOR_LIST \
    X("==", TOKEN_EQUAL_EQUAL, "==", 0) \
    X("!=", TOKEN_BANG_EQUAL, "!=", 0) \
    X("<=", TOKEN_LESS_EQUAL, "<=", 0) \
    X(">=", TOKEN_GREATER_EQUAL, ">=", 0) \
    X("&&", TOKEN_AND, "&&", 0) \
    X("&", TOKEN_AMPERSAND, "&", '&') \
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
    X("..<", TOKEN_DOTDOT_LT, "..<", 0) \
    X("..", TOKEN_DOTDOT, "..", 0) \
    X(".", TOKEN_DOT, ".", '.')

#define TOKEN_FUNCTION_LIST \
    /* Basic Arithmetic & Control */ \
    X("abs", TOKEN_FUNC_ABS) \
    X("mod", TOKEN_FUNC_MOD) \
    X("floor", TOKEN_FUNC_FLOOR) \
    X("ceil", TOKEN_FUNC_CEIL) \
    X("round", TOKEN_FUNC_ROUND) \
    X("min", TOKEN_FUNC_MIN) \
    X("max", TOKEN_FUNC_MAX) \
    X("clamp", TOKEN_FUNC_CLAMP) \
    /* Trigonometry */ \
    X("sin", TOKEN_FUNC_SIN) \
    X("cos", TOKEN_FUNC_COS) \
    X("tan", TOKEN_FUNC_TAN) \
    X("asin", TOKEN_FUNC_ASIN) \
    X("acos", TOKEN_FUNC_ACOS) \
    X("atan", TOKEN_FUNC_ATAN) \
    X("atan2", TOKEN_FUNC_ATAN2) \
    X("deg", TOKEN_FUNC_DEG) \
    X("rad", TOKEN_FUNC_RAD) \
    /* Geometry / Vector Helpers */ \
    X("sqrt", TOKEN_FUNC_SQRT) \
    X("pow", TOKEN_FUNC_POW) \
    X("hypot", TOKEN_FUNC_HYPOT) \
    X("lerp", TOKEN_FUNC_LERP) \
    X("map", TOKEN_FUNC_MAP) \
    X("distance", TOKEN_FUNC_DISTANCE) \
    /* Constants */ \
    X("PI", TOKEN_FUNC_PI) \
    X("TAU", TOKEN_FUNC_TAU) \
    X("EU", TOKEN_FUNC_EU) \
    X("DEG_TO_RAD", TOKEN_FUNC_DEG_TO_RAD) \
    X("RAD_TO_DEG", TOKEN_FUNC_RAD_TO_DEG) \
    /* Optional / Advanced */ \
    X("noise", TOKEN_FUNC_NOISE) \
    X("sign", TOKEN_FUNC_SIGN) \
    X("log", TOKEN_FUNC_LOG) \
    X("exp", TOKEN_FUNC_EXP)


#endif // KEYWORD_TABLE_H
