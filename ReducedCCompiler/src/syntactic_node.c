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

void syntactic_node_add_child(SyntacticNode* const parent, const SyntacticNode* const child)
{
    SyntacticNode** reallocated_children = realloc(parent->children, sizeof(SyntacticNode*) * (parent->nb_children + 1));
    if (reallocated_children == NULL)
    {
        perror("Failed to allocate memory for the syntactic node's new child");
        exit(EXIT_FAILURE);
    }
    parent->children = reallocated_children;
    parent->children[parent->nb_children++] = child;
}

void syntactic_node_display(const SyntacticNode* node)
{
    switch (node->type)
    {
        case NODE_CONST:          printf("CONST : value = %d\n", node->value);    break;
        case NODE_UNARY_MINUS:    printf("UNARY_MINUS\n");                        break;
        case NODE_NEGATION:       printf("NEGATION\n");                           break;
        case NODE_INDIRECTION:    printf("INDIRECTION\n");                        break;
        case NODE_ADDRESS:        printf("ADDRESS\n");                            break;
    }
}

void syntactic_node_display_tree(const SyntacticNode* root, int depth)
{
    char* depth_indicator = malloc(sizeof(char) * (depth + 1));
    if (depth_indicator == NULL)
    {
        perror("Failed to allocate memory for depth_indicator in syntactic_node_display_tree");
        exit(EXIT_FAILURE);
    }

    memset(depth_indicator, ' ', depth);
    depth_indicator[depth] = '\0';

    syntactic_node_display(root);
    for (int i = 0; i < root->nb_children; i++)
    {
        printf("%s|--", depth_indicator);
        syntactic_node_display_tree(root->children[i], depth + 3);
    }

    free(depth_indicator);
}