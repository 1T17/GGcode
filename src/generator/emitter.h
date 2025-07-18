#ifndef EMITTER_H
#define EMITTER_H

#include "parser/parser.h"

extern int statement_count; 


void emit_gcode(ASTNode* node, int debug);

void emit_block_stmt(ASTNode* node, int debug);
int get_statement_count();
void reset_emitter_state(void);



#endif // EMITTER_H
