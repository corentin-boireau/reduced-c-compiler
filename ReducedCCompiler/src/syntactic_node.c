#include "syntactic_node.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    node->index = INVALID_INDEX;

    return node;
}

SyntacticNode* syntactic_node_create_with_value(int type, int line, int col, int value)
{
    SyntacticNode* node = syntactic_node_create(type, line, col);
    node->value.int_val = value;

    return node;
}

void syntactic_node_add_child(SyntacticNode* parent, SyntacticNode* child)
{
    assert(parent != NULL);
    assert(child != NULL);

    SyntacticNode** reallocated_children = realloc(parent->children, sizeof(SyntacticNode*) * (parent->nb_children + (size_t) 1));
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
        case NODE_INVALID:          printf("INVALID\n");                                                         break;
        case NODE_CONST:            printf("CONST : value = %d\n", node->value.int_val);                         break;
        case NODE_UNARY_MINUS:      printf("UNARY_MINUS\n");                                                     break;
        case NODE_NEGATION:         printf("NEGATION\n");                                                        break;
        case NODE_INDIRECTION:      printf("INDIRECTION\n");                                                     break;
        case NODE_ADDRESS:          printf("ADDRESS\n");                                                         break;
        case NODE_ADD:              printf("ADD\n");                                                             break;
        case NODE_SUB:              printf("SUB\n");                                                             break;
        case NODE_MUL:              printf("MUL\n");                                                             break;
        case NODE_DIV:              printf("DIV\n");                                                             break;
        case NODE_MOD:              printf("MOD\n");                                                             break;
        case NODE_BLOCK:            printf("BLOCK\n");                                                           break;
        case NODE_SEQUENCE:         printf("SEQUENCE\n");                                                        break;
        case NODE_PRINT:            printf("PRINT\n");                                                           break;
        case NODE_EQUAL:            printf("EQUAL\n");                                                           break;
        case NODE_NOT_EQUAL:        printf("NOT EQUAL\n");                                                       break;
        case NODE_GREATER:          printf("GREATER\n");                                                         break;
        case NODE_GREATER_OR_EQUAL: printf("GREATER OR EQUAL\n");                                                break;
        case NODE_LESS:             printf("LESS\n");                                                            break;
        case NODE_LESS_OR_EQUAL:    printf("LESS OR EQUAL\n");                                                   break;
        case NODE_AND:              printf("AND\n");                                                             break;
        case NODE_OR:               printf("OR\n");                                                              break;
        case NODE_ASSIGNMENT:       printf("ASSIGNEMENT\n");                                                     break;
        case NODE_DROP:             printf("DROP\n");                                                            break;
        case NODE_DECL:             printf("DECL : name = %s, index = %d\n", node->value.str_val, node->index);  break;
        case NODE_REF:              printf("REF : name = %s, index = %d\n", node->value.str_val, node->index);   break;
    }
}

void syntactic_node_display_tree(const SyntacticNode* root, int depth, FILE* out_file)
{
    char* depth_indicator = malloc(sizeof(char) * (depth + (size_t) 1));
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
        fprintf(out_file, "%s|--", depth_indicator);
        syntactic_node_display_tree(root->children[i], depth + 3, out_file);
    }

    free(depth_indicator);
}

void syntactic_node_free(SyntacticNode* node)
{
    free(node->children);
    free(node);
}

void syntactic_node_free_tree(SyntacticNode* tree)
{
    for (int i = 0; i < tree->nb_children; i++)
    {
        syntactic_node_free_tree(tree->children[i]);
    }
    syntactic_node_free(tree);
}
