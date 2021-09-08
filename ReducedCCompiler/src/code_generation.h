#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include "syntactic_node.h"

void generate_program(const SyntacticNode* node);
void generate_code(const SyntacticNode* node);

#endif // CODE_GENERATION_H