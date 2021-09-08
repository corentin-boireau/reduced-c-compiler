#ifndef SYNTACTIC_NODE_H
#define SYNTACTIC_NODE_H

typedef struct SyntacticNode_s SyntacticNode;
struct SyntacticNode_s
{
    int type;
    int value;
    int line;
    int col;
    SyntacticNode** children;
    int nb_children;
};

SyntacticNode* syntactic_node_create(int type, int line, int col);
SyntacticNode* syntactic_node_create_with_value(int type, int line, int col, int value);
void syntactic_node_add_child(SyntacticNode* parent, SyntacticNode* child);
void syntactic_node_display(const SyntacticNode* node);
void syntactic_node_display_tree(const SyntacticNode* root, int depth);

enum
{
    NODE_INVALID = -1,     // Created when an unexpected token is found
    NODE_CONST,            // Constant value
                           
    // Prefix operators    
    NODE_UNARY_MINUS,      // '-' to denote the corresponding negative value
    NODE_NEGATION,         // '!' to denote the corresponding negation
    NODE_INDIRECTION,      // '*' to access the pointed memory
    NODE_ADDRESS,          // '&' to denote the address where the variable is stored

    // Binary operators
    NODE_ASSIGNMENT,       // 
    NODE_OR,               //
    NODE_AND,              //
    NODE_EQUAL,            //
    NODE_NOT_EQUAL,        //
    NODE_GREATER,          //
    NODE_GREATER_OR_EQUAL, //
    NODE_LESS,             //
    NODE_LESS_OR_EQUAL,    //
    NODE_MUL,              //
    NODE_DIV,              //
    NODE_MOD,              //
    NODE_ADD,              //
    NODE_SUB,              //

    NODE_BLOCK,            // Code block between '{' and '}'
    NODE_PRINT,            // Print the top of the stack
    NODE_DROP,             // Pop the stack
};
#endif // SYNTACTIC_NODE_H