#include "semantic_analysis.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void startScope(SymbolTable* table);
void endScope(SymbolTable* table);
// Declares a new symbol with the given name and increments the nb_variables counter.
int declare(SymbolTable* table, char* name, int type, int* stack_offset); 
int search(SymbolTable* table, char* name, int* stack_offset);
Symbol symbol_create(int stack_offset, char* name, int type);


SymbolTable symbol_table_create()
{
	SymbolTable table;
	table.scopes[0]		= 0;
	table.nb_symbols	= 0;
	table.nb_variables	= 0;
	table.current_scope = 0;
	table.nb_errors		= 0;

	table.symbols[table.nb_symbols] = symbol_create(NO_STACK_OFFSET, "putchar", SYMBOL_FUNC);
	table.symbols[table.nb_symbols++].nb_params = 1;

	table.symbols[table.nb_symbols] = symbol_create(NO_STACK_OFFSET, "getchar", SYMBOL_FUNC);
	table.symbols[table.nb_symbols++].nb_params = 0;

	return table;
}

Symbol symbol_create(int stack_offset, char* name, int type)
{
	Symbol symbol;
	symbol.stack_offset = stack_offset;
	symbol.name  = name;
	symbol.type  = type;

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
			int stack_offset;
			// declare() will return -1 if it failed
			if (declare(table, node->value.str_val, SYMBOL_VAR, &stack_offset) < 0)
			{
				fprintf(stderr, "Redeclaration of symbol \"%s\" at %d:%d\n", node->value.str_val, node->line, node->col);
				symbol_table_inc_error(table);
			}
			else
			{
				node->stack_offset = stack_offset;
			}
			break;
		}
		case NODE_REF :
		{
			int stack_offset;
			int symbol_index = search(table, node->value.str_val, &stack_offset);
			if (symbol_index < 0)
			{
				fprintf(stderr, "Reference to undeclared symbol \"%s\" at %d:%d\n", node->value.str_val, node->line, node->col);
				symbol_table_inc_error(table);
			}
			else if (table->symbols[symbol_index].type == SYMBOL_FUNC)
			{
				fprintf(stderr, "Symbol \"%s\" denotes a function name, did you mean \"%s()\" at %d:%d\n", node->value.str_val, node->value.str_val, node->line, node->col);
				symbol_table_inc_error(table);
			}
			else
			{
				node->stack_offset = stack_offset;
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
		case NODE_FUNCTION :
		{
			// node->value.str_val is shared with symbol created by declare()
			int index = declare(table, node->value.str_val, SYMBOL_FUNC, NULL);
			// declare() will return -1 if it failed
			if (index < 0)
			{
				fprintf(stderr, "Redeclaration of symbol \"%s\" at %d:%d\n", node->value.str_val, node->line, node->col);
				symbol_table_inc_error(table);
			}
			else
			{
				// NODE_FUNCTION always has a NODE_SEQUENCE for parameters at index 0 
				int nb_parameters = node->children[0]->nb_children;
				table->symbols[index].nb_params = nb_parameters;
				table->nb_variables = 0;
				startScope(table);
				for (int i = 0; i < node->nb_children; i++)
				{
					semantic_analysis(node->children[i], table);
				}
				endScope(table);
				// Don't allocate space on the stack for the parameters
				node->nb_var = table->nb_variables - nb_parameters;
			}
			break;
		}
		case NODE_CALL:
		{
			int called_index = search(table, node->value.str_val, NULL);
			if (called_index < 0)
			{
				fprintf(stderr, "Call to undefined symbol \"%s\" at %d:%d\n", node->value.str_val, node->line, node->col);
				symbol_table_inc_error(table);
			}
			else if (table->symbols[called_index].type != SYMBOL_FUNC)
			{
				fprintf(stderr, "Symbol \"%s\" is not a function at %d:%d\n", node->value.str_val, node->line, node->col);
				symbol_table_inc_error(table);
			}
			else
			{
				assert(node->nb_children == 1);

				int nb_args = node->children[0]->nb_children;
				int nb_params = table->symbols[called_index].nb_params;
				if (nb_args != nb_params)
				{
					fprintf(stderr, "Incorrect number of arguments to function \"%s()\" at %d:%d, expected %d arguments but %d given \n", 
									node->value.str_val, node->line, node->col, nb_params, nb_args);
					symbol_table_inc_error(table);
				}
				semantic_analysis(node->children[0], table);
			}
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

int declare(SymbolTable* table, char* name, int type, int* stack_offset)
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
		if (type != SYMBOL_FUNC)
		{
			assert(stack_offset != NULL);

			*stack_offset = table->nb_variables;
			table->symbols[table->nb_symbols] = symbol_create(*stack_offset, name, type);
			table->nb_variables++;
		}
		else
		{
			table->symbols[table->nb_symbols] = symbol_create(NO_STACK_OFFSET, name, type);
		}
		table->nb_symbols++;
	}

	return index;
}

int search(SymbolTable* table, char* name, int* stack_offset)
{
	int index = table->nb_symbols - 1;
	int found = 0;
	while (index >= 0 && !found)
	{
		found = strcmp(name, table->symbols[index].name) == 0;
		index--;
	}
	if (stack_offset != NULL)
	{
		if (found)
			*stack_offset = table->symbols[index + 1].stack_offset;
		else
			*stack_offset = NO_STACK_OFFSET;
	}

	return index + found;
}
