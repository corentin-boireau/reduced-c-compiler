#include "semantic_analysis.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void startScope(SymbolTable* table);
void endScope(SymbolTable* table);
// Declares a new symbol with the given name and increments the nb_variables counter.
int declare(SymbolTable* table, char* name); 
int search(SymbolTable* table, char* name);


SymbolTable symbol_table_create()
{
	SymbolTable table;
	table.scopes[0]     = 0;
	table.nb_symbols    = 0;
	table.nb_variables  = 0;
	table.current_scope = 0;
	table.nb_errors     = 0;

	return table;
}

Symbol symbol_create(int index, char* name)
{
	Symbol symbol;
	symbol.index = index;
	symbol.name  = name;

	return symbol;
}

void symbol_table_inc_error(SymbolTable* table)
{
	table->nb_errors++;

	if (table->nb_errors > MAX_SEMANTIC_ERROR)
	{
		semantic_analysis_report_and_exit(table);
	}
}

void semantic_analysis_report_and_exit(const SymbolTable* table)
{
	assert(table != NULL);

	fprintf(stderr, "\nSemantic analysis errors : %d\n", table->nb_errors);
	exit(EXIT_FAILURE);
}

void semantic_analysis(SyntacticNode* node, SymbolTable* table)
{
	assert(node != NULL);

	switch(node->type)
	{
		case NODE_DECL :
		{
			// node->value.str_val is shared with symbol created by declare()
			int index = declare(table, node->value.str_val);
			if (index == INVALID_INDEX)
			{
                fprintf(stderr, "Redeclaration of symbol %s at %d:%d\n", node->value.str_val, node->line, node->col);
				symbol_table_inc_error(table);
			}
			else
			{
				node->index = index;
			}
			break;
		}
		case NODE_REF :
		{
			int index = search(table, node->value.str_val);
			if (index == INVALID_INDEX)
			{
                fprintf(stderr, "Reference to undeclared symbol %s at %d:%d\n", node->value.str_val, node->line, node->col);
				symbol_table_inc_error(table);
			}
			else
			{
				node->index = index;
			}
			break;
		}
		case NODE_BLOCK :
		{
			startScope(table);
			for (int i = 0; i < node->nb_children; i++)
			{
				semantic_analysis(node->children[i], table);
			}
			endScope(table);
			break;
		}
		default :
		{
			for (int i = 0; i < node->nb_children; i++)
			{
				semantic_analysis(node->children[i], table);
			}
			break;
		}
	}
}

void startScope(SymbolTable* table)
{
	table->current_scope++;
	table->scopes[table->current_scope] = table->nb_symbols;
}

void endScope(SymbolTable* table)
{
	table->nb_symbols = table->scopes[table->current_scope];
	table->current_scope--;
}

int declare(SymbolTable* table, char* name)
{
	int index = table->scopes[table->current_scope];
	int found = 0;
	while (index < table->nb_symbols && !found)
	{
		found = strcmp(name, table->symbols[index].name) == 0;
		index++;
	}
	if (!found)
	{
		table->symbols[table->nb_symbols] = symbol_create(table->nb_symbols, name);
		table->nb_symbols++;
		table->nb_variables++;
	}
	else
	{
		index = INVALID_INDEX;
	}

	return index;
}

int search(SymbolTable* table, char* name)
{
	int index;
	int i = table->nb_symbols - 1;
	int found = 0;
	while (i >= 0 && !found)
	{
		found = strcmp(name, table->symbols[i].name) == 0;
		i--;
	}
	if (found)
	{
		index = i + 1;
	}
	else
	{
		index = INVALID_INDEX;
	}

	return index;
}
