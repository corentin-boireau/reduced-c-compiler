#ifndef SYNTACTIC_NODE_H
#define SYNTACTIC_NODE_H

#include <stdbool.h>
#include <stdio.h>

#define NO_STACK_OFFSET -1

typedef struct SyntacticNode_s SyntacticNode;
struct SyntacticNode_s
{
    int type;
    union
    {
        int int_val;
        char* str_val;
    } value;
    int stack_offset;   // Only usefull for variable declaration and references
                        // Indicates its location on the stack
    int  nb_var;        // For functions
    bool is_global;
    int  line;
    int  col;
    SyntacticNode* parent;
    SyntacticNode** children;
    int nb_children;
};

SyntacticNode* syntactic_node_create(int type, int line, int col);
SyntacticNode* syntactic_node_create_with_value(int type, int line, int col, int value);
void syntactic_node_add_child(SyntacticNode* parent, SyntacticNode* child);
void syntactic_node_display(const SyntacticNode* node, FILE *out_file);
void syntactic_node_display_tree(const SyntacticNode* root, int depth, FILE* out_file);

void syntactic_node_free(SyntacticNode* node);
void syntactic_node_free_tree(SyntacticNode* tree);

enum
{
    NODE_INVALID = -1,      // Created when an unexpected token is found
    NODE_CONST,             // Constant value

    // Prefix operators
    NODE_UNARY_MINUS,       // '-' to denote the corresponding negative value
    NODE_NEGATION,          // '!' to denote the corresponding negation
    NODE_DEREF,             // '*' to access the pointed memory
    NODE_ADDRESS,           // '&' to denote the address where the variable is stored

    // Binary operators
    NODE_ASSIGNMENT,        // '=' Assignment of a value to a variable
    NODE_OR,                //
    NODE_AND,               //
    NODE_EQUAL,             //
    NODE_NOT_EQUAL,         //
    NODE_GREATER,           //
    NODE_GREATER_OR_EQUAL,  //
    NODE_LESS,              //
    NODE_LESS_OR_EQUAL,     //
    NODE_MUL,               //
    NODE_DIV,               //
    NODE_MOD,               //
    NODE_ADD,               //
    NODE_SUB,               //

    NODE_DECL,              // Variable declaration
    NODE_REF,               // Reference to a variable
    NODE_BLOCK,             // Code block between '{' and '}'
    NODE_SEQUENCE,          // Code block without a new scope
    NODE_PRINT,             // Print the top of the stack
    NODE_DROP,              // Pop the stack
    NODE_CONDITION,
    NODE_INVERTED_CONDITION,
    NODE_LOOP,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_CONTINUE_LABEL,
    NODE_FUNCTION,
    NODE_PROGRAM,
    NODE_CALL,
    NODE_RETURN,
};
#endif // SYNTACTIC_NODE_H
