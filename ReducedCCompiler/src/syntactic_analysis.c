#include "syntactic_analysis.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Syntactic rules
SyntacticNode* sr_grammar(SyntacticAnalyzer* analyzer);               // Whole program
SyntacticNode* sr_global_declaration(SyntacticAnalyzer* analyzer);    // Function or Global variable declaration
SyntacticNode* sr_instruction(SyntacticAnalyzer* analyzer);           // Instruction
SyntacticNode* sr_expression(SyntacticAnalyzer* analyzer);            // Expression
SyntacticNode* sr_prefix(SyntacticAnalyzer* analyzer);                // Prefix
SyntacticNode* sr_suffix(SyntacticAnalyzer* analyzer);                // Suffix
SyntacticNode* sr_atom(SyntacticAnalyzer* analyzer);                  // Atom

SyntacticNode* sr_expression_prio(SyntacticAnalyzer* analyzer, int priority); // Expression with priority


SyntacticNode* opti_constant_prefix(SyntacticNode* node, SyntacticAnalyzer* analyzer);

void syntactic_analyzer_inc_error(SyntacticAnalyzer* analyzer)
{
    analyzer->nb_errors++;

    if (analyzer->nb_errors > MAX_SYNTACTIC_ERROR)
    {
        syntactic_analyzer_report_and_exit(analyzer);
    }
}

void syntactic_analyzer_inc_warning(SyntacticAnalyzer* analyzer)
{
    analyzer->nb_warnings++;
}


#define RIGHT_TO_LEFT 0
#define LEFT_TO_RIGHT 1

typedef struct OperatorInfo_s OperatorInfo;
struct OperatorInfo_s
{
    int priority;
    int associativity;
    int node_type;
};
#define NB_BINARY_OPERATORS 19
#define MIN_PRIORITY 0
OperatorInfo get_operator_info(int token_type)
{
    OperatorInfo op_info = { -1, -1, -1 };
    switch (token_type)
    {
        case TOK_EQUAL:
        {
            op_info.priority      = 1;
            op_info.associativity = RIGHT_TO_LEFT;
            op_info.node_type     = NODE_ASSIGNMENT;
            break;
        }
        case TOK_PLUS_EQUAL:
        case TOK_MINUS_EQUAL:
        case TOK_MUL_EQUAL:
        case TOK_DIV_EQUAL:
        case TOK_MOD_EQUAL:
        {
            op_info.priority      = 1;
            op_info.associativity = RIGHT_TO_LEFT;
            op_info.node_type     = NODE_COMPOUND;
            break;
        }
        case TOK_2_PIPE:
        {
            op_info.priority      = 2;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_OR;
            break;
        }
        case TOK_2_AMPERSAND:
        {
            op_info.priority      = 3;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_AND;
            break;
        }
        case TOK_2_EQUAL:
        {
            op_info.priority      = 4;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_EQUAL;
            break;
        }
        case TOK_NOT_EQUAL:
        {
            op_info.priority      = 4;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_NOT_EQUAL;
            break;
        }
        case TOK_GREATER:
        {
            op_info.priority      = 5;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_GREATER;
            break;
        }
        case TOK_GREATER_OR_EQUAL:
        {
            op_info.priority      = 5;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_GREATER_OR_EQUAL;
            break;
        }
        case TOK_LESS:
        {
            op_info.priority      = 5;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_LESS;
            break;
        }
        case TOK_LESS_OR_EQUAL:
        {
            op_info.priority      = 5;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_LESS_OR_EQUAL;
            break;
        }
        case TOK_PLUS:
        {
            op_info.priority      = 6;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_ADD;
            break;
        }
        case TOK_MINUS:
        {
            op_info.priority      = 6;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_SUB;
            break;
        }
        case TOK_STAR:
        {
            op_info.priority      = 7;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_MUL;
            break;
        }
        case TOK_SLASH:
        {
            op_info.priority      = 7;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_DIV;
            break;
        }
        case TOK_PERCENT:
        {
            op_info.priority      = 7;
            op_info.associativity = LEFT_TO_RIGHT;
            op_info.node_type     = NODE_MOD;
            break;
        }
    }

    assert(op_info.priority != -1);
    assert(op_info.associativity != -1);
    assert(op_info.node_type != -1);
    return op_info;
}

static inline bool is_binary_op(int token_type)
{
    return 0 <= token_type && token_type < NB_BINARY_OPERATORS;
}

