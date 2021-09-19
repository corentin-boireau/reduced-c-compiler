#include "syntactic_analysis.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

// Syntactic rules
SyntacticNode* sr_grammar(SyntacticAnalyzer* analyzer);     // Whole program
SyntacticNode* sr_instruction(SyntacticAnalyzer* analyzer); // Instruction
SyntacticNode* sr_expression(SyntacticAnalyzer* analyzer);  // Expression
SyntacticNode* sr_prefix(SyntacticAnalyzer* analyzer);      // Prefix 
SyntacticNode* sr_suffix(SyntacticAnalyzer* analyzer);      // Suffix
SyntacticNode* sr_atom(SyntacticAnalyzer* analyzer);        // Atom

SyntacticNode* sr_expression_prio(SyntacticAnalyzer* analyzer, int priority); // Expression with priority


SyntacticNode* opti_constant_prefix(SyntacticNode* node);

#define	RIGHT_TO_LEFT   0
#define LEFT_TO_RIGHT   1
typedef struct OperatorInfo_s OperatorInfo;
struct OperatorInfo_s
{
	int priority;
	int associativity;
	int node_type;
};
#define NB_OPERATORS 14
#define MIN_PRIORITY 0
static  OperatorInfo OP_INFOS[NB_OPERATORS] = 
{
	{1, RIGHT_TO_LEFT, NODE_ASSIGNMENT},

	{2, LEFT_TO_RIGHT, NODE_OR},

	{3, LEFT_TO_RIGHT, NODE_AND},

	{4, LEFT_TO_RIGHT, NODE_EQUAL},
	{4, LEFT_TO_RIGHT, NODE_NOT_EQUAL},

	{5, LEFT_TO_RIGHT, NODE_GREATER},
	{5, LEFT_TO_RIGHT, NODE_GREATER_OR_EQUAL},
	{5, LEFT_TO_RIGHT, NODE_LESS},
	{5, LEFT_TO_RIGHT, NODE_LESS_OR_EQUAL},

	{6, LEFT_TO_RIGHT, NODE_ADD },
	{6, LEFT_TO_RIGHT, NODE_SUB },

	{7, LEFT_TO_RIGHT, NODE_MUL},
	{7, LEFT_TO_RIGHT, NODE_DIV},
	{7, LEFT_TO_RIGHT, NODE_MOD},
};
static inline int is_binary_op(int token_type)
{
	return token_type < NB_OPERATORS && token_type >= 0;
}

void syntactic_analyzer_inc_error(SyntacticAnalyzer* analyzer)
{
	analyzer->nb_errors++;

	if (analyzer->nb_errors > MAX_SYNTACTIC_ERROR)
	{
		syntactic_analyzer_report_and_exit(analyzer);
	}
}

SyntacticAnalyzer syntactic_analyzer_create(char* source_buffer, unsigned char optimisations)
{
	assert(source_buffer != NULL);

	SyntacticAnalyzer analyzer;
	
	analyzer.tokenizer      = tokenizer_create(source_buffer);
	analyzer.syntactic_tree = NULL;
	analyzer.nb_errors      = 0;
	analyzer.optimisations  = optimisations;

	return analyzer;
}

SyntacticNode* syntactic_analyzer_build_tree(SyntacticAnalyzer* analyzer)
{
	assert(analyzer != NULL);

	tokenizer_step(&(analyzer->tokenizer));
	if ( ! (analyzer->tokenizer.next.type == TOK_EOF))
	{
		analyzer->syntactic_tree = sr_grammar(analyzer);
	}

	return analyzer->syntactic_tree;
}


void syntactic_analyzer_report_and_exit(const SyntacticAnalyzer* analyzer)
{
	assert(analyzer != NULL);

	fprintf(stderr, "\nSyntactic analysis errors : %d\n", analyzer->nb_errors);
	exit(EXIT_FAILURE);
}


// Syntactic rules

SyntacticNode* sr_grammar(SyntacticAnalyzer* analyzer)
{
	assert(analyzer != NULL);

	// G ---> I
	return sr_instruction(analyzer);
}

