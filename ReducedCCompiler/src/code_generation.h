#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include <stdio.h>

#include "syntactic_node.h"

void generate_program(const SyntacticNode* node, FILE * stream);
void generate_code(const SyntacticNode* node, FILE * stream, int loop_nb);

#endif // CODE_GENERATION_H