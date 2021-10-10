#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include <stdio.h>

#include "syntactic_node.h"

#define NO_LOOP -1

void generate_program(const SyntacticNode* node, FILE * stream, int is_init_called);
void generate_code(const SyntacticNode* node, FILE * stream, int loop_nb);

#endif // CODE_GENERATION_H
