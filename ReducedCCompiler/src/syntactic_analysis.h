#ifndef SYNTACTIC_ANALYSIS_H
#define SYNTACTIC_ANALYSIS_H

#include "syntactic_node.h"
#include "token.h"

typedef struct
{
	Tokenizer      tokenizer;
	SyntacticNode* syntactic_tree;
	int            nb_errors;
} SyntacticAnalyzer;

SyntacticAnalyzer syntactic_analyzer_create(char* source_buffer);
SyntacticNode* syntactic_analyzer_build_tree(SyntacticAnalyzer* analyzer);


// Syntactic rules
SyntacticNode* syntactic_rule_grammar(Tokenizer* tokenizer);    // Whole program
SyntacticNode* syntactic_rule_prefix(Tokenizer* tokenizer);     // Prefix 
SyntacticNode* syntactic_rule_suffix(Tokenizer* tokenizer);     // Suffix
SyntacticNode* syntactic_rule_atom(Tokenizer* tokenizer);       // Atome
SyntacticNode* syntactic_rule_expression(Tokenizer* tokenizer); // Expression

#endif // SYNTACTIC_ANALYSIS_H