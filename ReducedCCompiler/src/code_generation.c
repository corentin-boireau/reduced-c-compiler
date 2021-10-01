#include "code_generation.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void generate_program(const SyntacticNode* program, int nb_var, FILE * stream)
{
	assert(program != NULL);

	fprintf(stream, ".start\n");
	fprintf(stream, "resn %d\n", nb_var);
	generate_code(program, stream);
	fprintf(stream, "halt\n");
}

void generate_code(const SyntacticNode* node, FILE * stream)
{
	assert(node != NULL);

	switch (node->type)
	{
		case NODE_NEGATION:
		{
			generate_code(node->children[0], stream);
			fprintf(stream, "not\n");
			break;
		}
		case NODE_UNARY_MINUS:
		{
			fprintf(stream, "push 0\n");
			generate_code(node->children[0], stream);
			fprintf(stream, "sub\n");
			break;
		}
		case NODE_ADD :
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "add\n");
			break;
		}
		case NODE_SUB:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "sub\n");
			break;
		}
		case NODE_MUL:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "mul\n");
			break;
		}
		case NODE_DIV:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "div\n");
			break;
		}
		case NODE_MOD:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "mod\n");
			break;
		}
		case NODE_AND:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "and\n");
			break;
		}
		case NODE_OR:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "or\n");
			break;
		}
		case NODE_EQUAL:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "cmpeq\n");
			break;
		}
		case NODE_NOT_EQUAL:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "cmpne\n");
			break;
		}
		case NODE_LESS:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "cmplt\n");
			break;
		}
		case NODE_LESS_OR_EQUAL:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "cmple\n");
			break;
		}
		case NODE_GREATER:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "cmpgt\n");
			break;
		}
		case NODE_GREATER_OR_EQUAL:
		{
			generate_code(node->children[0], stream);
			generate_code(node->children[1], stream);
			fprintf(stream, "cmpge\n");
			break;
		}
		case NODE_PRINT:
		{
			generate_code(node->children[0], stream);
			fprintf(stream, "dbg\n");
			break;
		}
		case NODE_SEQUENCE:
		case NODE_BLOCK:
		{
			for (int i = 0; i < node->nb_children; i++)
			{
				generate_code(node->children[i], stream);
			}
			break;
		}
		case NODE_DROP:
		{
			generate_code(node->children[0], stream);
			fprintf(stream, "drop\n");
			break;
		}
		case NODE_DECL :
			break; 
		case NODE_REF:
		{
			fprintf(stream, "get %d\n", node->index);
			break;
		}
		case NODE_ASSIGNMENT :
		{
			if (node->children[0]->type != NODE_REF)
			{
				fprintf(stderr, "%d:%d error : Left operand of assignement must be a variable reference\n", node->line, node->col);
				exit(EXIT_FAILURE);
			}
			generate_code(node->children[1], stream);
			fprintf(stream, "dup\n");
			fprintf(stream, "set %d\n", node->children[0]->index);
			break;
		}
		case NODE_CONST: fprintf(stream, "push %d\n", node->value.int_val); break;
	}
}