#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

#include "syntactic_node.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct Symbol_s Symbol;
struct Symbol_s
{
    SyntacticNode* declaration;
    int            stack_offset;
    int            nb_params;
    uint8_t        flags;
};

#define SET  (1 << 0)
#define READ (1 << 1)

static inline void symbol_set_flag(Symbol* symbol, uint8_t flag)
{
    assert(symbol != NULL);
    symbol->flags |= flag;
}
static inline bool symbol_is_flag_set(Symbol* symbol, uint8_t flag)
{
    assert(symbol != NULL);
    return (symbol->flags & flag) != 0;
}

#define MAX_SYMBOLS 500
#define MAX_SCOPES  20
#define MAX_SEMANTIC_ERROR 3 // If there are more than MAX_SEMANTIC_ERROR, we stop the semantic analysis

typedef struct SymbolTable_s SymbolTable;
struct SymbolTable_s
{
    Symbol symbols[MAX_SYMBOLS];
    int    scopes[MAX_SCOPES + 1];
    int    nb_symbols;
    int    nb_glob_variables;
    int    nb_variables;
    int    current_scope;
    int    nb_errors;
    int    nb_warnings;
};

SymbolTable symbol_table_create();
void semantic_analysis(SyntacticNode* tree, SymbolTable* table);
void semantic_analysis_report_and_exit(const SymbolTable* table);

#endif // SEMANTIC_ANALYSIS_H
