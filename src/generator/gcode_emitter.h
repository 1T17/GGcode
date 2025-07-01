#ifndef GCODE_EMITTER_H
#define GCODE_EMITTER_H

#include "parser/parser.h"

void emit_gcode(ASTNode* node, int debug);
int get_statement_count();



#endif // GCODE_EMITTER_H
