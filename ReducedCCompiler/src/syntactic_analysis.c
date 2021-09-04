#include "syntactic_analysis.h"

#include <stdlib.h>
#include <stdio.h>


SyntacticAnalyzer syntactic_analyzer_create(char* source_buffer)
{
	SyntacticAnalyzer analyzer;
	
	analyzer.tokenizer      = tokenizer_create(source_buffer);
	analyzer.syntactic_tree = NULL;
	analyzer.nb_errors      = 0;

	return analyzer;
}

SyntacticNode* syntactic_analyzer_build_tree(SyntacticAnalyzer* analyzer)
{
	// TODO : init tokenizer and build the syntactic tree by using the rules
}



// Syntactic rules

SyntacticNode* syntactic_rule_grammar(Tokenizer* tokenizer)
{
	return syntactic_rule_expression(tokenizer);
}

SyntacticNode* syntactic_rule_prefix(Tokenizer* tokenizer)
{
	SyntacticNode* node = NULL;

	if (tokenizer_check(tokenizer, TOK_PLUS))
	{
		node = syntactic_rule_prefix(tokenizer);
	}
	else if (tokenizer_check(tokenizer, TOK_MINUS))
	{
		node = syntactic_node_create(NODE_UNARY_MINUS, tokenizer->line, tokenizer->col);
		SyntacticNode* next_prefix_node = syntactic_rule_prefix(tokenizer);
		syntactic_node_add_child(node, next_prefix_node);
	}
	else if (tokenizer_check(tokenizer, TOK_NOT))
	{
		node = syntactic_node_create(NODE_NEGATION, tokenizer->line, tokenizer->col);
		SyntacticNode* next_prefix_node = syntactic_rule_prefix(tokenizer);
		syntactic_node_add_child(node, next_prefix_node);
	}
	else if (tokenizer_check(tokenizer, TOK_STAR))
	{
		fprintf(stderr, "Unexpected token at %d:%d\n", tokenizer->current.line, tokenizer->current.col);
	}
	else if (tokenizer_check(tokenizer, TOK_AMPERSAND))
	{
		fprintf(stderr, "Unexpected token at %d:%d\n", tokenizer->current.line, tokenizer->current.col);
	}
	else
	{
		node = syntactic_rule_suffix(tokenizer);
	}

	return node;
}

SyntacticNode* syntactic_rule_suffix(Tokenizer* tokenizer)
{
	return syntactic_rule_atom(tokenizer);
}

SyntacticNode* syntactic_rule_atom(Tokenizer* tokenizer)
{
	SyntacticNode* node;

	if (tokenizer_check(tokenizer, TOK_CONST))
	{
		node = syntactic_node_create_with_value(NODE_CONST, tokenizer->line, tokenizer->col, tokenizer->current.value.int_val);
	}
	else if (tokenizer_check(tokenizer, TOK_OPEN_PARENTHESIS))
	{
		node = syntactic_rule_expression(tokenizer);
		tokenizer_accept(tokenizer, TOK_CLOSE_PARENTHESIS);
	}
	else
	{
		fprintf(stderr, "Unexpected token at %d:%d\n", tokenizer->next.line, tokenizer->next.col);
		exit(EXIT_FAILURE);
	}

	return node;
}

SyntacticNode* syntactic_rule_expression(Tokenizer* tokenizer)
{
	return syntactic_rule_prefix(tokenizer);
}