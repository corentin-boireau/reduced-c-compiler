#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H 

#include "syntactic_node.h"

typedef struct Symbol_s Symbol;
struct Symbol_s
{
	char* name;
	int   index;
};

#define MAX_SYMBOLS 500
#define MAX_SCOPES  20

typedef struct SymbolTable_s SymbolTable;
struct SymbolTable_s
{
	Symbol symbols[MAX_SYMBOLS];
	int    scopes[MAX_SCOPES];
	int    nb_variables;
};


void semantic_analysis(SyntacticNode* tree, SymbolTable* table);

#endif // SEMANTIC_ANALYSIS_H