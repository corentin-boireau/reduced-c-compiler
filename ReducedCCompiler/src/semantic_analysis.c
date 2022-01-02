#include "semantic_analysis.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void startScope(SymbolTable* table);
void endScope(SymbolTable* table);
// Declares a new symbol with the given name and increments the nb_variables counter.
Symbol* declare(SymbolTable* table, SyntacticNode* declaration);
Symbol* search(SymbolTable* table, char* name);
Symbol symbol_create(int stack_offset, SyntacticNode* decl);


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

    // Fake nodes that hold the two I/O primitive functions
    SyntacticNode* putchar_function = syntactic_node_create(NODE_FUNCTION, 0, 0); 
    putchar_function->value.str_val = "putchar";
    table.symbols[table.nb_symbols] = symbol_create(NO_STACK_OFFSET, putchar_function);
    table.symbols[table.nb_symbols++].nb_params = 1;

    SyntacticNode* getchar_function = syntactic_node_create(NODE_FUNCTION, 0, 0);
    getchar_function->value.str_val = "getchar";
    table.symbols[table.nb_symbols] = symbol_create(NO_STACK_OFFSET, getchar_function);
    table.symbols[table.nb_symbols++].nb_params = 0;

    return table;
}

Symbol symbol_create(int stack_offset, SyntacticNode* decl)
{
    assert(decl != NULL);

    Symbol symbol;
    symbol.stack_offset = stack_offset;
    symbol.declaration  = decl;
    symbol.flags        = 0;

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
            if (table->current_scope == 0)
                syntactic_node_set_flag(node, GLOBAL_FLAG);
            Symbol* var_symbol = declare(table, node);
            if (var_symbol == NULL)
            {
                fprintf(stderr, "(%d:%d):error: Redeclaration of symbol \"%s\".\n", 
                    node->line, node->col, node->value.str_val);
                symbol_table_inc_error(table);
            }
            else
            {
                node->stack_offset = var_symbol->stack_offset;
            }

            if (node->nb_children == 1)
            {
                SyntacticNode* initialization = node->children[0];
                assert(initialization->type == NODE_ASSIGNMENT);
                semantic_analysis(initialization, table);
            }
            break;
        }
        case NODE_REF :
        {
            Symbol* ref_symbol = search(table, node->value.str_val);
            if (ref_symbol == NULL)
            {
                fprintf(stderr, "(%d:%d):error: Reference to undeclared symbol \"%s\".\n", 
                    node->line, node->col, node->value.str_val);
                symbol_table_inc_error(table);
            }
            else
            {
                switch (ref_symbol->declaration->type)
                {
                    case NODE_FUNCTION:
                    {
                        fprintf(stderr, "(%d:%d):error: Symbol \"%s\" denotes a function name, did you mean \"%s()\" ?\n",
                                node->line, node->col, node->value.str_val, node->value.str_val);
                        symbol_table_inc_error(table);
                        break;
                    }
                    case NODE_DECL:
                    {
                        node->flags = ref_symbol->declaration->flags;
                        node->stack_offset = ref_symbol->stack_offset;
                        if (node->parent->type == NODE_ASSIGNMENT && node->parent->children[0] == node)
                        {
                            if ( ! syntactic_node_is_flag_set(node, CONST_FLAG))
                                symbol_set_flag(ref_symbol, SET);
                            else
                            {
                                if (node->parent->parent->type == NODE_DECL) // The assignment is an initialization, so the 'const' variable can be set
                                    symbol_set_flag(ref_symbol, SET);
                                else
                                {
                                    fprintf(stderr, "(%d:%d):error: assignment of read-only variable '%s'\n",
                                        node->line, node->col, node->value.str_val);
                                    symbol_table_inc_error(table);
                                }
                            }
                        }
                        else
                        {
                            if ( ! symbol_is_flag_set(ref_symbol, SET) && ! syntactic_node_is_flag_set(node, GLOBAL_FLAG))
                            {
                                fprintf(stderr, "(%d:%d):warning: '%s' is used uninitialized\n",
                                        node->line, node->col, node->value.str_val);
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
            Symbol* function_symbol = declare(table, node);
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
            else if (called_symbol->declaration->type != NODE_FUNCTION)
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

                assert(ref_symbol != NULL);
                assert(ref_symbol->declaration->type == NODE_DECL);
                
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
    assert(table != NULL);

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
    assert(table != NULL);

    for (int i = table->nb_symbols - 1; i >= table->scopes[table->current_scope]; i--)
    {
        Symbol* symbol = &(table->symbols[i]);
        if ( ! symbol_is_flag_set(symbol, READ))
        {
            if (symbol_is_flag_set(symbol, SET))
            {
                fprintf(stderr, "(%d:%d):warning: '%s' is set but not used\n",
                    symbol->declaration->line, symbol->declaration->col, symbol->declaration->value.str_val);
                symbol_table_inc_warning(table);
            }
            else
            {
                fprintf(stderr, "(%d:%d):warning: unused variable '%s'\n", 
                    symbol->declaration->line, symbol->declaration->col, symbol->declaration->value.str_val);
                symbol_table_inc_warning(table);
            }
        }
        else if ( ! symbol_is_flag_set(symbol, SET))  // Could happen for global variables
        {
            fprintf(stderr, "(%d:%d):warning: '%s' is read but never set\n",
                symbol->declaration->line, symbol->declaration->col, symbol->declaration->value.str_val);
            symbol_table_inc_warning(table);
        }
    }
    table->nb_symbols = table->scopes[table->current_scope];
    table->current_scope--;
}

Symbol* declare(SymbolTable* table, SyntacticNode* declaration)
{
    assert(table != NULL && declaration != NULL);

    Symbol* declared_symbol = NULL;
    int index = table->scopes[table->current_scope];
    bool found = false;
    while (index < table->nb_symbols && !found)
    {
        found = strcmp(declaration->value.str_val, table->symbols[index].declaration->value.str_val) == 0;
        index++;
    }
    if (!found)
    {
        declared_symbol = table->symbols + table->nb_symbols;
        switch (declaration->type)
        {
            case NODE_DECL:
            {
                if (syntactic_node_is_flag_set(declaration, GLOBAL_FLAG))
                {
                    int offset = table->nb_glob_variables;
                    *declared_symbol = symbol_create(offset, declaration);
                    table->nb_glob_variables++;
                }
                else
                {
                    int stack_offset = table->nb_variables;
                    *declared_symbol = symbol_create(stack_offset, declaration);
                    table->nb_variables++;
                }
                break;
            }
            case NODE_FUNCTION:
            {
                *declared_symbol = symbol_create(NO_STACK_OFFSET, declaration);
                break;
            }
            default: // Invalid type
                assert(false); // Should never be reached
        }
        table->nb_symbols++;
    }

    return declared_symbol;
}

Symbol* search(SymbolTable* table, char* name)
{
    assert(table != NULL && name != NULL);

    Symbol* symbol = NULL;
    int index = table->nb_symbols - 1;
    int found = 0;
    while (index >= 0 && !found)
    {
        found = strcmp(name, table->symbols[index].declaration->value.str_val) == 0;
        index--;
    }
    if (found)
    {
        symbol = table->symbols + index + 1;
    }

    return symbol;
}
