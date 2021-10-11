#include "syntactic_analysis.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Syntactic rules
SyntacticNode* sr_grammar(SyntacticAnalyzer* analyzer);		// Whole program
SyntacticNode* sr_function(SyntacticAnalyzer* analyzer);	// Function
SyntacticNode* sr_instruction(SyntacticAnalyzer* analyzer); // Instruction
SyntacticNode* sr_expression(SyntacticAnalyzer* analyzer);	// Expression
SyntacticNode* sr_prefix(SyntacticAnalyzer* analyzer);		// Prefix 
SyntacticNode* sr_suffix(SyntacticAnalyzer* analyzer);		// Suffix
SyntacticNode* sr_atom(SyntacticAnalyzer* analyzer);		// Atom

SyntacticNode* sr_expression_prio(SyntacticAnalyzer* analyzer, int priority); // Expression with priority


SyntacticNode* opti_constant_prefix(SyntacticNode* node);

#define	RIGHT_TO_LEFT	0
#define LEFT_TO_RIGHT	1
typedef struct OperatorInfo_s OperatorInfo;
struct OperatorInfo_s
{
	int priority;
	int associativity;
	int node_type;
};
#define NB_OPERATORS 14
#define MIN_PRIORITY 0
OperatorInfo get_operator_info(int token_type)
{
	OperatorInfo op_info = { -1, -1, -1 };
	switch (token_type)
	{
		case TOK_EQUAL:
		{
			op_info.priority	  = 1;
			op_info.associativity = RIGHT_TO_LEFT;
			op_info.node_type	  = NODE_ASSIGNMENT;
			break;
		}
		case TOK_2_PIPE:
		{
			op_info.priority	  = 2;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_OR;
			break;
		}
		case TOK_2_AMPERSAND:
		{
			op_info.priority	  = 3;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_AND;
			break;
		}
		case TOK_2_EQUAL:
		{
			op_info.priority	  = 4;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_EQUAL;
			break;
		}
		case TOK_NOT_EQUAL:
		{
			op_info.priority	  = 4;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_NOT_EQUAL;
			break;
		}
		case TOK_GREATER:
		{
			op_info.priority	  = 5;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_GREATER;
			break;
		}
		case TOK_GREATER_OR_EQUAL:
		{
			op_info.priority	  = 5;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_GREATER_OR_EQUAL;
			break;
		}
		case TOK_LESS:
		{
			op_info.priority	  = 5;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_LESS;
			break;
		}
		case TOK_LESS_OR_EQUAL:
		{
			op_info.priority	  = 5;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_LESS_OR_EQUAL;
			break;
		}
		case TOK_PLUS:
		{
			op_info.priority	  = 6;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_ADD;
			break;
		}
		case TOK_MINUS:
		{
			op_info.priority	  = 6;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_SUB;
			break;
		}
		case TOK_STAR:
		{
			op_info.priority	  = 7;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_MUL;
			break;
		}
		case TOK_SLASH:
		{
			op_info.priority	  = 7;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_DIV;
			break;
		}
		case TOK_PERCENT:
		{
			op_info.priority	  = 7;
			op_info.associativity = LEFT_TO_RIGHT;
			op_info.node_type	  = NODE_MOD;
			break;
		}
	}

	assert(op_info.priority != -1);
	assert(op_info.associativity != -1);
	assert(op_info.node_type != -1);
	return op_info;
}

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
	
	analyzer.tokenizer		= tokenizer_create(source_buffer);
	analyzer.syntactic_tree = NULL;
	analyzer.nb_errors		= 0;
	analyzer.optimisations	= optimisations;

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
	SyntacticNode* program = syntactic_node_create(NODE_PROGRAM, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
	while (!tokenizer_check(&(analyzer->tokenizer), TOK_EOF))
	{
		SyntacticNode* function = sr_function(analyzer);
		syntactic_node_add_child(program, function);
	}
	return program;
}

