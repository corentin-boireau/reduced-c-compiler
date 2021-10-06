#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H 

#include "syntactic_node.h"

typedef struct Symbol_s Symbol;
struct Symbol_s
{
	char* name;
	int   stack_offset;
	int   type;
	int   nb_params;
};

enum
{
	SYMBOL_VAR,
	SYMBOL_FUNC,
	SYMBOL_PTR,
};

#define MAX_SYMBOLS 500
#define MAX_SCOPES  20
#define MAX_SEMANTIC_ERROR 3 // If there are more than MAX_SEMANTIC_ERROR, we stop the semantic analysis

typedef struct SymbolTable_s SymbolTable;
struct SymbolTable_s
{
	Symbol symbols[MAX_SYMBOLS];
	int    scopes[MAX_SCOPES];
	int    nb_symbols;
	int    nb_variables;
	int    current_scope;
	int    nb_errors;
};

SymbolTable symbol_table_create();
void semantic_analysis(SyntacticNode* tree, SymbolTable* table);
void semantic_analysis_report_and_exit(const SymbolTable* table);

#endif // SEMANTIC_ANALYSIS_H