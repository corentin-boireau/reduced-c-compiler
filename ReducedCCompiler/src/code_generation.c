#include "code_generation.h"

void generate_program(SyntacticNode* program)
{
	printf(".start\n");
	generate_code(program);
	// printf(".dbg\n");
	printf(".halt\n");
}

void generate_code(SyntacticNode* node)
{
	switch (node->type)
	{
		
		case NODE_NEGATION:
		{
			generate_code(node->children[0]);
			printf("neg\n");
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
