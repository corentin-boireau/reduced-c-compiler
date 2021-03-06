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
    node->stack_offset = NO_STACK_OFFSET;
    node->nb_var = 0;

    node->line = line;
    node->col = col;

    node->parent = NULL;
    node->children = NULL;
    node->nb_children = 0;
    node->flags = 0;

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

    assert(child->parent == NULL);
    child->parent = parent;
}

void syntactic_node_display(const SyntacticNode* node, FILE *out_file)
{
    switch (node->type)
    {
        case NODE_INVALID:              fprintf(out_file, "INVALID\n");                                                               break;
        case NODE_CONSTANT:                fprintf(out_file, "CONST : value = %d\n", node->value.int_val);                               break;
        case NODE_UNARY_MINUS:          fprintf(out_file, "UNARY_MINUS\n");                                                           break;
        case NODE_NEGATION:             fprintf(out_file, "NEGATION\n");                                                              break;
        case NODE_ADDRESS:              fprintf(out_file, "ADDRESS\n");                                                               break;
        case NODE_ADD:                  fprintf(out_file, "ADD\n");                                                                   break;
        case NODE_SUB:                  fprintf(out_file, "SUB\n");                                                                   break;
        case NODE_MUL:                  fprintf(out_file, "MUL\n");                                                                   break;
        case NODE_DIV:                  fprintf(out_file, "DIV\n");                                                                   break;
        case NODE_MOD:                  fprintf(out_file, "MOD\n");                                                                   break;
        case NODE_BLOCK:                fprintf(out_file, "BLOCK\n");                                                                 break;
        case NODE_SEQUENCE:             fprintf(out_file, "SEQUENCE\n");                                                              break;
        case NODE_PRINT:                fprintf(out_file, "PRINT\n");                                                                 break;
        case NODE_EQUAL:                fprintf(out_file, "EQUAL\n");                                                                 break;
        case NODE_NOT_EQUAL:            fprintf(out_file, "NOT EQUAL\n");                                                             break;
        case NODE_GREATER:              fprintf(out_file, "GREATER\n");                                                               break;
        case NODE_GREATER_OR_EQUAL:     fprintf(out_file, "GREATER OR EQUAL\n");                                                      break;
        case NODE_LESS:                 fprintf(out_file, "LESS\n");                                                                  break;
        case NODE_LESS_OR_EQUAL:        fprintf(out_file, "LESS OR EQUAL\n");                                                         break;
        case NODE_AND:                  fprintf(out_file, "AND\n");                                                                   break;
        case NODE_OR:                   fprintf(out_file, "OR\n");                                                                    break;
        case NODE_ASSIGNMENT:           fprintf(out_file, "ASSIGNEMENT\n");                                                           break;
        case NODE_DROP:                 fprintf(out_file, "DROP\n");                                                                  break;
        case NODE_DECL:                 fprintf(out_file, "DECL : name = %s, index = %d%s%s\n", node->value.str_val, node->stack_offset,
                                        (node->stack_offset == NO_STACK_OFFSET) ? "" : 
                                        syntactic_node_is_flag_set(node, GLOBAL_FLAG) ? " (global)" : " (local)",
                                        syntactic_node_is_flag_set(node, CONST_FLAG)  ? " (const)"  : "");                            break;
        case NODE_REF:                  fprintf(out_file, "REF : name = %s, index = %d%s%s\n", node->value.str_val, node->stack_offset,
                                        (node->stack_offset == NO_STACK_OFFSET) ? "" : 
                                        syntactic_node_is_flag_set(node, GLOBAL_FLAG) ? " (global)" : " (local)",
                                        syntactic_node_is_flag_set(node, CONST_FLAG)  ? " (const)"  : "");                            break;
        case NODE_CONDITION:            fprintf(out_file, "CONDITION\n");                                                             break;
        case NODE_INVERTED_CONDITION:   fprintf(out_file, "INVERTED CONDITION\n");                                                    break;
        case NODE_LOOP:                 fprintf(out_file, "LOOP\n");                                                                  break;
        case NODE_BREAK:                fprintf(out_file, "BREAK\n");                                                                 break;
        case NODE_CONTINUE:             fprintf(out_file, "CONTINUE\n");                                                              break;
        case NODE_CONTINUE_LABEL:       fprintf(out_file, "CONTINUE LABEL\n");                                                        break;
        case NODE_FUNCTION:             fprintf(out_file, "FUNCTION : name = %s\n", node->value.str_val);                             break;
        case NODE_PROGRAM:              fprintf(out_file, "PROGRAM\n");                                                               break;
        case NODE_CALL:                 fprintf(out_file, "CALL : name = %s\n", node->value.str_val);                                 break;
        case NODE_RETURN:               fprintf(out_file, "RETURN\n");                                                                break;
        case NODE_DEREF:                fprintf(out_file, "DEREF\n");                                                                 break;
        case NODE_COMPOUND:             fprintf(out_file, "COMPOUND\n");                                                              break;
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

    syntactic_node_display(root, out_file);
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
