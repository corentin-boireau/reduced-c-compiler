#ifndef SYNTACTIC_ANALYSIS_H
#define SYNTACTIC_ANALYSIS_H

#include "syntactic_node.h"
#include "token.h"

#define MAX_ERROR 0 // If there are more than MAX_ERROR, we stop the syntactic analysis

typedef struct
{
	Tokenizer      tokenizer;
	SyntacticNode* syntactic_tree;
	int            nb_errors;
} SyntacticAnalyzer;

SyntacticAnalyzer syntactic_analyzer_create(char* source_buffer);
SyntacticNode* syntactic_analyzer_build_tree(SyntacticAnalyzer* analyzer);
void syntactic_analyzer_report_and_exit(SyntacticAnalyzer* analyzer);


// Syntactic rules
SyntacticNode* syntactic_rule_grammar(SyntacticAnalyzer* analyzer);    // Whole program
SyntacticNode* syntactic_rule_prefix(SyntacticAnalyzer* analyzer);     // Prefix 
SyntacticNode* syntactic_rule_suffix(SyntacticAnalyzer* analyzer);     // Suffix
SyntacticNode* syntactic_rule_atom(SyntacticAnalyzer* analyzer);       // Atome
SyntacticNode* syntactic_rule_expression(SyntacticAnalyzer* analyzer); // Expression

#endif // SYNTACTIC_ANALYSIS_H