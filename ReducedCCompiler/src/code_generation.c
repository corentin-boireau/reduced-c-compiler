#include "code_generation.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


void generate_program(const SyntacticNode* program, FILE * stream, int is_init_called)
{
    assert(program != NULL);
    
    #define RUNTIME_CODE \
        "       prep main"   "\n" \
        "       call 0"      "\n" \
        "       halt"        "\n" \
                             "\n" \
        ".putchar"           "\n" \
        "       send"        "\n" \
        "       push 0"      "\n" \
        "       ret"         "\n" \
                             "\n" \
        ".getchar"           "\n" \
        "       recv"        "\n" \
        "       ret"         \

    char asm_runtime[] =
        ".start"             "\n"
        RUNTIME_CODE         "\n"
        ;

    char asm_runtime_with_init[] =
        ".start"             "\n"
        "       prep _Init"	 "\n"
        "       call 0"		 "\n"
        RUNTIME_CODE         "\n"
        ;

    generate_code(program, stream, NO_LOOP);

    fprintf(stream, "%s\n", is_init_called ? asm_runtime_with_init : asm_runtime);
}

void generate_code(const SyntacticNode* node, FILE * stream, int loop_nb)
{
    assert(node != NULL);

    static int label_counter = 0;

    switch (node->type)
    {
        case NODE_NEGATION:
        {
            generate_code(node->children[0], stream, loop_nb);
            fprintf(stream, "        not\n");
            break;
        }
        case NODE_UNARY_MINUS:
        {
            fprintf(stream, "        push 0\n");
            generate_code(node->children[0], stream, loop_nb);
            fprintf(stream, "        sub\n");
            break;
        }
        case NODE_ADD :
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        add\n");
            break;
        }
        case NODE_SUB:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        sub\n");
            break;
        }
        case NODE_MUL:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        mul\n");
            break;
        }
        case NODE_DIV:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        div\n");
            break;
        }
        case NODE_MOD:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        mod\n");
            break;
        }
        case NODE_AND:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        and\n");
            break;
        }
        case NODE_OR:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        or\n");
            break;
        }
        case NODE_EQUAL:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        cmpeq\n");
            break;
        }
        case NODE_NOT_EQUAL:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        cmpne\n");
            break;
        }
        case NODE_LESS:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        cmplt\n");
            break;
        }
        case NODE_LESS_OR_EQUAL:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        cmple\n");
            break;
        }
        case NODE_GREATER:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        cmpgt\n");
            break;
        }
        case NODE_GREATER_OR_EQUAL:
        {
            generate_code(node->children[0], stream, loop_nb);
            generate_code(node->children[1], stream, loop_nb);
            fprintf(stream, "        cmpge\n");
            break;
        }
        case NODE_PRINT:
        {
            generate_code(node->children[0], stream, loop_nb);
            fprintf(stream, "        dbg\n");
            break;
        }
        case NODE_PROGRAM:
        case NODE_SEQUENCE:
        case NODE_BLOCK:
        {
            for (int i = 0; i < node->nb_children; i++)
            {
                generate_code(node->children[i], stream, loop_nb);
            }
            break;
        }
        case NODE_DROP:
        {
            generate_code(node->children[0], stream, loop_nb);
            fprintf(stream, "        drop\n");
            break;
        }
        case NODE_DECL :
            break; 
        case NODE_REF:
        {
            fprintf(stream, "        get %d\n", node->stack_offset);
            break;
        }
        case NODE_ASSIGNMENT :
        {
            assert(node->nb_children == 2);

            SyntacticNode* assignable = node->children[0];
            if (assignable->type == NODE_REF)
            {
                generate_code(node->children[1], stream, loop_nb);
                fprintf(stream, "        dup\n");
                fprintf(stream, "        set %d\n", assignable->stack_offset);
            }
            else if (assignable->type == NODE_DEREF)
            {
                assert(assignable->nb_children == 1);

                generate_code(node->children[1], stream, loop_nb);
                fprintf(stream, "        dup\n");
                generate_code(assignable->children[0], stream, loop_nb);
                fprintf(stream, "        write\n");
            }
            else
            {
                fprintf(stderr, "%d:%d error : Left operand of assignement must be a variable reference\n", node->line, node->col);
                exit(EXIT_FAILURE);
            }
            break;
        }
        case NODE_CONDITION:
        {
            int has_else = (node->nb_children == 3);
            int label_number = label_counter++;
            generate_code(node->children[0], stream, loop_nb);
            fprintf(stream, has_else ? "        jumpf else_%d\n"
                                     : "        jumpf endif_%d\n", label_number);
            generate_code(node->children[1], stream, loop_nb);
            if (has_else)
            {
                fprintf(stream, "        jump endif_%d\n", label_number);
                fprintf(stream, ".else_%d\n", label_number);
                generate_code(node->children[2], stream, loop_nb);
            }
            fprintf(stream, ".endif_%d\n", label_number);
            break;
        }
        case NODE_INVERTED_CONDITION:
        {
            int has_else = (node->nb_children == 3);
            int label_number = label_counter++;
            generate_code(node->children[0], stream, loop_nb);
            fprintf(stream, has_else ? "        jumpt else_%d\n"
                                     : "        jumpt endif_%d\n", label_number);
            generate_code(node->children[1], stream, loop_nb);
            if (has_else)
            {
                fprintf(stream, "        jump endif_%d\n", label_number);
                fprintf(stream, ".else_%d\n", label_number);
                generate_code(node->children[2], stream, loop_nb);
            }
            fprintf(stream, ".endif_%d\n", label_number);
            break;
        }
        case NODE_LOOP:
        {
            int current_loop_number = label_counter++;
            fprintf(stream, ".loop_%d\n", current_loop_number);
            for (int i = 0; i < node->nb_children; i++)
            {
                generate_code(node->children[i], stream, current_loop_number);
            }
            fprintf(stream, "        jump loop_%d\n", current_loop_number);
            fprintf(stream, ".endloop_%d\n", current_loop_number);
            break;
        }
        case NODE_FUNCTION:
        {
            fprintf(stream, ".%s\n", node->value.str_val);
            fprintf(stream, "        resn %d\n", node->nb_var);
            for (int i = 0; i < node->nb_children; i++)
            {
                generate_code(node->children[i], stream, loop_nb);
            }
            fprintf(stream, "        push 0\n");
            fprintf(stream, "        ret\n");
            break;
        }
        case NODE_CALL:
        {
            assert(node->nb_children == 1 && node->children[0]->type == NODE_SEQUENCE);

            fprintf(stream, "        prep %s\n", node->value.str_val);
            for (int i = 0; i < node->nb_children; i++)
            {
                generate_code(node->children[i], stream, loop_nb);
            }
            fprintf(stream, "        call %d\n", node->children[0]->nb_children);
            break;
        }
        case NODE_RETURN:
        {
            int has_retval = (node->nb_children > 0);
            if (has_retval)
            {
                generate_code(node->children[0], stream, loop_nb);
            }
            else
            {
                fprintf(stream, "        push 0\n");
            }
            fprintf(stream, "        ret\n");

            break;
        }
        case NODE_CONTINUE:
        {
            fprintf(stream, "        jump loop_%d\n", loop_nb);
            break;
        }
        case NODE_BREAK:
        {
            fprintf(stream, "        jump endloop_%d\n", loop_nb);
            break;
        }
        case NODE_DEREF:
        {
            assert(node->nb_children == 1);
            generate_code(node->children[0], stream, loop_nb);
            fprintf(stream, "        read\n");
            break;
        }
        case NODE_CONST: fprintf(stream, "        push %d\n", node->value.int_val); break;
    }
}
