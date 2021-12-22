#include "semantic_analysis.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void startScope(SymbolTable* table);
void endScope(SymbolTable* table);
// Declares a new symbol with the given name and increments the nb_variables counter.
Symbol* declare(SymbolTable* table, char* name, int type);
Symbol* search(SymbolTable* table, char* name);
Symbol symbol_create(int stack_offset, char* name, int type);


SymbolTable symbol_table_create()
{
    SymbolTable table;
    table.scopes[0]         = 0;
    table.nb_symbols        = 0;
    table.nb_glob_variables = 0;
    table.nb_variables      = 0;
    table.current_scope     = 0;
    table.nb_errors         = 0;
    table.nb_warnings       = 0;

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
    symbol.flags = 0;

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

void symbol_table_inc_warning(SymbolTable* table)
{
    table->nb_warnings++;
}

void semantic_analysis_report_and_exit(const SymbolTable* table)
{
    assert(table != NULL);

    fprintf(stderr, "\nSemantic analysis warnings : %d\n", table->nb_warnings);
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
            int decl_type = (table->current_scope == 0) ? SYMBOL_GLOBAL_VAR : SYMBOL_LOCAL_VAR;
            Symbol* var_symbol = declare(table, node->value.str_val, decl_type);
            if (var_symbol == NULL)
            {
                fprintf(stderr, "(%d:%d):error: Redeclaration of symbol \"%s\".\n", node->line, node->col, node->value.str_val);
                symbol_table_inc_error(table);
            }
            else
            {
                node->stack_offset = var_symbol->stack_offset;
            }
            break;
        }
        case NODE_REF :
        {
            Symbol* ref_symbol = search(table, node->value.str_val);
            if (ref_symbol == NULL)
            {
                fprintf(stderr, "(%d:%d):error: Reference to undeclared symbol \"%s\".\n", node->line, node->col, node->value.str_val);
                symbol_table_inc_error(table);
            }
            else
            {
                switch (ref_symbol->type)
                {
                    case SYMBOL_FUNC:
                    {
                        fprintf(stderr, "(%d:%d):error: Symbol \"%s\" denotes a function name, did you mean \"%s()\" ?\n",
                                node->line, node->col, node->value.str_val, node->value.str_val);
                        symbol_table_inc_error(table);
                        break;
                    }
                    case SYMBOL_LOCAL_VAR:
                    case SYMBOL_GLOBAL_VAR:
                    {
                        node->is_global = (ref_symbol->type == SYMBOL_GLOBAL_VAR);

                        node->stack_offset = ref_symbol->stack_offset;
                        if (node->parent->type == NODE_ASSIGNMENT && node->parent->children[0] == node)
                            symbol_set_flag(ref_symbol, SET);
                        else
                        {
                            if ( ! symbol_is_flag_set(ref_symbol, SET) && ref_symbol->type == SYMBOL_LOCAL_VAR)
                            {
                                fprintf(stderr, "(%d:%d):warning: '%s' is used uninitialized\n",
                                        node->line, node->col, ref_symbol->name);
                                symbol_table_inc_warning(table);
                            }
                            symbol_set_flag(ref_symbol, READ);
                        }
                        break;
                    }
                    default:
                        assert(false);
                }
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
            Symbol* function_symbol = declare(table, node->value.str_val, SYMBOL_FUNC);
            if (function_symbol == NULL)
            {
                fprintf(stderr, "(%d:%d):error: Redeclaration of symbol \"%s\".\n", node->line, node->col, node->value.str_val);
                symbol_table_inc_error(table);
            }
            else
            {
                // NODE_FUNCTION always has a NODE_SEQUENCE for parameters at index 0
                assert(node->children[0]->type == NODE_SEQUENCE);
                int nb_parameters = node->children[0]->nb_children;
                function_symbol->nb_params = nb_parameters;
                table->nb_variables = 0;
                startScope(table);

                // Set the SET flag on function parameters as they are set by the call
                semantic_analysis(node->children[0], table);
                for (int i = 0; i < nb_parameters; i++)
                    symbol_set_flag(&(table->symbols[table->scopes[table->current_scope] + i]), SET);

                for (int i = 1; i < node->nb_children; i++)
                    semantic_analysis(node->children[i], table);

                endScope(table);
                // Don't allocate space on the stack for the parameters
                node->nb_var = table->nb_variables - nb_parameters;
            }
            break;
        }
        case NODE_CALL:
        {
            Symbol* called_symbol = search(table, node->value.str_val);
            if (called_symbol == NULL)
            {
                fprintf(stderr, "(%d:%d):error: Call to undefined symbol \"%s\".\n", node->line, node->col, node->value.str_val);
                symbol_table_inc_error(table);
            }
            else if (called_symbol->type != SYMBOL_FUNC)
            {
                fprintf(stderr, "(%d:%d):error: Symbol \"%s\" is not a function.\n", node->line, node->col, node->value.str_val);
                symbol_table_inc_error(table);
            }
            else
            {
                assert(node->nb_children == 1);

                int nb_args = node->children[0]->nb_children;
                int nb_params = called_symbol->nb_params;
                if (nb_args != nb_params)
                {
                    fprintf(stderr, "(%d:%d):error: Incorrect number of arguments to function \"%s()\", expected %d arguments but %d given.\n",
                            node->line, node->col, node->value.str_val, nb_params, nb_args);
                    symbol_table_inc_error(table);
                }
                semantic_analysis(node->children[0], table);
            }
            break;
        }
        case NODE_ASSIGNMENT:
        {
            assert(node->nb_children == 2);
            semantic_analysis(node->children[1], table);

            SyntacticNode* assignable = node->children[0];
            assert(assignable->type == NODE_DEREF || assignable->type == NODE_REF);
            semantic_analysis(assignable, table);
            break;
        }
        case NODE_COMPOUND:
        {
            assert(node->nb_children == 1);
            assert(node->children[0]->nb_children == 2);

            semantic_analysis(node->children[0], table);
            // TODO semantic_analysis() on the left operand of the assignment
            SyntacticNode* ref_node = node->children[0]->children[0];
            if (ref_node->type == NODE_REF)
            {
                Symbol* ref_symbol = search(table, ref_node->value.str_val);
                if (ref_symbol != NULL && (ref_symbol->type == SYMBOL_LOCAL_VAR || ref_symbol->type == SYMBOL_GLOBAL_VAR))
                    symbol_set_flag(ref_symbol, SET);
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
    if (table->current_scope > MAX_SCOPES)
    {
        fprintf(stderr, "FATAL ERROR : Max number of scopes exceeded (%d > %d)\n", table->current_scope, MAX_SCOPES);
        exit(EXIT_FAILURE);
    }
    table->scopes[table->current_scope] = table->nb_symbols;
}

void endScope(SymbolTable* table)
{
    for (int i = table->nb_symbols - 1; i >= table->scopes[table->current_scope]; i--)
    {
        Symbol* symbol = &(table->symbols[i]);
        if (!symbol_is_flag_set(symbol, READ))
        {
            if (symbol_is_flag_set(symbol, SET))
            {
                fprintf(stderr, "warning: '%s' is set but not used\n", symbol->name);
                symbol_table_inc_warning(table);
            }
            else
            {
                fprintf(stderr, "warning: unused variable '%s'\n", symbol->name);
                symbol_table_inc_warning(table);
            }
        }
    }
    table->nb_symbols = table->scopes[table->current_scope];
    table->current_scope--;
}

Symbol* declare(SymbolTable* table, char* name, int type)
{
    Symbol* declared_symbol = NULL;
    int index = table->scopes[table->current_scope];
    bool found = false;
    while (index < table->nb_symbols && !found)
    {
        found = strcmp(name, table->symbols[index].name) == 0;
        index++;
    }
    if (!found)
    {
        declared_symbol = table->symbols + table->nb_symbols;
        switch (type)
        {
            case SYMBOL_LOCAL_VAR:
            {
                int stack_offset = table->nb_variables;
                *declared_symbol = symbol_create(stack_offset, name, type);
                table->nb_variables++;
                break;
            }
            case SYMBOL_GLOBAL_VAR:
            {
                int offset = table->nb_glob_variables;
                *declared_symbol = symbol_create(offset, name, type);
                table->nb_glob_variables++;
                break;
            }
            case SYMBOL_FUNC:
            {
                *declared_symbol = symbol_create(NO_STACK_OFFSET, name, type);
                break;
            }
            default: // Invalid type
                assert(0); // Should never be reached
        }
        table->nb_symbols++;
    }

    return declared_symbol;
}

Symbol* search(SymbolTable* table, char* name)
{
    Symbol* symbol = NULL;
    int index = table->nb_symbols - 1;
    int found = 0;
    while (index >= 0 && !found)
    {
        found = strcmp(name, table->symbols[index].name) == 0;
        index--;
    }
    if (found)
    {
        symbol = table->symbols + index + 1;
    }

    return symbol;
}
