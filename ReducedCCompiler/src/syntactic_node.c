#include "syntactic_node.h"

#include <stdlib.h>

SyntacticNode* syntactic_node_create(int type, int line, int col)
{
    SyntacticNode* node = malloc(sizeof(SyntacticNode));
    if (node == NULL)
    {
        perror("Failed to allocate memory for the node");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->line = line;
    node->col  = col;
    node->nb_children = 0;
    node->children = NULL;

    return node;
}

SyntacticNode* syntactic_node_create_with_value(int type, int line, int col, int value)
{
    SyntacticNode* node = syntactic_node_create(type, line, col);
    node->value = value;

    return node;
}

void syntactic_node_add_child(SyntacticNode* parent, SyntacticNode* child)
{
    parent->children = realloc(parent->children, sizeof(SyntacticNode*) * (parent->nb_children + 1));
    parent->children[parent->nb_children++] = child;
}