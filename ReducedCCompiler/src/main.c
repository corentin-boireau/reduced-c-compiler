#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "token.h"
#include "syntactic_node.h"
#include "syntactic_analysis.h"
#include "semantic_analysis.h"
#include "code_generation.h"
#include "argtable3/argtable3.h"

#if defined(_WIN32) || defined(WIN32) 
    #include <io.h>
    #define F_OK 0
    #define W_OK 2
    #define R_OK 4
    #define ACCESS(pathname, mode) _access(pathname, mode)

    #define STAT(pathname, statbuf) _stat(pathname, statbuf)
    #define STAT_S _stat
#else
    #include <unistd.h>
    #define ACCESS(pathname, mode) access(pathname, mode)

    #define STAT(pathname, statbuf) stat(pathname, statbuf)
    #define STAT_S stat
#endif

#define STAGE_LEXICAL   "lexical"
#define STAGE_SYNTACTIC "syntactic"
#define STAGE_SEMANTIC  "semantic"

// Loads the source file content as a null terminated buffer allocated dynamically
// TO BE FREED
char* load_file_content(const char * path); 

void lexical_analysis_on_file(const char * path, int verbose, FILE* out_file);
void test_display_syntactic_tree();
void syntactical_analysis_on_file(const char * path, int verbose, unsigned char optimisations, FILE* out_file);
void semantic_analysis_on_file(char* path, unsigned char optimisations);
void compile_file(const char * path, int verbose, unsigned char optimisations, FILE* out_file);

/* global arg_xxx structs */
struct arg_lit *verb,  *help,  *version;
struct arg_file *output,  *input;
struct arg_str *stage;
struct arg_end *end;

int main(int argc, char* argv[])
{
    /* the global arg_xxx structs are initialised within the argtable */
    void* argtable[] =
    {
        help    = arg_litn("h", "help", 0, 1, "display this help and exit"),
        version = arg_litn(NULL, "version", 0, 1, "display version info and exit"),
        verb    = arg_litn("v", "verbose", 0, 1, "verbose output"),
        output  = arg_filen("o", "output", "file", 0, 1, "output file"),
        input    = arg_filen(NULL, NULL, "<file>", 1, 1, "input file"),
        stage   = arg_strn(NULL, "stage", "<lexical|syntactical|semantic>", 0, 1, "stop the compilation at this stage"),
        end     = arg_end(20),
    };

    int  exitcode = EXIT_SUCCESS;
    char progname[] = "rcc";

    int nb_errors;
    nb_errors = arg_parse(argc, argv, argtable);

    /* special case: '--help' takes precedence over error reporting */
    if (help->count > 0)
    {
        printf("Reduced C Compiler.\n\n");
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-40s %s\n");
    }
    else if (nb_errors > 0)
    {
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        exitcode = EXIT_FAILURE;
    }
    else
    {
        FILE *output_file = stdout;

        if (ACCESS(*(input->filename), R_OK) == -1)
        {
            // TODO ACCESS() doesn't fail on windows when an asked permission is denied
            if (errno == EACCES)
            {
                fprintf(stderr, "%s : Permission denied\n", *(input->filename));
                exitcode = EXIT_FAILURE;
            }
            else if (errno == ENOENT)
            {
                fprintf(stderr, "%s : No such file or directory\n", *(input->filename));
                exitcode = EXIT_FAILURE;
            }
        }
        else
        {
            if (output->count > 0)
            {
                output_file = fopen(*(output->filename), "w");
                if (output_file == NULL)
                {
                    fprintf(stderr, "%s : Failed to open the output file\n", *(input->filename));
                    exitcode = EXIT_FAILURE;
                }
            }

            if (output_file != NULL)
            {
                // TODO activate/deactivate opti from cli
                unsigned char opti = OPTI_CONST_OPERATIONS;
                // Stage handling
                if (stage->count == 0)
                {
                    compile_file(*(input->filename), verb->count, opti, output_file);
                }
                else
                {
                    if (strcmp(*(stage->sval), STAGE_LEXICAL) == 0)
                    {
                        lexical_analysis_on_file(*(input->filename), verb->count, output_file);
                    }
                    else if (strcmp(*(stage->sval), STAGE_SYNTACTIC) == 0)
                    {
                    }
                    else if (strcmp(*(stage->sval), STAGE_SEMANTIC) == 0)
                    {
                    }
                    else
                    {
                    }
                }
            }
        }
    }

    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    return exitcode;
}

