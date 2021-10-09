#include "code_generation.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void generate_program(const SyntacticNode* program, FILE * stream)
{
	assert(program != NULL);

	char asm_runtime[] =
		".start"             "\n"
		"		prep main"	 "\n"
		"		call 0"		 "\n"
		"		halt"		 "\n"
							 "\n"
		".putchar"			 "\n"
		"		send"		 "\n"
		"		push 0"		 "\n"
		"		ret"		 "\n"
							 "\n"
		".getchar"			 "\n"
		"		recv"		 "\n"
		"		ret"		 "\n"
		;

	generate_code(program, stream, NO_LOOP);

	fprintf(stream, "%s\n", asm_runtime);
}

void generate_code(const SyntacticNode* node, FILE * stream, int loop_nb)
{
	assert(node != NULL);

	static int label_counter = 0;

	switch (node->type)
	{
		case NODE_NEGATION:
		{
			generate_code(node->children[0], stream, loop_nb);
			fprintf(stream, "\t\tnot\n");
			break;
		}
		case NODE_UNARY_MINUS:
		{
			fprintf(stream, "\t\tpush 0\n");
			generate_code(node->children[0], stream, loop_nb);
			fprintf(stream, "\t\tsub\n");
			break;
		}
		case NODE_ADD :
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tadd\n");
			break;
		}
		case NODE_SUB:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tsub\n");
			break;
		}
		case NODE_MUL:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tmul\n");
			break;
		}
		case NODE_DIV:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tdiv\n");
			break;
		}
		case NODE_MOD:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tmod\n");
			break;
		}
		case NODE_AND:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tand\n");
			break;
		}
		case NODE_OR:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tor\n");
			break;
		}
		case NODE_EQUAL:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tcmpeq\n");
			break;
		}
		case NODE_NOT_EQUAL:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tcmpne\n");
			break;
		}
		case NODE_LESS:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tcmplt\n");
			break;
		}
		case NODE_LESS_OR_EQUAL:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tcmple\n");
			break;
		}
		case NODE_GREATER:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tcmpgt\n");
			break;
		}
		case NODE_GREATER_OR_EQUAL:
		{
			generate_code(node->children[0], stream, loop_nb);
			generate_code(node->children[1], stream, loop_nb);
			fprintf(stream, "\t\tcmpge\n");
			break;
		}
		case NODE_PRINT:
		{
			generate_code(node->children[0], stream, loop_nb);
			fprintf(stream, "\t\tdbg\n");
			break;
		}
		case NODE_PROGRAM:
		case NODE_SEQUENCE:
		case NODE_BLOCK:
		{
			for (int i = 0; i < node->nb_children; i++)
			{
				generate_code(node->children[i], stream, loop_nb);
			}
			break;
		}
		case NODE_DROP:
		{
			generate_code(node->children[0], stream, loop_nb);
			fprintf(stream, "\t\tdrop\n");
			break;
		}
		case NODE_DECL :
			break; 
		case NODE_REF:
		{
			fprintf(stream, "\t\tget %d\n", node->stack_offset);
			break;
		}
		case NODE_ASSIGNMENT :
		{
			assert(node->nb_children == 2);

			SyntacticNode* assignable = node->children[0];
			if (assignable->type == NODE_REF)
			{
				generate_code(node->children[1], stream, loop_nb);
				fprintf(stream, "\t\tdup\n");
				fprintf(stream, "\t\tset %d\n", assignable->stack_offset);
			}
			else if (assignable->type == NODE_DEREF)
			{
				assert(assignable->nb_children == 1);

				generate_code(node->children[1], stream, loop_nb);
				fprintf(stream, "\t\tdup\n");
				generate_code(assignable->children[0], stream, loop_nb);
				fprintf(stream, "\t\twrite\n");
			}
			else
			{
				fprintf(stderr, "%d:%d error : Left operand of assignement must be a variable reference\n", node->line, node->col);
				exit(EXIT_FAILURE);
			}
			break;
		}
		case NODE_CONDITION:
		{
			int has_else = (node->nb_children == 3);
			int label_number = label_counter++;
			generate_code(node->children[0], stream, loop_nb);
			fprintf(stream, has_else ? "\t\tjumpf else_%d\n"
									 : "\t\tjumpf endif_%d\n", label_number);
			generate_code(node->children[1], stream, loop_nb);
			if (has_else)
			{
				fprintf(stream, "\t\tjump endif_%d\n", label_number);
				fprintf(stream, ".else_%d\n", label_number);
				generate_code(node->children[2], stream, loop_nb);
			}
			fprintf(stream, ".endif_%d\n", label_number);
			break;
		}
		case NODE_INVERTED_CONDITION:
		{
			int has_else = (node->nb_children == 3);
			int label_number = label_counter++;
			generate_code(node->children[0], stream, loop_nb);
			fprintf(stream, has_else ? "\t\tjumpt else_%d\n"
									 : "\t\tjumpt endif_%d\n", label_number);
			generate_code(node->children[1], stream, loop_nb);
			if (has_else)
			{
				fprintf(stream, "\t\tjump endif_%d\n", label_number);
				fprintf(stream, ".else_%d\n", label_number);
				generate_code(node->children[2], stream, loop_nb);
			}
			fprintf(stream, ".endif_%d\n", label_number);
			break;
		}
		case NODE_LOOP:
		{
			int current_loop_number = label_counter++;
			fprintf(stream, ".loop_%d\n", current_loop_number);
			for (int i = 0; i < node->nb_children; i++)
			{
				generate_code(node->children[i], stream, current_loop_number);
			}
			fprintf(stream, "\t\tjump loop_%d\n", current_loop_number);
			fprintf(stream, ".endloop_%d\n", current_loop_number);
			break;
		}
		case NODE_FUNCTION:
		{
			fprintf(stream, ".%s\n", node->value.str_val);
			fprintf(stream, "\t\tresn %d\n", node->nb_var);
			for (int i = 0; i < node->nb_children; i++)
			{
				generate_code(node->children[i], stream, loop_nb);
			}
			fprintf(stream, "\t\tpush 0\n");
			fprintf(stream, "\t\tret\n");
			break;
		}
		case NODE_CALL:
		{
			assert(node->nb_children == 1 && node->children[0]->type == NODE_SEQUENCE);

			fprintf(stream, "\t\tprep %s\n", node->value.str_val);
			for (int i = 0; i < node->nb_children; i++)
			{
				generate_code(node->children[i], stream, loop_nb);
			}
			fprintf(stream, "\t\tcall %d\n", node->children[0]->nb_children);
			break;
		}
		case NODE_RETURN:
		{
			int has_retval = (node->nb_children > 0);
			if (has_retval)
			{
				generate_code(node->children[0], stream, loop_nb);
			}
			else
			{
				fprintf(stream, "\t\tpush 0\n");
			}
			fprintf(stream, "\t\tret\n");

			break;
		}
		case NODE_CONTINUE:
		{
			fprintf(stream, "\t\tjump loop_%d\n", loop_nb);
			break;
		}
		case NODE_BREAK:
		{
			fprintf(stream, "\t\tjump endloop_%d\n", loop_nb);
			break;
		}
		case NODE_DEREF:
		{
			assert(node->nb_children == 1);
			generate_code(node->children[0], stream, loop_nb);
			fprintf(stream, "\t\tread\n");
			break;
		}
		case NODE_CONST: fprintf(stream, "\t\tpush %d\n", node->value.int_val); break;
	}
}
