#ifndef TOKEN_UTILS_H
#define TOKEN_UTILS_H

#include "token_types.h" // For Token_Type
#include "lexer.h"       // For Token structure

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new Token object.
 * @param type Token_Type enum value.
 * @param value The string value of the token (will be duplicated).
 * @param line Line number where the token appears.
 * @param column Column number where the token starts.
 * @return A populated Token struct.
 */
Token make_token(Token_Type type, const char *value, int line, int column);

/**
 * @brief Free memory associated with a Token (its value string).
 * @param token The token to clean up.
 */
void token_free(Token token);



#ifdef __cplusplus
}
#endif

#endif // TOKEN_UTILS_H
