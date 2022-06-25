#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include <stdio.h>

#include "syntactic_node.h"

#define NO_LOOP -1

void generate_program(SyntacticNode* program, FILE * stream, int is_init_called, int nb_global_variables, SyntacticNode** global_declarations);
void generate_code(SyntacticNode* node, FILE * stream, int loop_nb, int nb_global_variables, SyntacticNode** global_declarations);

#endif // CODE_GENERATION_H
