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
SyntacticNode* sr_atom(SyntacticAnalyzer* analyzer);        // Atome

SyntacticNode* sr_expression_prio(SyntacticAnalyzer* analyzer, int priority); // Expression with priority

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
inline int is_binary_op(int token_type)
{
	return token_type < NB_OPERATORS && token_type >= 0;
}

void syntactic_analyzer_inc_error(SyntacticAnalyzer* analyzer)
{
	analyzer->nb_errors++;

	if (analyzer->nb_errors > MAX_ERROR)
	{
		syntactic_analyzer_report_and_exit(analyzer);
	}
}

SyntacticAnalyzer syntactic_analyzer_create(char* source_buffer)
{
	assert(source_buffer != NULL);

	SyntacticAnalyzer analyzer;
	
	analyzer.tokenizer      = tokenizer_create(source_buffer);
	analyzer.syntactic_tree = NULL;
	analyzer.nb_errors      = 0;

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
		node = syntactic_node_create(NODE_BLOCK, analyzer->tokenizer.line, analyzer->tokenizer.col);
		while (!tokenizer_check(&(analyzer->tokenizer), TOK_CLOSE_BRACE))
		{
			syntactic_node_add_child(node, sr_instruction(analyzer));
		}
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_PRINT))
	{ // I ---> 'print' E ';'
		node = syntactic_node_create(NODE_PRINT, analyzer->tokenizer.line, analyzer->tokenizer.col);
		SyntacticNode* expr_printed = sr_expression(analyzer);
		syntactic_node_add_child(node, expr_printed);
		tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
	}
	else
	{ // I ---> E ';'
		node = syntactic_node_create(NODE_DROP, analyzer->tokenizer.line, analyzer->tokenizer.col);
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
				node = syntactic_node_create(node_info.node_type, analyzer->tokenizer.line, analyzer->tokenizer.line);
				syntactic_node_add_child(node, operand1);
				syntactic_node_add_child(node, operand2);
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
		node = syntactic_node_create(NODE_UNARY_MINUS, analyzer->tokenizer.line, analyzer->tokenizer.col);
		SyntacticNode* next_prefix_node = sr_prefix(analyzer);
		syntactic_node_add_child(node, next_prefix_node);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_NOT))
	{ // P ---> '!' P
		node = syntactic_node_create(NODE_NEGATION, analyzer->tokenizer.line, analyzer->tokenizer.col);
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
		node = syntactic_node_create_with_value(NODE_CONST, analyzer->tokenizer.line, analyzer->tokenizer.col, analyzer->tokenizer.current.value.int_val);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS))
	{ // A ---> '(' E ')'
		node = sr_expression(analyzer);
		tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
	}
	else
	{ // Unexpected token
		node = syntactic_node_create(NODE_INVALID, analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
		fprintf(stderr, "Unexpected token at %d:%d\n", analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
		syntactic_analyzer_inc_error(analyzer);
	}

	return node;
}