void compile_file(const char * path, int verbose, unsigned char optimisations, FILE* out_file)
{
    char* file_content = load_file_content(path);

    if(verbose)
        printf("File content :\n\n%s\n", file_content);

    SyntacticAnalyzer analyzer = syntactic_analyzer_create(file_content, optimisations);

    if (syntactic_analyzer_build_tree(&analyzer) == NULL)
    {
        fprintf(stderr, "The source file \"%s\" is empty", path);
    }
    else
    {
        if (analyzer.nb_errors > 0)
        {
            fprintf(stderr, "%d error(s) found during syntactical analysis : compilation aborted", analyzer.nb_errors);
        }
        else
        {
            
            if (verbose)
            {
                printf("\nSyntactic tree before semantic analysis : \n\n");
                syntactic_node_display_tree(analyzer.syntactic_tree, 0, out_file);
            }

            SymbolTable table = symbol_table_create();
            semantic_analysis(analyzer.syntactic_tree, &table);

            if (verbose)
            {
                printf("\nSyntactic tree after semantic analysis : \n\n");
                syntactic_node_display_tree(analyzer.syntactic_tree, 0, out_file);
            }
            if (table.nb_errors > 0)
            {
                fprintf(stderr, "%d error(s) found during semantic analysis : compilation aborted", table.nb_errors);
            }
            else
            {
                if (verbose)
                {
                    printf("\nGenerated code :\n\n");
                }
                generate_program(analyzer.syntactic_tree, table.nb_variables, out_file);
            }
        }
    }

    free(file_content);
}

void syntactical_analysis_on_file(const char * path, int verbose, unsigned char optimisations, FILE* out_file)
{
    char* file_content = load_file_content(path);

    if (verbose)
        printf("File content :\n\n%s\n\n", file_content);

    SyntacticAnalyzer analyzer = syntactic_analyzer_create(file_content, optimisations);

    if (syntactic_analyzer_build_tree(&analyzer) == NULL)
    {
        fprintf(stderr, "The source file \"%s\" is empty", path);
    }
    else
    {   
        if (verbose)
            printf("\n\nSyntactic tree : \n\n");
        syntactic_node_display_tree(analyzer.syntactic_tree, 0, out_file);
    }

    free(file_content);
}
void semantic_analysis_on_file(char* path, unsigned char optimisations)
{
    char* file_content = load_file_content(path);

    printf("File content :\n\n%s\n\n", file_content);

    SyntacticAnalyzer analyzer = syntactic_analyzer_create(file_content, optimisations);

    if (syntactic_analyzer_build_tree(&analyzer) == NULL)
    {
        fprintf(stderr, "The source file \"%s\" is empty", path);
    }
    else
    {   
        printf("\n\nSyntactic tree before : \n\n");
        syntactic_node_display_tree(analyzer.syntactic_tree, 0, stdout);

        SymbolTable table = symbol_table_create();
        semantic_analysis(analyzer.syntactic_tree, &table);

        printf("\n\nSyntactic tree after : \n\n");
        syntactic_node_display_tree(analyzer.syntactic_tree, 0, stdout);
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

    syntactic_node_display_tree(negation_node, 0, stdout);
}

void lexical_analysis_on_file(const char * path, int verbose, FILE* out_file)
{
    char* file_content = load_file_content(path);

    if (verbose)
    {
        printf("File content :\n\n%s\n", file_content);
        printf("\nToken list : \n");
    }
    Tokenizer tokenizer = tokenizer_create(file_content);
    while (tokenizer.next.type != TOK_EOF)
    {
        tokenizer_step(&tokenizer);
        token_display(tokenizer.current, out_file);
    }

    free(file_content);
}

char* load_file_content(const char * path)
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

    size_t nb_char_loaded = fread(file_content, sizeof(char), file_size, src_file); 
    // May be different from file_size, e.g. if line breaks are CRLF instead of LF

    if (ferror(src_file))
    {
        fprintf(stderr, "Failed to read content from the source file \"%s\"\n", path);
        exit(EXIT_FAILURE);
    }

    file_content[nb_char_loaded] = '\0';

    fclose(src_file);

    return file_content;
}