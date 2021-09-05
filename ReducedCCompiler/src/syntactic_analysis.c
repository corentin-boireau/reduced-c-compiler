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
	tokenizer_step(&(analyzer->tokenizer));
	if ( ! (analyzer->tokenizer.next.type == TOK_EOF))
	{
		analyzer->syntactic_tree = syntactic_rule_grammar(analyzer);
	}

	return analyzer->syntactic_tree;
}


void syntactic_analyzer_report_and_exit(SyntacticAnalyzer* analyzer)
{
	fprintf(stderr, "\nSyntactic analysis errors : %d\n", analyzer->nb_errors);
	exit(EXIT_FAILURE);
}


// Syntactic rules

SyntacticNode* syntactic_rule_grammar(SyntacticAnalyzer* analyzer)
{
	return syntactic_rule_expression(analyzer);
}

SyntacticNode* syntactic_rule_prefix(SyntacticAnalyzer* analyzer)
{
	SyntacticNode* node;

	if (tokenizer_check(&(analyzer->tokenizer), TOK_PLUS))
	{
		node = syntactic_rule_prefix(analyzer);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_MINUS))
	{
		node = syntactic_node_create(NODE_UNARY_MINUS, analyzer->tokenizer.line, analyzer->tokenizer.col);
		SyntacticNode* next_prefix_node = syntactic_rule_prefix(analyzer);
		syntactic_node_add_child(node, next_prefix_node);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_NOT))
	{
		node = syntactic_node_create(NODE_NEGATION, analyzer->tokenizer.line, analyzer->tokenizer.col);
		SyntacticNode* next_prefix_node = syntactic_rule_prefix(analyzer);
		syntactic_node_add_child(node, next_prefix_node);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_STAR))
	{
		fprintf(stderr, "Unexpected token at %d:%d\n", analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		analyzer->nb_errors++;
		node = syntactic_node_create(NODE_INVALID, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);

		if (analyzer->nb_errors > MAX_ERROR)
		{
			syntactic_analyzer_report_and_exit(analyzer);
		}
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_AMPERSAND))
	{
		fprintf(stderr, "Unexpected token at %d:%d\n", analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
		analyzer->nb_errors++;
		node = syntactic_node_create(NODE_INVALID, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);

		if (analyzer->nb_errors > MAX_ERROR)
		{
			syntactic_analyzer_report_and_exit(analyzer);
		}
	}
	else
	{
		node = syntactic_rule_suffix(analyzer);
	}

	return node;
}

SyntacticNode* syntactic_rule_suffix(SyntacticAnalyzer* analyzer)
{
	return syntactic_rule_atom(analyzer);
}

SyntacticNode* syntactic_rule_atom(SyntacticAnalyzer* analyzer)
{
	SyntacticNode* node;

	if (tokenizer_check(&(analyzer->tokenizer), TOK_CONST))
	{
		node = syntactic_node_create_with_value(NODE_CONST, analyzer->tokenizer.line, analyzer->tokenizer.col, analyzer->tokenizer.current.value.int_val);
	}
	else if (tokenizer_check(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS))
	{
		node = syntactic_rule_expression(analyzer);
		tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
	}
	else
	{
		fprintf(stderr, "Unexpected token at %d:%d\n", analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
		analyzer->nb_errors++;
		node = syntactic_node_create(NODE_INVALID, analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
		if (analyzer->nb_errors > MAX_ERROR)
		{
			syntactic_analyzer_report_and_exit(analyzer);
		}
	}

	return node;
}

SyntacticNode* syntactic_rule_expression(SyntacticAnalyzer* analyzer)
{
	return syntactic_rule_prefix(analyzer);
}