#include "code_generation.h"

#include <assert.h>
#include <stdio.h>

void generate_program(const SyntacticNode* program)
{
	assert(program != NULL);

	printf(".start\n");
	generate_code(program);
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
		case NODE_ADD :
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("add\n");
			break;
		}
		case NODE_SUB:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("sub\n");
			break;
		}
		case NODE_MUL:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("mul\n");
			break;
		}
		case NODE_DIV:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("div\n");
			break;
		}
		case NODE_MOD:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("mod\n");
			break;
		}
		case NODE_AND:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("and\n");
			break;
		}
		case NODE_OR:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("or\n");
			break;
		}
		case NODE_EQUAL:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("cmpeq\n");
			break;
		}
		case NODE_NOT_EQUAL:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("cmpne\n");
			break;
		}
		case NODE_LESS:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("cmplt\n");
			break;
		}
		case NODE_LESS_OR_EQUAL:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("cmple\n");
			break;
		}
		case NODE_GREATER:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("cmpgt\n");
			break;
		}
		case NODE_GREATER_OR_EQUAL:
		{
			generate_code(node->children[0]);
			generate_code(node->children[1]);
			printf("cmpge\n");
			break;
		}
		case NODE_PRINT:
		{
			generate_code(node->children[0]);
			printf("dbg\n");
			break;
		}
		case NODE_SEQUENCE:
		case NODE_BLOCK:
		{
			for (int i = 0; i < node->nb_children; i++)
			{
				generate_code(node->children[i]);
			}
			break;
		}
		case NODE_DROP:
		{
			generate_code(node->children[0]);
			printf("drop\n");
			break;
		}
		case NODE_CONST: printf("push %d\n", node->value.int_val); break;
	}
}