#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include <stdio.h>

#include "syntactic_node.h"

void generate_program(const SyntacticNode* node, int nb_var, FILE * stream);
void generate_code(const SyntacticNode* node, FILE * stream);

#endif // CODE_GENERATION_H