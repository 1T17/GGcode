#ifndef EMITTER_H
#define EMITTER_H

#include "parser/parser.h"

void emit_gcode(ASTNode* node, int debug);
int get_statement_count();



#endif // EMITTER_H
