#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "../parser/ast_nodes.h"

void set_var(const char* name, double value);
double get_var(const char* name);
int var_exists(const char* name);         // ✅ Add this line
double eval_expr(ASTNode* node);

#endif
