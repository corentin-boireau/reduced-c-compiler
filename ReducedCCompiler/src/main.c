#include <stdio.h>
#include <stdlib.h>

#include "token.h"
#include "syntactic_node.h"
#include "syntactic_analysis.h"
#include "code_generation.h"

// Loads the source file content as a null terminated buffer allocated dynamically
// TO BE FREED
char* load_file_content(char* path); 

void test_lexical_analysis_on_file(char* path);
void test_display_syntactic_tree();
void test_syntactical_analysis_on_file(char* path);
void test_compile_file(char* path, int verbose);
int main()
{
    // char valid_filepath[] = "res/valid.c";
    // char junk_filepath[] = "res/junk.c";
    // char mix_filepath[] = "res/mix.c";
    // 
    // test_lexical_analysis_on_file(valid_filepath);
    // test_lexical_analysis_on_file(junk_filepath);
    // test_lexical_analysis_on_file(mix_filepath);
    
    // test_display_syntactic_tree();

    char simple_expression_path[] = "res/simple_expression.c";
    //test_syntactical_analysis_on_file(simple_expression_path);
    test_compile_file(simple_expression_path, 0);
    return 0;
}

void test_compile_file(char* path, int verbose)
{
    char* file_content = load_file_content(path);

    if(verbose) printf("File content :\n\n%s\n", file_content);

    SyntacticAnalyzer analyzer = syntactic_analyzer_create(file_content);

    if (syntactic_analyzer_build_tree(&analyzer) == NULL)
    {
        fprintf(stderr, "The source file \"%s\" is empty", path);
    }
    else
    {
        if (analyzer.nb_errors > 0)
        {
            fprintf(stderr, "%d error(s) found : compilation aborted", analyzer.nb_errors);
        }
        else
        {
            if (verbose)
            {
                printf("\nSyntactic tree : \n\n");
                syntactic_node_display_tree(analyzer.syntactic_tree, 0);
                printf("\nGenerated code :\n\n");
            }
            generate_program(analyzer.syntactic_tree);
        }
    }

    free(file_content);
}

void test_syntactical_analysis_on_file(char* path)
{
    char* file_content = load_file_content(path);

    printf("File content :\n\n%s\n\n", file_content);

    SyntacticAnalyzer analyzer = syntactic_analyzer_create(file_content);

    if (syntactic_analyzer_build_tree(&analyzer) == NULL)
    {
        fprintf(stderr, "The source file \"%s\" is empty", path);
    }
    else
    {   
        printf("\n\nSyntactic tree : \n\n");
        syntactic_node_display_tree(analyzer.syntactic_tree, 0);
    }

    free(file_content);
}

void test_display_syntactic_tree()
{
    SyntacticNode* negation_node = syntactic_node_create(NODE_NEGATION, 1, 1);
    SyntacticNode* address_node  = syntactic_node_create(NODE_ADDRESS, 1, 2);
    SyntacticNode* constant_node = syntactic_node_create_with_value(NODE_CONST, 1, 3, 1024);

    syntactic_node_add_child(negation_node, address_node);
    syntactic_node_add_child(negation_node, constant_node);

    syntactic_node_add_child(address_node, syntactic_node_create(NODE_INDIRECTION, 1, 1));
    syntactic_node_add_child(address_node, syntactic_node_create(NODE_NEGATION, 1, 1));

    syntactic_node_add_child(constant_node, syntactic_node_create(NODE_ADDRESS, 1, 1));
    syntactic_node_add_child(constant_node, syntactic_node_create(NODE_INDIRECTION, 1, 1));
    syntactic_node_add_child(constant_node, syntactic_node_create(NODE_NEGATION, 1, 1));
    syntactic_node_add_child(constant_node, syntactic_node_create(NODE_UNARY_MINUS, 1, 1));

    syntactic_node_display_tree(negation_node, 0);
}

void test_lexical_analysis_on_file(char* path)
{
    char* file_content = load_file_content(path);

    printf("File content :\n\n%s\n", file_content);
    printf("\nToken list : \n");
    Tokenizer tokenizer = tokenizer_create(file_content);
    while (tokenizer.next.type != TOK_EOF)
    {
        tokenizer_step(&tokenizer);
        token_display(tokenizer.current);
    }

    free(file_content);
}


char* load_file_content(char* path)
{
    FILE* src_file = fopen(path, "r");
    if (src_file == NULL)
    {
        perror("failed to open the source file");
        exit(EXIT_FAILURE);
    }

    fseek(src_file, 0, SEEK_END);
    size_t file_size = ftell(src_file);
    fseek(src_file, 0, SEEK_SET);

    char* file_content = malloc(sizeof(char) * file_size + 1);
    if (file_content == NULL)
    {
        perror("Failed to allocate the buffer for the file content");
        exit(EXIT_FAILURE);
    }

    if (fread(file_content, sizeof(char), file_size, src_file) != file_size)
    {
        perror("Failed to read the whole content of the source file");
        exit(EXIT_FAILURE);
    }
    file_content[file_size] = '\0';

    fclose(src_file);

    return file_content;
}