#ifndef SYNTACTIC_ANALYSIS_H
#define SYNTACTIC_ANALYSIS_H

#include "syntactic_node.h"
#include "token.h"

#define MAX_SYNTACTIC_ERROR 3 // If there are more than MAX_SYNTACTIC_ERROR, we stop the syntactic analysis

typedef struct SyntacticAnalyzer_s SyntacticAnalyzer;
struct SyntacticAnalyzer_s
{
	Tokenizer      tokenizer;
	SyntacticNode* syntactic_tree;
	int            nb_errors;
};

SyntacticAnalyzer syntactic_analyzer_create(char* source_buffer);
SyntacticNode* syntactic_analyzer_build_tree(SyntacticAnalyzer* analyzer);
void syntactic_analyzer_report_and_exit(const SyntacticAnalyzer* analyzer);

#endif // SYNTACTIC_ANALYSIS_H