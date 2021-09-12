#include "semantic_analysis.h"

void startScope(SymbolTable* table);
void endScope(SymbolTable* table);
// Declares a new symbol with the given name and increments the nb_variables counter.
void declare(SymbolTable* table, char* name); 
Symbol* search(SymbolTable* table, char* name);


void semantic_analysis(SyntacticNode* node, SymbolTable* table)
{
	switch(node->type)
	{
		case NODE_DECL :
		{
			declare(table, node->value.str_val);
			break;
		}
		case NODE_REF :
		{
			Symbol* sym = search(table, node->value.str_val);
			node->index = sym->index;
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