SyntacticNode* sr_instruction(SyntacticAnalyzer* analyzer)
{
	assert(analyzer != NULL);

	SyntacticNode* node;

	// To handle empty instructions (ex : "int x = 1;;")
	// Might also handle empty blocks (ex : "{}")
	// while (tokenizer_check(&(analyzer->tokenizer), TOK_SEMICOLON));


	if (tokenizer_check(&(analyzer->tokenizer), TOK_OPEN_BRACE))
	{ // I ---> '{' I* '}'
		node = syntactic_node_create(NODE_BLOCK, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		while (!tokenizer_check(&(analyzer->tokenizer), TOK_CLOSE_BRACE))
		{
			syntactic_node_add_child(node, sr_instruction(analyzer));
		}
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_PRINT))
	{ // I ---> 'print' E ';'
		node = syntactic_node_create(NODE_PRINT, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		SyntacticNode* expr_printed = sr_expression(analyzer);
		syntactic_node_add_child(node, expr_printed);
		tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_INT))
	{ // I ---> 'int' ident ';'
		node = syntactic_node_create(NODE_SEQUENCE, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);

		tokenizer_accept(&(analyzer->tokenizer), TOK_IDENTIFIER);
		SyntacticNode* decl = syntactic_node_create(NODE_DECL, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		decl->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
		syntactic_node_add_child(node, decl);

		while (!tokenizer_check(&(analyzer->tokenizer), TOK_SEMICOLON))
		{
			tokenizer_accept(&(analyzer->tokenizer), TOK_COMMA);
			tokenizer_accept(&(analyzer->tokenizer), TOK_IDENTIFIER);

            decl = syntactic_node_create(NODE_DECL, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
            decl->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
            syntactic_node_add_child(node, decl);
		}
	}
	else
	{ // I ---> E ';'
		node = syntactic_node_create(NODE_DROP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		syntactic_node_add_child(node, sr_expression(analyzer));
		tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
	}

	return node;
}

SyntacticNode* sr_expression(SyntacticAnalyzer* analyzer)
{
	return sr_expression_prio(analyzer, MIN_PRIORITY);
}

SyntacticNode* sr_expression_prio(SyntacticAnalyzer* analyzer, int priority)
{
	assert(analyzer != NULL);
	
	// E ---> P '=' E
	// 		| P '||' E 
	//		| P '&&' E 
	// 		| P '==' | '!=' E 
	// 		| P '<' | '<=' | '>' | '>='  E 
	//		| P '+' | '-' E
	//		| P '*' | '/' E
	
	SyntacticNode* node = sr_prefix(analyzer);
	int end_expr = 0;
	while (!end_expr)
	{
		if (is_binary_op(analyzer->tokenizer.next.type))
		{
			OperatorInfo node_info = OP_INFOS[analyzer->tokenizer.next.type];
			if (node_info.priority < priority)
			{
				end_expr = 1;
			}
			else
			{
				tokenizer_step(&(analyzer->tokenizer));
				SyntacticNode* operand1 = node;
				SyntacticNode* operand2 = sr_expression_prio(analyzer, node_info.priority + node_info.associativity);
				
				// Optimisation of operations on constants
				if (is_opti_enabled(analyzer, OPTI_CONST_OPERATIONS)
					&& node_info.node_type != NODE_ASSIGNMENT 
			        && operand1->type == NODE_CONST && operand2->type == NODE_CONST)
				{
					int value = -1;
					switch (node_info.node_type)
					{
						case NODE_ADD:              value = operand1->value.int_val  +   operand2->value.int_val;  break;
						case NODE_SUB:              value = operand1->value.int_val  -   operand2->value.int_val;  break;
						case NODE_MUL:              value = operand1->value.int_val  *   operand2->value.int_val;  break;
						case NODE_AND:              value = operand1->value.int_val  &&  operand2->value.int_val;  break;
						case NODE_OR:               value = operand1->value.int_val  ||  operand2->value.int_val;  break;
						case NODE_EQUAL:            value = operand1->value.int_val  ==  operand2->value.int_val;  break;
						case NODE_NOT_EQUAL:        value = operand1->value.int_val  !=  operand2->value.int_val;  break;
						case NODE_LESS:             value = operand1->value.int_val  <   operand2->value.int_val;  break;
						case NODE_LESS_OR_EQUAL:    value = operand1->value.int_val  <=  operand2->value.int_val;  break;
						case NODE_GREATER:          value = operand1->value.int_val  >   operand2->value.int_val;  break;
						case NODE_GREATER_OR_EQUAL: value = operand1->value.int_val  >=  operand2->value.int_val;  break;

						case NODE_DIV:
						{
							if (operand2->value.int_val == 0)
							{
								node = syntactic_node_create(NODE_INVALID, operand2->line, operand2->col);
								fprintf(stderr, "%d:%d error : Division by zero\n", operand2->line, operand2->col);
								syntactic_analyzer_inc_error(analyzer);
							}
							else
							{
								value = operand1->value.int_val / operand2->value.int_val;
							}
							break;
						}
						case NODE_MOD:
						{
							if (operand2->value.int_val == 0)
							{
								node = syntactic_node_create(NODE_INVALID, operand2->line, operand2->col);
								fprintf(stderr, "%d:%d error : Modulo by zero\n", operand2->line, operand2->col);
								syntactic_analyzer_inc_error(analyzer);
							}
							else
							{
								value = operand1->value.int_val % operand2->value.int_val;
							}
							break;
						}
					}

					if(node->type != NODE_INVALID)
						node = syntactic_node_create_with_value(NODE_CONST, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col, value);
				}
				else
				{
					node = syntactic_node_create(node_info.node_type, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
					syntactic_node_add_child(node, operand1);
					syntactic_node_add_child(node, operand2);
				}
			}
		}
		else
			end_expr = 1;
	}

	return node;
}

SyntacticNode* sr_prefix(SyntacticAnalyzer* analyzer)
{
	assert(analyzer != NULL);

	SyntacticNode* node;

	if (tokenizer_check(&(analyzer->tokenizer), TOK_PLUS))
	{ // P ---> '+' P
		node = sr_prefix(analyzer);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_MINUS))
	{ // P ---> '-' P
		node = syntactic_node_create(NODE_UNARY_MINUS, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		SyntacticNode* next_prefix_node = sr_prefix(analyzer);
		syntactic_node_add_child(node, next_prefix_node);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_NOT))
	{ // P ---> '!' P
		node = syntactic_node_create(NODE_NEGATION, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		SyntacticNode* next_prefix_node = sr_prefix(analyzer);
		syntactic_node_add_child(node, next_prefix_node);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_STAR))
	{ // P ---> '*' P
		node = syntactic_node_create(NODE_INVALID, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		fprintf(stderr, "Unexpected token at %d:%d\n", analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		syntactic_analyzer_inc_error(analyzer);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_AMPERSAND))
	{ // P ---> '&' P
		node = syntactic_node_create(NODE_INVALID, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		fprintf(stderr, "Unexpected token at %d:%d\n", analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		syntactic_analyzer_inc_error(analyzer);
	}
	else
	{ // P ---> S
		node = sr_suffix(analyzer);
	}
	
	// *** Optimistations ***
	if(is_opti_enabled(analyzer, OPTI_CONST_OPERATIONS))
		node = opti_constant_prefix(node);

	// **********************
	return node;
}

SyntacticNode* sr_suffix(SyntacticAnalyzer* analyzer)
{
	assert(analyzer != NULL);

	// S ---> A
	return sr_atom(analyzer);
}

SyntacticNode* sr_atom(SyntacticAnalyzer* analyzer)
{
	assert(analyzer != NULL);

	SyntacticNode* node;

	if (tokenizer_check(&(analyzer->tokenizer), TOK_CONST))
	{ // A ---> const
		node = syntactic_node_create_with_value(NODE_CONST, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col, analyzer->tokenizer.current.value.int_val);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS))
	{ // A ---> '(' E ')'
		node = sr_expression(analyzer);
		tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_IDENTIFIER))
	{ // A ---> ident
		node = syntactic_node_create(NODE_REF, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		node->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
	}
	else
	{ // Unexpected token
		node = syntactic_node_create(NODE_INVALID, analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
		fprintf(stderr, "Unexpected token at %d:%d\n", analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
		syntactic_analyzer_inc_error(analyzer);
	}

	return node;
}

// Optimisations

SyntacticNode* opti_constant_prefix(SyntacticNode* node)
{
	SyntacticNode* optimized_node = node;

	switch (node->type)
	{
		case NODE_UNARY_MINUS:
		{
			assert(node->children[0] != NULL);
			SyntacticNode* constant = node->children[0];

			if (constant->type == NODE_CONST)
			{
				optimized_node = constant;
				optimized_node->value.int_val = - constant->value.int_val;

				syntactic_node_free(node);
			}
			break;
		}
		case NODE_NEGATION:
		{
			assert(node->children[0] != NULL);
			SyntacticNode* constant = node->children[0];
			
			if (constant->type == NODE_CONST)
			{
				optimized_node = constant;
				optimized_node->value.int_val = ! constant->value.int_val;

				syntactic_node_free(node);
			}
			break;
		}
	}
	
	return optimized_node;
}