SyntacticNode* sr_function(SyntacticAnalyzer* analyzer)
{
	assert(analyzer != NULL);

	SyntacticNode* node;

	if (tokenizer_check(&(analyzer->tokenizer), TOK_INT))
	{
		node = syntactic_node_create(NODE_FUNCTION, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		tokenizer_accept(&(analyzer->tokenizer), TOK_IDENTIFIER);
		node->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
		tokenizer_accept(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS);
		SyntacticNode *seq = syntactic_node_create(NODE_SEQUENCE, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		// args
		if (!tokenizer_check(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS))
		{
			do
			{
				tokenizer_accept(&(analyzer->tokenizer), TOK_INT);
				tokenizer_accept(&(analyzer->tokenizer), TOK_IDENTIFIER);
				SyntacticNode* decl = syntactic_node_create(NODE_DECL, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
				decl->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
				syntactic_node_add_child(seq, decl);
			} while (tokenizer_check(&(analyzer->tokenizer), TOK_COMMA));
			tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
		}
		syntactic_node_add_child(node, seq);
		//tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
		SyntacticNode* instruction = sr_instruction(analyzer);
		syntactic_node_add_child(node, instruction);
	}
	else
	{ // Unexpected token
		node = syntactic_node_create(NODE_INVALID, analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
		fprintf(stderr, "Unexpected token at %d:%d\n", analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
		syntactic_analyzer_inc_error(analyzer);
	}

	return node;
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
	{ // I ---> 'int' ident ('=' E)? (',' ident ('=' E)? )* ';'
		node = syntactic_node_create(NODE_SEQUENCE, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);

		tokenizer_accept(&(analyzer->tokenizer), TOK_IDENTIFIER);
		SyntacticNode* decl = syntactic_node_create(NODE_DECL, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		decl->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
		syntactic_node_add_child(node, decl);

		if (tokenizer_check(&(analyzer->tokenizer), TOK_EQUAL))
		{
			SyntacticNode* ref = syntactic_node_create(NODE_REF, decl->line, decl->col);
			size_t nb_char = strlen(decl->value.str_val);
			ref->value.str_val = malloc((nb_char + 1) * sizeof(char));
			if (ref->value.str_val == NULL)
			{
				perror("Failed to allocate space for reference's name");
				exit(EXIT_FAILURE);
			}
			memcpy(ref->value.str_val, decl->value.str_val, (nb_char + 1) * sizeof(char));

			SyntacticNode* assignment = syntactic_node_create(NODE_ASSIGNMENT, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
			SyntacticNode* expr = sr_expression(analyzer);

			SyntacticNode* drop = syntactic_node_create(NODE_DROP, assignment->line, assignment->col);

			syntactic_node_add_child(assignment, ref);
			syntactic_node_add_child(assignment, expr);
			syntactic_node_add_child(drop, assignment);
			syntactic_node_add_child(node, drop);
		}

		while (!tokenizer_check(&(analyzer->tokenizer), TOK_SEMICOLON))
		{
			tokenizer_accept(&(analyzer->tokenizer), TOK_COMMA);
			tokenizer_accept(&(analyzer->tokenizer), TOK_IDENTIFIER);

			decl = syntactic_node_create(NODE_DECL, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
			decl->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
			syntactic_node_add_child(node, decl);

			if (tokenizer_check(&(analyzer->tokenizer), TOK_EQUAL))
			{
				SyntacticNode* ref = syntactic_node_create(NODE_REF, decl->line, decl->col);
				size_t nb_char = strlen(decl->value.str_val);
				ref->value.str_val = malloc((nb_char + 1) * sizeof(char));
				if (ref->value.str_val == NULL)
				{
					perror("Failed to allocate space for reference's name");
					exit(EXIT_FAILURE);
				}
				memcpy(ref->value.str_val, decl->value.str_val, (nb_char + 1) * sizeof(char));

				SyntacticNode* assignment = syntactic_node_create(NODE_ASSIGNMENT, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
				SyntacticNode* expr = sr_expression(analyzer);

				SyntacticNode* drop = syntactic_node_create(NODE_DROP, assignment->line, assignment->col);

				syntactic_node_add_child(assignment, ref);
				syntactic_node_add_child(assignment, expr);
				syntactic_node_add_child(drop, assignment);
				syntactic_node_add_child(node, drop);
			}
		}
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_IF))
	{ // I ---> 'if' '(' E ')' I ?('else' I)
		node = syntactic_node_create(NODE_CONDITION, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		tokenizer_accept(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS);
		SyntacticNode *expr = sr_expression(analyzer);
		syntactic_node_add_child(node, expr);
		tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
		SyntacticNode* instruction1 = sr_instruction(analyzer);
		syntactic_node_add_child(node, instruction1);
		if (tokenizer_check(&(analyzer->tokenizer), TOK_ELSE))
		{
			SyntacticNode* instruction2 = sr_instruction(analyzer);
			syntactic_node_add_child(node, instruction2);
		}
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_WHILE))
	{ // I ---> 'while' '(' E ')' I
		node = syntactic_node_create(NODE_LOOP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		tokenizer_accept(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS);
		SyntacticNode *cond = syntactic_node_create(NODE_CONDITION, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		SyntacticNode *expr = sr_expression(analyzer);
		tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
		SyntacticNode* instruction = sr_instruction(analyzer);
		SyntacticNode *node_break = syntactic_node_create(NODE_BREAK, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);

		syntactic_node_add_child(cond, expr);
		syntactic_node_add_child(cond, instruction);
		syntactic_node_add_child(cond, node_break);

		syntactic_node_add_child(node, cond);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_DO))
	{ // I ---> 'do' I 'while' '(' E ')' ';'
		node = syntactic_node_create(NODE_LOOP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		SyntacticNode* instruction = sr_instruction(analyzer);
		tokenizer_accept(&(analyzer->tokenizer), TOK_WHILE);
		tokenizer_accept(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS);
		SyntacticNode *inv_cond = syntactic_node_create(NODE_INVERTED_CONDITION, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		SyntacticNode *expr = sr_expression(analyzer);
		tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
		tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
		SyntacticNode *node_break = syntactic_node_create(NODE_BREAK, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);

		syntactic_node_add_child(inv_cond, expr);
		syntactic_node_add_child(inv_cond, node_break);

		syntactic_node_add_child(node, instruction);
		syntactic_node_add_child(node, inv_cond);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_FOR))
	{ // I ---> 'for' '(' E1 ';' E2 ';' E3 ')' I
		node = syntactic_node_create(NODE_SEQUENCE, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		SyntacticNode *loop = syntactic_node_create(NODE_LOOP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		tokenizer_accept(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS);
		SyntacticNode *expr1 = sr_expression(analyzer);
		SyntacticNode *drop1 = syntactic_node_create(NODE_DROP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
		SyntacticNode *inv_cond = syntactic_node_create(NODE_INVERTED_CONDITION, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		SyntacticNode *expr2 = sr_expression(analyzer);
		tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
		SyntacticNode *expr3 = sr_expression(analyzer);
		SyntacticNode *drop3 = syntactic_node_create(NODE_DROP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
		SyntacticNode* instruction = sr_instruction(analyzer);
		SyntacticNode *node_break = syntactic_node_create(NODE_BREAK, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);

		syntactic_node_add_child(drop1, expr1);

		syntactic_node_add_child(inv_cond, expr2);
		syntactic_node_add_child(inv_cond, node_break);

		syntactic_node_add_child(drop3, expr3);

		syntactic_node_add_child(loop, inv_cond);
		syntactic_node_add_child(loop, instruction);
		syntactic_node_add_child(loop, drop3);
		
		syntactic_node_add_child(node, drop1);
		syntactic_node_add_child(node, loop);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_CONTINUE))
	{ // I ---> 'continue' ';'
		node = syntactic_node_create(NODE_CONTINUE, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_BREAK))
	{ // I ---> 'break' ';'
		node = syntactic_node_create(NODE_BREAK, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_RETURN))
	{ // I ---> 'return' E ';'
		node = syntactic_node_create(NODE_RETURN, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		if (!tokenizer_check(&(analyzer->tokenizer), TOK_SEMICOLON))
		{
			SyntacticNode* expr = sr_expression(analyzer);
			syntactic_node_add_child(node, expr);
			tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
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
	//		| P '||' E 
	//		| P '&&' E 
	//		| P '==' | '!=' E 
	//		| P '<' | '<=' | '>' | '>='  E 
	//		| P '+' | '-' E
	//		| P '*' | '/' E
	
	SyntacticNode* node = sr_prefix(analyzer);
	int end_expr = 0;
	while (!end_expr)
	{
		if (is_binary_op(analyzer->tokenizer.next.type))
		{
			OperatorInfo node_info = get_operator_info(analyzer->tokenizer.next.type);
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
						case NODE_ADD:				value = operand1->value.int_val  +	 operand2->value.int_val;  break;
						case NODE_SUB:				value = operand1->value.int_val  -	 operand2->value.int_val;  break;
						case NODE_MUL:				value = operand1->value.int_val  *	 operand2->value.int_val;  break;
						case NODE_AND:				value = operand1->value.int_val  &&  operand2->value.int_val;  break;
						case NODE_OR:				value = operand1->value.int_val  ||  operand2->value.int_val;  break;
						case NODE_EQUAL:			value = operand1->value.int_val  ==  operand2->value.int_val;  break;
						case NODE_NOT_EQUAL:		value = operand1->value.int_val  !=  operand2->value.int_val;  break;
						case NODE_LESS:				value = operand1->value.int_val  <	 operand2->value.int_val;  break;
						case NODE_LESS_OR_EQUAL:	value = operand1->value.int_val  <=  operand2->value.int_val;  break;
						case NODE_GREATER:			value = operand1->value.int_val  >	 operand2->value.int_val;  break;
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
		node = syntactic_node_create(NODE_DEREF, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		SyntacticNode* prefix = sr_prefix(analyzer);
		syntactic_node_add_child(node, prefix);
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

	// S ---> A ( '[' E ']' )*
	SyntacticNode* node = sr_atom(analyzer);
	while (tokenizer_check(&(analyzer->tokenizer), TOK_OPEN_BRACKET))
	{
		SyntacticNode* deref = syntactic_node_create(NODE_DEREF, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		SyntacticNode* index_expr = sr_expression(analyzer);
		tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_BRACKET);

		SyntacticNode* indexation_addr = syntactic_node_create(NODE_ADD, index_expr->line, index_expr->col);
		syntactic_node_add_child(indexation_addr, node);
		syntactic_node_add_child(indexation_addr, index_expr);
		syntactic_node_add_child(deref, indexation_addr);
		node = deref;
	}

	return node ;
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
		int line = analyzer->tokenizer.current.line;
		int col  = analyzer->tokenizer.current.col;
		// var;
		if (!tokenizer_check(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS))
		{
			node = syntactic_node_create(NODE_REF, line, col);
			node->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
		}
		// function(arg1, ...)
		else
		{
			node = syntactic_node_create(NODE_CALL, line, col);
			node->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
			SyntacticNode *seq = syntactic_node_create(NODE_SEQUENCE, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
			// args
			if (!tokenizer_check(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS))
			{
				do
				{
					SyntacticNode* arg = sr_expression(analyzer);
					syntactic_node_add_child(seq, arg);
				} while (tokenizer_check(&(analyzer->tokenizer), TOK_COMMA));
				tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
			}
			syntactic_node_add_child(node, seq);
		}
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