static inline bool is_flag_set(uint8_t flags, uint8_t flag_to_check)
{
    return (flags & flag_to_check) != 0;
}
static inline uint8_t set_flag(uint8_t flags, uint8_t flag_to_add)
{
    return flags | flag_to_add;
}

uint8_t subrule_specifiers(SyntacticAnalyzer* analyzer, uint8_t specifier_flags)
{
    while (tokenizer_check(&(analyzer->tokenizer), TOK_CONST_SPECIFIER))
    {
        if (is_flag_set(specifier_flags, CONST_FLAG))
        {
            fprintf(stderr, "(%d:%d):warning: duplicate 'const' declaration specifier\n", analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
            syntactic_analyzer_inc_warning(analyzer);
        }
        else
        {
            specifier_flags = set_flag(specifier_flags, CONST_FLAG);
        }
    }

    return specifier_flags;
}

SyntacticNode* subrule_decl_initialization(SyntacticAnalyzer* analyzer, SyntacticNode *decl)
{
    SyntacticNode* ref = syntactic_node_create(NODE_REF, decl->line, decl->col);
    size_t nb_char = strlen(decl->value.str_val);
    ref->value.str_val = malloc((nb_char + 1) * sizeof(char));
    if (ref->value.str_val == NULL)
    {
        perror("Failed to allocate space for reference's name");
        exit(EXIT_FAILURE);
    }
    memcpy(ref->value.str_val, decl->value.str_val, (nb_char + 1) * sizeof(char));

    SyntacticNode* assignment = syntactic_node_create(NODE_ASSIGNMENT, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
    SyntacticNode* expr = sr_expression(analyzer);

    syntactic_node_add_child(assignment, ref);
    syntactic_node_add_child(assignment, expr);

    return assignment;
}

SyntacticNode* subrule_single_decl(SyntacticAnalyzer* analyzer, SyntacticNode* declaration_seq, bool allow_init)
{
    tokenizer_accept(&(analyzer->tokenizer), TOK_IDENTIFIER);
    SyntacticNode* decl = syntactic_node_create(NODE_DECL, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
    decl->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
    analyzer->tokenizer.current.value.str_val = NULL;
    syntactic_node_add_child(declaration_seq, decl);

    if (allow_init && tokenizer_check(&(analyzer->tokenizer), TOK_EQUAL))
    {
        SyntacticNode* init = subrule_decl_initialization(analyzer, decl);
        syntactic_node_add_child(decl, init);
    }

    return decl;
}

SyntacticNode* subrule_decl_instruction(SyntacticAnalyzer* analyzer, bool allow_init)
{
    SyntacticNode *declarations = syntactic_node_create(NODE_SEQUENCE, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
    subrule_single_decl(analyzer, declarations, allow_init);

    while (!tokenizer_check(&(analyzer->tokenizer), TOK_SEMICOLON))
    {
        tokenizer_accept(&(analyzer->tokenizer), TOK_COMMA);
        subrule_single_decl(analyzer, declarations, allow_init);
    }

    return declarations;
}

SyntacticAnalyzer syntactic_analyzer_create(char* source_buffer, optimization_t optimizations)
{
    assert(source_buffer != NULL);

    SyntacticAnalyzer analyzer;

    analyzer.tokenizer      = tokenizer_create(source_buffer);
    analyzer.syntactic_tree = NULL;
    analyzer.nb_errors      = 0;
    analyzer.nb_warnings    = 0;
    analyzer.optimizations  = optimizations;

    return analyzer;
}

SyntacticNode* syntactic_analyzer_build_tree(SyntacticAnalyzer* analyzer)
{
    assert(analyzer != NULL);

    tokenizer_step(&(analyzer->tokenizer));
    if ( ! (analyzer->tokenizer.next.type == TOK_EOF))
    {
        analyzer->syntactic_tree = sr_grammar(analyzer);
    }

    return analyzer->syntactic_tree;
}


void syntactic_analyzer_report_and_exit(const SyntacticAnalyzer* analyzer)
{
    assert(analyzer != NULL);

    fprintf(stderr, "\nSyntactic analysis warnings : %d\n", analyzer->nb_warnings);
    fprintf(stderr, "\nSyntactic analysis errors : %d\n", analyzer->nb_errors);
    exit(EXIT_FAILURE);
}


// Syntactic rules

SyntacticNode* sr_grammar(SyntacticAnalyzer* analyzer)
{
    assert(analyzer != NULL);

    // G ---> (funDecl | varDecl)*
    SyntacticNode* program = syntactic_node_create(NODE_PROGRAM, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
    while (!tokenizer_check(&(analyzer->tokenizer), TOK_EOF))
    {
        SyntacticNode* global_decl = sr_global_declaration(analyzer);
        syntactic_node_add_child(program, global_decl);
    }
    return program;
}



SyntacticNode* sr_global_declaration(SyntacticAnalyzer* analyzer)
{
    assert(analyzer != NULL);

    SyntacticNode* global_decl = NULL;

    uint8_t specifiers = subrule_specifiers(analyzer, 0);

    if (tokenizer_check(&(analyzer->tokenizer), TOK_INT))
    {
        specifiers = subrule_specifiers(analyzer, specifiers);
        tokenizer_accept(&(analyzer->tokenizer), TOK_IDENTIFIER);
        Token tok_identifier = analyzer->tokenizer.current;
        if (tokenizer_check(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS))
        {
            global_decl = syntactic_node_create(NODE_FUNCTION, tok_identifier.line, tok_identifier.col);
            global_decl->value.str_val = tok_identifier.value.str_val; // Steal the pointer from the token to avoid a copy
            global_decl->flags = specifiers;

            if (syntactic_node_is_flag_set(global_decl, CONST_FLAG))
            {
                fprintf(stderr, "(%d:%d):warning: 'const' specifier ignored on function return type\n", global_decl->line, global_decl->col);
                syntactic_analyzer_inc_warning(analyzer);
            }

            SyntacticNode* seq = syntactic_node_create(NODE_SEQUENCE, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
            // args
            if ( ! tokenizer_check(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS))
            {
                do
                {
                    tokenizer_accept(&(analyzer->tokenizer), TOK_INT);
                    tokenizer_accept(&(analyzer->tokenizer), TOK_IDENTIFIER);
                    SyntacticNode* decl = syntactic_node_create(NODE_DECL, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
                    decl->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
                    syntactic_node_add_child(seq, decl);
                } while (tokenizer_check(&(analyzer->tokenizer), TOK_COMMA));
                tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
            }
            syntactic_node_add_child(global_decl, seq);

            tokenizer_accept(&(analyzer->tokenizer), TOK_OPEN_BRACE);
            SyntacticNode* function_body = syntactic_node_create(NODE_SEQUENCE, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
            while (!tokenizer_check(&(analyzer->tokenizer), TOK_CLOSE_BRACE))
            {
                syntactic_node_add_child(function_body, sr_instruction(analyzer));
            }
            syntactic_node_add_child(global_decl, function_body);
        }
        else
        {
            global_decl = syntactic_node_create(NODE_SEQUENCE, tok_identifier.line, tok_identifier.col);
            SyntacticNode* var_decl = syntactic_node_create(NODE_DECL, tok_identifier.line, tok_identifier.col);
            var_decl->value.str_val = tok_identifier.value.str_val; // Steal the pointer from the token to avoid a copy
            syntactic_node_add_child(global_decl, var_decl);
            if (tokenizer_check(&(analyzer->tokenizer), TOK_EQUAL))
            {
                SyntacticNode* init = subrule_decl_initialization(analyzer, var_decl);
                syntactic_node_add_child(var_decl, init);
            }

            while (!tokenizer_check(&(analyzer->tokenizer), TOK_SEMICOLON))
            {
                tokenizer_accept(&(analyzer->tokenizer), TOK_COMMA);
                subrule_single_decl(analyzer, global_decl, true);
            }

            for (int i = 0; i < global_decl->nb_children; i++)
                global_decl->children[i]->flags = specifiers;
        }

    }
    else
    { // Unexpected token
        global_decl = syntactic_node_create(NODE_INVALID, analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
        fprintf(stderr, "(%d:%d):error: Unexpected token : ", analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
        token_display_given(analyzer->tokenizer.next, stderr);
        fprintf(stderr, "\n");
        syntactic_analyzer_inc_error(analyzer);
    }

    return global_decl;
}

SyntacticNode* sr_instruction(SyntacticAnalyzer* analyzer)
{
    assert(analyzer != NULL);

    SyntacticNode* node;

    // To handle empty instructions (ex : "int x = 1;;")
    // Might also handle empty blocks (ex : "{}")
    // while (tokenizer_check(&(analyzer->tokenizer), TOK_SEMICOLON));

    uint8_t specifiers = subrule_specifiers(analyzer, 0);
    if (specifiers != 0)
    { // I ---> specifier* 'int' specifier* ident ('=' E)? (',' ident ('=' E)? )* ';'
        tokenizer_accept(&(analyzer->tokenizer), TOK_INT);
        specifiers = subrule_specifiers(analyzer, specifiers);
        node = subrule_decl_instruction(analyzer, true);
        for (int i = 0; i < node->nb_children; i++)
            node->children[i]->flags = specifiers;
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_INT))
    { // I ---> 'int' specifier* ident ('=' E)? (',' ident ('=' E)? )* ';'
        specifiers = subrule_specifiers(analyzer, specifiers);
        node = subrule_decl_instruction(analyzer, true);
        for (int i = 0; i < node->nb_children; i++)
            node->children[i]->flags = specifiers;
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_OPEN_BRACE))
    { // I ---> '{' I* '}'
        node = syntactic_node_create(NODE_BLOCK, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        while (!tokenizer_check(&(analyzer->tokenizer), TOK_CLOSE_BRACE))
        {
            syntactic_node_add_child(node, sr_instruction(analyzer));
        }
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_PRINT))
    { // I ---> 'print' E ';'
        node = syntactic_node_create(NODE_PRINT, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode* expr_printed = sr_expression(analyzer);
        syntactic_node_add_child(node, expr_printed);
        tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_IF))
    { // I ---> 'if' '(' E ')' I ('else' I)?
        node = syntactic_node_create(NODE_CONDITION, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        tokenizer_accept(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS);
        SyntacticNode *expr = sr_expression(analyzer);
        syntactic_node_add_child(node, expr);
        tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
        SyntacticNode* instruction1 = sr_instruction(analyzer);
        syntactic_node_add_child(node, instruction1);
        if (tokenizer_check(&(analyzer->tokenizer), TOK_ELSE))
        {
            SyntacticNode* instruction2 = sr_instruction(analyzer);
            syntactic_node_add_child(node, instruction2);
        }
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_WHILE))
    { // I ---> 'while' '(' E ')' I
        node = syntactic_node_create(NODE_LOOP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode *continue_label = syntactic_node_create(NODE_CONTINUE_LABEL, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        tokenizer_accept(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS);
        SyntacticNode *cond = syntactic_node_create(NODE_CONDITION, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode *expr = sr_expression(analyzer);
        tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
        SyntacticNode* instruction = sr_instruction(analyzer);
        SyntacticNode *node_break = syntactic_node_create(NODE_BREAK, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);

        syntactic_node_add_child(node, continue_label);

        syntactic_node_add_child(cond, expr);
        syntactic_node_add_child(cond, instruction);
        syntactic_node_add_child(cond, node_break);

        syntactic_node_add_child(node, cond);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_DO))
    { // I ---> 'do' I 'while' '(' E ')' ';'
        node = syntactic_node_create(NODE_LOOP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode* instruction = sr_instruction(analyzer);
        tokenizer_accept(&(analyzer->tokenizer), TOK_WHILE);
        tokenizer_accept(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS);
        SyntacticNode *continue_label = syntactic_node_create(NODE_CONTINUE_LABEL, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode *inv_cond = syntactic_node_create(NODE_INVERTED_CONDITION, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode *expr = sr_expression(analyzer);
        tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
        tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
        SyntacticNode *node_break = syntactic_node_create(NODE_BREAK, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);

        syntactic_node_add_child(inv_cond, expr);
        syntactic_node_add_child(inv_cond, node_break);

        syntactic_node_add_child(node, instruction);
        syntactic_node_add_child(node, continue_label);
        syntactic_node_add_child(node, inv_cond);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_FOR))
    { // I ---> 'for' '(' E1 ';' E2 ';' E3 ')' I
        // We want to permit declaration in the 'E1' space but we don't want to pollute the scope with it.
        // Thus, we embrace the loop in a block that will create a scope.
        node = syntactic_node_create(NODE_BLOCK, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode *loop = syntactic_node_create(NODE_LOOP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        tokenizer_accept(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS);

        if (tokenizer_check(&(analyzer->tokenizer), TOK_INT))
        { // I ---> 'int' ident ('=' E)? (',' ident ('=' E)? )* ';'
            SyntacticNode* decl_seq = subrule_decl_instruction(analyzer, 1);
            syntactic_node_add_child(node, decl_seq);
        }
        else
        {
            SyntacticNode* expr1 = sr_expression(analyzer);
            SyntacticNode* drop1 = syntactic_node_create(NODE_DROP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
            tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
            syntactic_node_add_child(drop1, expr1);
            syntactic_node_add_child(node, drop1);
        }
        SyntacticNode *inv_cond = syntactic_node_create(NODE_INVERTED_CONDITION, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode *expr2 = sr_expression(analyzer);
        tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
        SyntacticNode *continue_label = syntactic_node_create(NODE_CONTINUE_LABEL, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode *expr3 = sr_expression(analyzer);
        SyntacticNode *drop3 = syntactic_node_create(NODE_DROP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
        SyntacticNode* instruction = sr_instruction(analyzer);
        SyntacticNode *node_break = syntactic_node_create(NODE_BREAK, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);

        // As we don't want to create an additional scope, we put a sequence in place of the block in instruction
        if (instruction->type == NODE_BLOCK)
            instruction->type = NODE_SEQUENCE;

        syntactic_node_add_child(inv_cond, expr2);
        syntactic_node_add_child(inv_cond, node_break);

        syntactic_node_add_child(drop3, expr3);

        syntactic_node_add_child(loop, inv_cond);
        syntactic_node_add_child(loop, instruction);
        syntactic_node_add_child(loop, continue_label);
        syntactic_node_add_child(loop, drop3);

        syntactic_node_add_child(node, loop);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_CONTINUE))
    { // I ---> 'continue' ';'
        node = syntactic_node_create(NODE_CONTINUE, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_BREAK))
    { // I ---> 'break' ';'
        node = syntactic_node_create(NODE_BREAK, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_RETURN))
    { // I ---> 'return' E ';'
        node = syntactic_node_create(NODE_RETURN, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        if (!tokenizer_check(&(analyzer->tokenizer), TOK_SEMICOLON))
        {
            SyntacticNode* expr = sr_expression(analyzer);
            syntactic_node_add_child(node, expr);
            tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
        }
    }
    else
    { // I ---> E ';'
        node = syntactic_node_create(NODE_DROP, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        syntactic_node_add_child(node, sr_expression(analyzer));
        tokenizer_accept(&(analyzer->tokenizer), TOK_SEMICOLON);
    }

    return node;
}

SyntacticNode* sr_expression(SyntacticAnalyzer* analyzer)
{
    return sr_expression_prio(analyzer, MIN_PRIORITY);
}

SyntacticNode* sr_expression_prio(SyntacticAnalyzer* analyzer, int priority)
{
    assert(analyzer != NULL);

    // E ---> P '=' E
    //      | P '+=' E
    //      | P '-=' E
    //      | P '*=' E
    //      | P '/=' E
    //      | P '%=' E
    //      | P '||' E
    //      | P '&&' E
    //      | P '==' | '!=' E
    //      | P '<' | '<=' | '>' | '>='  E
    //      | P '+' | '-' E
    //      | P '*' | '/' E

    SyntacticNode* node = sr_prefix(analyzer);
    bool end_expr = false;
    while (!end_expr)
    {
        if (is_binary_op(analyzer->tokenizer.next.type))
        {
            Token token_operator = analyzer->tokenizer.next;
            OperatorInfo node_info = get_operator_info(token_operator.type);
            if (node_info.priority < priority)
            {
                end_expr = true;
            }
            else
            {
                tokenizer_step(&(analyzer->tokenizer));
                SyntacticNode* operand1 = node;
                SyntacticNode* operand2 = sr_expression_prio(analyzer, node_info.priority + node_info.associativity);

                bool has_been_folded = false;
                // Optimization of operations on constants
                if (is_opti_enabled(analyzer->optimizations, OPTI_CONST_FOLD)
                    && node_info.node_type != NODE_ASSIGNMENT && node_info.node_type != NODE_COMPOUND
                    && operand1->type == NODE_CONSTANT && operand2->type == NODE_CONSTANT)
                {
                    int value = -1;
                    const int op1_val = operand1->value.int_val;
                    const int op2_val = operand2->value.int_val;
                    switch (node_info.node_type)
                    {
                        case NODE_AND:              value = op1_val &&  op2_val;  break;
                        case NODE_OR:               value = op1_val ||  op2_val;  break;
                        case NODE_EQUAL:            value = op1_val ==  op2_val;  break;
                        case NODE_NOT_EQUAL:        value = op1_val !=  op2_val;  break;
                        case NODE_LESS:             value = op1_val <   op2_val;  break;
                        case NODE_LESS_OR_EQUAL:    value = op1_val <=  op2_val;  break;
                        case NODE_GREATER:          value = op1_val >   op2_val;  break;
                        case NODE_GREATER_OR_EQUAL: value = op1_val >=  op2_val;  break;

                        case NODE_ADD:
                        {
                            if ((op2_val > 0 && (op1_val > (INT_MAX - op2_val)))
                                || (op2_val < 0 && (op1_val < (INT_MIN - op2_val))))
                            {
                                fprintf(stderr, "(%d:%d):warning: integer overflow in '+' operation\n", token_operator.line, token_operator.col);
                                syntactic_analyzer_inc_warning(analyzer);
                            }
                            else
                            {
                                value = op1_val + op2_val;
                                has_been_folded = true;
                            }
                            break;
                        }
                        case NODE_SUB:
                        {
                            if ((op2_val > 0 && op1_val < INT_MIN + op2_val)
                                || (op2_val < 0 && op1_val > INT_MAX + op2_val))
                            {
                                fprintf(stderr, "(%d:%d):warning: integer overflow in '-' operation\n", token_operator.line, token_operator.col);
                                syntactic_analyzer_inc_warning(analyzer);
                            }
                            else
                            {
                                value = op1_val - op2_val;
                                has_been_folded = true;
                            }
                            break;
                        }
                        case NODE_MUL:
                        {
                            bool will_overflow = false;
                            if ((op1_val == -1 && op2_val == INT_MIN) || (op1_val == INT_MIN && op2_val == -1))
                                will_overflow = true;
                            else if (op2_val > 1) // multiplicaton by 0 or 1 can't overflow
                                will_overflow = (op1_val > INT_MAX / op2_val) || (op1_val < INT_MIN / op2_val);
                            else if (op2_val < -1) // multiplicaton by -1 only overflows with INT_MIN and has already been checked
                                will_overflow = (op1_val < INT_MAX / op2_val) || (op1_val > INT_MIN / op2_val);

                            if (will_overflow)
                            {
                                fprintf(stderr, "(%d:%d):warning: integer overflow in '*' operation\n", token_operator.line, token_operator.col);
                                syntactic_analyzer_inc_warning(analyzer);
                            }
                            else
                            {
                                value = op1_val * op2_val;
                                has_been_folded = true;
                            }
                            break;
                        }
                        case NODE_DIV:
                        {
                            if (op2_val == 0)
                            {
                                fprintf(stderr, "(%d:%d):warning: divison by zero\n", token_operator.line, token_operator.col);
                                syntactic_analyzer_inc_warning(analyzer);
                            }
                            else if (op1_val == INT_MIN && op2_val == -1)
                            {
                                fprintf(stderr, "(%d:%d):warning: integer overflow in '/' operation\n", token_operator.line, token_operator.col);
                                syntactic_analyzer_inc_warning(analyzer);
                            }
                            else
                            {
                                value = op1_val / op2_val;
                                has_been_folded = true;
                            }
                            break;
                        }
                        case NODE_MOD:
                        {
                            if (op2_val == 0)
                            {
                                fprintf(stderr, "(%d:%d):warning: divison by zero:\n", token_operator.line, token_operator.col);
                                syntactic_analyzer_inc_warning(analyzer);
                            }
                            else if (op1_val == INT_MIN && op2_val == -1)
                            {
                                fprintf(stderr, "(%d:%d):warning: integer overflow in '%%' operation\n", token_operator.line, token_operator.col);
                                syntactic_analyzer_inc_warning(analyzer);
                            }
                            else
                            {
                                value = op1_val % op2_val;
                                has_been_folded = true;
                            }
                            break;
                        }
                    }

                    if(has_been_folded)
                        node = syntactic_node_create_with_value(NODE_CONSTANT, operand1->line, operand1->col, value);
                }

                if ( ! has_been_folded)
                {
                    if (node_info.node_type != NODE_ASSIGNMENT && node_info.node_type != NODE_COMPOUND)
                    {
                        node = syntactic_node_create(node_info.node_type, token_operator.line, token_operator.col);
                        syntactic_node_add_child(node, operand1);
                        syntactic_node_add_child(node, operand2);
                    }
                    else if (operand1->type != NODE_REF && operand1->type != NODE_DEREF)
                    {
                        fprintf(stderr, "(%d:%d):error: Left operand of assignement must be a lvalue.\n", operand1->line, operand1->col);
                        syntactic_analyzer_inc_error(analyzer);
                    }
                    else
                    {
                        node = syntactic_node_create(node_info.node_type, token_operator.line, token_operator.col);
                        if (node_info.node_type == NODE_ASSIGNMENT)
                        {
                            syntactic_node_add_child(node, operand1);
                            syntactic_node_add_child(node, operand2);
                        }
                        else // NODE_COMPOUND
                        {
                            SyntacticNode* arithmetic_op = NULL;
                            switch (token_operator.type)
                            {
                                case TOK_PLUS_EQUAL:  arithmetic_op = syntactic_node_create(NODE_ADD, token_operator.line, token_operator.col); break;
                                case TOK_MINUS_EQUAL: arithmetic_op = syntactic_node_create(NODE_SUB, token_operator.line, token_operator.col); break;
                                case TOK_MUL_EQUAL:   arithmetic_op = syntactic_node_create(NODE_MUL, token_operator.line, token_operator.col); break;
                                case TOK_DIV_EQUAL:   arithmetic_op = syntactic_node_create(NODE_DIV, token_operator.line, token_operator.col); break;
                                case TOK_MOD_EQUAL:   arithmetic_op = syntactic_node_create(NODE_MOD, token_operator.line, token_operator.col); break;
                                default:              assert(false);
                            }
                            syntactic_node_add_child(arithmetic_op, operand1);
                            syntactic_node_add_child(arithmetic_op, operand2);
                            syntactic_node_add_child(node, arithmetic_op);
                        }
                    }
                }
            }
        }
        else
            end_expr = true;
    }

    return node;
}

SyntacticNode* sr_prefix(SyntacticAnalyzer* analyzer)
{
    assert(analyzer != NULL);

    SyntacticNode* node;

    if (tokenizer_check(&(analyzer->tokenizer), TOK_PLUS))
    { // P ---> '+' P
        node = sr_prefix(analyzer);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_MINUS))
    { // P ---> '-' P
        node = syntactic_node_create(NODE_UNARY_MINUS, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode* next_prefix_node = sr_prefix(analyzer);
        syntactic_node_add_child(node, next_prefix_node);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_NOT))
    { // P ---> '!' P
        node = syntactic_node_create(NODE_NEGATION, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode* next_prefix_node = sr_prefix(analyzer);
        syntactic_node_add_child(node, next_prefix_node);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_STAR))
    { // P ---> '*' P
        node = syntactic_node_create(NODE_DEREF, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode* prefix = sr_prefix(analyzer);
        syntactic_node_add_child(node, prefix);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_AMPERSAND))
    { // P ---> '&' P
        Token tok_ampersand = analyzer->tokenizer.current; // We keep the token to be able to create a NODE_ADDRESS from it if neeeded
        SyntacticNode* prefix = sr_prefix(analyzer);
        if (prefix->type == NODE_DEREF)
        {
            assert(prefix->nb_children == 1);
            node = prefix->children[0];
            node->parent = NULL; // Since the parent is deleted, the reference is removed
            free(prefix);
        }
        else if (prefix->type == NODE_REF)
        {
            node = syntactic_node_create(NODE_ADDRESS, tok_ampersand.line, tok_ampersand.col);
            syntactic_node_add_child(node, prefix);
        }
        else
        {
            node = syntactic_node_create(NODE_INVALID, tok_ampersand.line, tok_ampersand.col);
            fprintf(stderr, "(%d:%d):error: Unexpected node. lvalue required as unary '&' operand but this node was given :\n", prefix->line, prefix->col);
            syntactic_node_display(prefix, stderr);
            syntactic_analyzer_inc_error(analyzer);
        }

    }
    else
    { // P ---> S
        node = sr_suffix(analyzer);
    }

    // *** Optimizations ***
    if (is_opti_enabled(analyzer->optimizations, OPTI_CONST_FOLD))
        node = opti_constant_prefix(node, analyzer);

    // **********************
    return node;
}

SyntacticNode* sr_suffix(SyntacticAnalyzer* analyzer)
{
    assert(analyzer != NULL);

    // S ---> A ( '[' E ']' )*
    SyntacticNode* node = sr_atom(analyzer);
    while (tokenizer_check(&(analyzer->tokenizer), TOK_OPEN_BRACKET))
    {
        SyntacticNode* deref = syntactic_node_create(NODE_DEREF, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
        SyntacticNode* index_expr = sr_expression(analyzer);
        tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_BRACKET);

        SyntacticNode* indexation_addr = syntactic_node_create(NODE_ADD, index_expr->line, index_expr->col);
        syntactic_node_add_child(indexation_addr, node);
        syntactic_node_add_child(indexation_addr, index_expr);
        syntactic_node_add_child(deref, indexation_addr);
        node = deref;
    }
    return node ;
}

SyntacticNode* sr_atom(SyntacticAnalyzer* analyzer)
{
    assert(analyzer != NULL);

    SyntacticNode* node;

    if (tokenizer_check(&(analyzer->tokenizer), TOK_CONSTANT))
    { // A ---> const
        node = syntactic_node_create_with_value(NODE_CONSTANT, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col, analyzer->tokenizer.current.value.int_val);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS))
    { // A ---> '(' E ')'
        node = sr_expression(analyzer);
        tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
    }
    else if (tokenizer_check(&(analyzer->tokenizer), TOK_IDENTIFIER))
    { // A ---> ident
        int line = analyzer->tokenizer.current.line;
        int col  = analyzer->tokenizer.current.col;
        // var;
        if (!tokenizer_check(&(analyzer->tokenizer), TOK_OPEN_PARENTHESIS))
        {
            node = syntactic_node_create(NODE_REF, line, col);
            node->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
            analyzer->tokenizer.current.value.str_val = NULL;
        }
        // function(arg1, ...)
        else
        {
            node = syntactic_node_create(NODE_CALL, line, col);
            node->value.str_val = analyzer->tokenizer.current.value.str_val; // Steal the pointer from the token to avoid a copy
            analyzer->tokenizer.current.value.str_val = NULL;
            SyntacticNode *seq = syntactic_node_create(NODE_SEQUENCE, analyzer->tokenizer.current.line, analyzer->tokenizer.current.col);
            // args
            if (!tokenizer_check(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS))
            {
                do
                {
                    SyntacticNode* arg = sr_expression(analyzer);
                    syntactic_node_add_child(seq, arg);
                } while (tokenizer_check(&(analyzer->tokenizer), TOK_COMMA));
                tokenizer_accept(&(analyzer->tokenizer), TOK_CLOSE_PARENTHESIS);
            }
            syntactic_node_add_child(node, seq);
        }
    }
    else
    { // Unexpected token
        node = syntactic_node_create(NODE_INVALID, analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
        fprintf(stderr, "(%d:%d):error: Unexpected token : ", analyzer->tokenizer.next.line, analyzer->tokenizer.next.col);
        token_display_given(analyzer->tokenizer.next, stderr);
        fprintf(stderr, "\n");
        syntactic_analyzer_inc_error(analyzer);
    }

    return node;
}

// Optimizations

SyntacticNode* opti_constant_prefix(SyntacticNode* node, SyntacticAnalyzer* analyzer)
{
    SyntacticNode* optimized_node = node;

    switch (node->type)
    {
        case NODE_UNARY_MINUS:
        {
            assert(node->children[0] != NULL);
            SyntacticNode* constant = node->children[0];

            if (constant->type == NODE_CONSTANT)
            {
                optimized_node = constant;
                if (constant->value.int_val == INT_MIN)
                {
                    fprintf(stderr, "(%d:%d):warning: negating INT_MIN (%d) would overflow, value let to %d\n", node->line, node->col, INT_MIN, INT_MIN);
                    syntactic_analyzer_inc_warning(analyzer);
                }
                else
                {
                    optimized_node->value.int_val = - constant->value.int_val;
                    syntactic_node_free(optimized_node->parent);
                    optimized_node->parent = NULL;
                }

            }
            break;
        }
        case NODE_NEGATION:
        {
            assert(node->children[0] != NULL);
            SyntacticNode* constant = node->children[0];

            if (constant->type == NODE_CONSTANT)
            {
                optimized_node = constant;
                optimized_node->value.int_val = ! constant->value.int_val;

                syntactic_node_free(optimized_node->parent);
                optimized_node->parent = NULL;
            }
            break;
        }
    }

    return optimized_node;
}
