#include "code_generation.h"

#include <assert.h>
#include <stdio.h>

void generate_program(const SyntacticNode* program)
{
	assert(program != NULL);

	printf(".start\n");
	generate_code(program);
	printf("dbg\n");
	printf("halt\n");
}

void generate_code(const SyntacticNode* node)
{
	assert(node != NULL);

	switch (node->type)
	{
		
		case NODE_NEGATION:
		{
			generate_code(node->children[0]);
			printf("not\n");
			break;
		}
		case NODE_UNARY_MINUS:
		{
			printf("push 0\n");
			generate_code(node->children[0]);
			printf("sub\n");
			break;
		}
		case NODE_CONST:       printf("push %d\n", node->value); break;
	}
}