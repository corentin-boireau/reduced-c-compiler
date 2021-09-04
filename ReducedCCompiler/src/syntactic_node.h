#ifndef SYNTACTIC_NODE_H
#define SYNTACTIC_NODE_H

typedef struct
{
    int type;
    int value;
    int line;
    int col;
    struct SyntacticNode** children;
    int nb_children;
} SyntacticNode;

SyntacticNode* syntactic_node_create(int type, int line, int col);
SyntacticNode* syntactic_node_create_with_value(int type, int line, int col, int value);
void syntactic_node_add_child(SyntacticNode* const parent, const SyntacticNode* const child);
void syntactic_node_display(const SyntacticNode* node);
void syntactic_node_display_tree(const SyntacticNode* root, int depth);

enum
{
    NODE_INVALID = -1, // Created when an unexpected token is found
    NODE_CONST,        // Constant value
                       
    // Prefix operators
    NODE_UNARY_MINUS,  // '-' to denote the corresponding negative value
    NODE_NEGATION,     // '!' to denote the corresponding negation
    NODE_INDIRECTION,  // '*' to access the pointed memory
    NODE_ADDRESS,      // '&' to denote the address where the variable is stored
};
#endif // SYNTACTIC_NODE_H