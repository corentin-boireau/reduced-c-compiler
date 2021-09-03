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

enum
{
    NODE_CONST      // Constant value
};
#endif