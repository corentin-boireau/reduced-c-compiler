#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include "argtable3/argtable3.h"

#include "token.h"
#include "syntactic_node.h"
#include "syntactic_analysis.h"
#include "semantic_analysis.h"
#include "code_generation.h"
#include "optimization.h"


#define RCC_NAME            "rcc"
#define RCC_LONG_NAME       "Reduced C Compiler"
#define RCC_VERSION         "0.1"
#define RCC_RUNTIME_ENV_VAR "RCC_RUNTIME"

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

// Loads the source file content as a null terminated buffer allocated dynamically and closes the FILE*
char* load_file_content_and_close(FILE * file);

void lexical_analysis_on_file(FILE* in_file, int verbose, FILE* out_file);
void syntactic_analysis_on_file(FILE* in_file, int verbose, unsigned char optimisations, FILE* out_file);
void semantic_analysis_on_file(FILE* in_file, int verbose, unsigned char optimisations, FILE* out_file, FILE * runtime_file);
void compile_file(FILE* in_file, int verbose, unsigned char optimisations, FILE* out_file, FILE * runtime_file);


/* global arg_xxx structs */
struct arg_lit *verb, *help, *version, *no_runtime;
struct arg_file *output, *input, *runtime_filename;
struct arg_str *stage;
struct arg_lit *no_const_fold;
struct arg_end *end;

int main(int argc, char* argv[])
{
    /* the global arg_xxx structs are initialised within the argtable */
    void* argtable[] =
    {
        input            = arg_filen(NULL, NULL,      "<file>",                         1, 1, "input file"),
        output           = arg_filen( "o", "output",  "<file>",                         0, 1, "output file"),
        verb             = arg_litn(  "v", "verbose",                                   0, 1, "verbose output"),
        no_runtime       = arg_litn( NULL, "no-runtime",                                0, 1, "no runtime"),
        runtime_filename = arg_filen(NULL, "runtime", "<file>",                         0, 1, "runtime file, default to environnment variable RCC_RUNTIME"),
        stage            = arg_strn( NULL, "stage",   "<lexical|syntactical|semantic>", 0, 1, "stop the compilation at this stage"),
        no_const_fold    = arg_litn( NULL, "no-const-fold",                             0, 1, "disable constant folding"),
        help             = arg_litn(  "h", "help",                                      0, 1, "display this help and exit"),
        version          = arg_litn( NULL, "version",                                   0, 1, "display version info and exit"),
        end              = arg_end(20),
    };
    int exitcode = EXIT_SUCCESS;

    int nb_errors;
    nb_errors = arg_parse(argc, argv, argtable);

    /* special case: '--help' takes precedence over error reporting */
    if (help->count > 0)
    {
        printf("%s.\n\n", RCC_LONG_NAME);
        printf("Usage: %s", RCC_NAME);
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-40s %s\n");
        exit(EXIT_SUCCESS);
    }
    else if (version->count > 0)
    {
        printf("%s %s\n", RCC_LONG_NAME, RCC_VERSION);
        exit(EXIT_SUCCESS);
    }
    else if (nb_errors > 0)
    {
        /* Display the error details contained in the arg_end struct. */
        arg_print_errors(stdout, end, RCC_NAME);
        printf("Try '%s --help' for more information.\n", RCC_NAME);
        exit(EXIT_FAILURE);
    }

    FILE* source_file = NULL;
    FILE* output_file = stdout;

    if (ACCESS(*(input->filename), R_OK) == -1)
    {
        // TODO ACCESS() doesn't fail on windows when an asked permission is denied
        if (errno == EACCES)
        {
            fprintf(stderr, "%s: error. %s : Permission denied\n", RCC_NAME, *(input->filename));
            exit(EXIT_FAILURE);
        }
        else if (errno == ENOENT)
        {
            fprintf(stderr, "%s: error. %s : No such file or directory\n", RCC_NAME, *(input->filename));
            exit(EXIT_FAILURE);
        }
    }

    source_file = fopen(*(input->filename), "r");
    if (source_file == NULL)
    {
        perror("failed to open the source file");
        exit(EXIT_FAILURE);
    }

    if (output->count > 0)
    {
        output_file = fopen(*(output->filename), "w");
        if (output_file == NULL)
        {
            fprintf(stderr, "%s: error. Failed to open the output file \"%s\"\n", RCC_NAME, *(output->filename));
            exit(EXIT_FAILURE);
        }
    }

    optimization_t opti = NO_OPTIMIZATION;
    if (no_const_fold->count == 0)
        opti |= OPTI_CONST_FOLD;

    FILE* runtime_file = NULL;
    if (no_runtime->count > 0)
    {
        if (runtime_filename->count > 0)
        {
            fprintf(stderr, "%s: invalid option. \"--%s\" option is incompatible with \"--%s\" option.\n",
                    RCC_NAME, runtime_filename->hdr.longopts, no_runtime->hdr.longopts);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        char* runtime_path = NULL;
        if (runtime_filename->count > 0)
            runtime_path = *( (char**) (runtime_filename)->filename);
        else
            runtime_path = getenv(RCC_RUNTIME_ENV_VAR);

        if (runtime_path != NULL)
            runtime_file = fopen(runtime_path, "r");
        else
        {
            fprintf(stderr, "%s: runtime path not provided. Please define the environnment variable %s.\nOtherwise use one of these options --runtime <file> or --no-runtime.\n",
                    RCC_NAME, RCC_RUNTIME_ENV_VAR);
            exit(EXIT_FAILURE);
        }


        if (runtime_file == NULL)
        {
            fprintf(stderr, "%s: Failed to open the runtime file : %s\n", RCC_NAME, runtime_path);
            exit(EXIT_FAILURE);
        }
    }

    // Stage handling
    if (stage->count == 0)
    {
        compile_file(source_file, verb->count, opti, output_file, runtime_file);
    }
    else
    {
        if (strcmp(*(stage->sval), STAGE_LEXICAL) == 0)
        {
            lexical_analysis_on_file(source_file, verb->count, output_file);
        }
        else if (strcmp(*(stage->sval), STAGE_SYNTACTIC) == 0)
        {
            syntactic_analysis_on_file(source_file, verb->count, opti, output_file);
        }
        else if (strcmp(*(stage->sval), STAGE_SEMANTIC) == 0)
        {
            semantic_analysis_on_file(source_file, verb->count, opti, output_file, runtime_file);
        }
        else
        {
            fprintf(stderr, "%s: invalid option. Unknown stage \"%s\"\nValid stages : \"%s\", \"%s\", \"%s\"",
                RCC_NAME, *(stage->sval), STAGE_LEXICAL, STAGE_SYNTACTIC, STAGE_SEMANTIC);
            exit(EXIT_FAILURE);
        }
    }

    fclose(output_file);
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    return exitcode;
}

void compile_file(FILE* in_file, int verbose, unsigned char optimisations, FILE* out_file, FILE* runtime_file)
{
    SymbolTable table = symbol_table_create();

    // ** Runtime ** //
    SyntacticAnalyzer runtime_analyzer = {0};

    if (runtime_file != NULL)
    {
        char* runtime_content = load_file_content_and_close(runtime_file);
        runtime_analyzer = syntactic_analyzer_create(runtime_content, optimisations);
        syntactic_analyzer_build_tree(&runtime_analyzer);
        assert(runtime_analyzer.syntactic_tree != NULL);
        assert(runtime_analyzer.nb_errors == 0);

        free(runtime_content);

        semantic_analysis(runtime_analyzer.syntactic_tree, &table);
        assert(table.nb_errors == 0);
    }
    // ************ //

    char* usercode_content = load_file_content_and_close(in_file);

    if(verbose)
        printf("File content :\n\n%s\n", usercode_content);

    SyntacticAnalyzer usercode_analyzer = syntactic_analyzer_create(usercode_content, optimisations);
    syntactic_analyzer_build_tree(&usercode_analyzer);
    free(usercode_content);

    if (usercode_analyzer.syntactic_tree == NULL)
    {
        fprintf(stderr, "%s: error. The source file is empty\n", RCC_NAME);
    }
    else
    {
        if (usercode_analyzer.nb_errors > 0)
        {
            if(usercode_analyzer.nb_errors == 1)
                fprintf(stderr, "%s: error. 1 error found during syntactical analysis : compilation aborted\n", RCC_NAME);
            else
                fprintf(stderr, "%s: error. %d errors found during syntactical analysis : compilation aborted\n", RCC_NAME, usercode_analyzer.nb_errors);

            exit(EXIT_FAILURE);
        }
        else
        {
            if (verbose)
            {
                printf("\nSyntactic tree before semantic analysis : \n\n");
                syntactic_node_display_tree(usercode_analyzer.syntactic_tree, 0, out_file);
            }

            semantic_analysis(usercode_analyzer.syntactic_tree, &table);

            if (verbose)
            {
                printf("\nSyntactic tree after semantic analysis : \n\n");
                syntactic_node_display_tree(usercode_analyzer.syntactic_tree, 0, out_file);
            }
            if (table.nb_errors > 0)
            {
                if(table.nb_errors == 1)
                    fprintf(stderr, "%s: error. 1 error found during semantic analysis : compilation aborted\n", RCC_NAME);
                else
                    fprintf(stderr, "%s: error. %d errors found during semantic analysis : compilation aborted\n", RCC_NAME, table.nb_errors);

                exit(EXIT_FAILURE);
            }
            else
            {
                if (verbose)
                    printf("\nGenerated code :\n\n");

                SyntacticNode** global_declarations = malloc(table.nb_glob_variables * sizeof(SyntacticNode*));
                if (global_declarations == NULL)
                {
                    perror("Failed to allocate memory for the global_declarations array");
                    exit(EXIT_FAILURE);
                }

                if(runtime_file != NULL)
                    generate_code(runtime_analyzer.syntactic_tree, out_file, NO_LOOP, table.nb_glob_variables, global_declarations);

                generate_program(usercode_analyzer.syntactic_tree, out_file, no_runtime->count == 0, table.nb_glob_variables, global_declarations);
            }
        }
    }
}

void syntactic_analysis_on_file(FILE* in_file, int verbose, unsigned char optimisations, FILE* out_file)
{
    char* usercode_content = load_file_content_and_close(in_file);

    if (verbose)
        fprintf(out_file, "File content :\n\n%s\n\n", usercode_content);

    SyntacticAnalyzer usercode_analyzer = syntactic_analyzer_create(usercode_content, optimisations);
    syntactic_analyzer_build_tree(&usercode_analyzer);
    free(usercode_content);

    if (usercode_analyzer.syntactic_tree == NULL)
    {
        fprintf(stderr, "%s: error. The source file is empty\n", RCC_NAME);
    }
    else
    {
        if (verbose)
            fprintf(out_file, "\n\nSyntactic tree : \n\n");

        syntactic_node_display_tree(usercode_analyzer.syntactic_tree, 0, out_file);
    }
}

void semantic_analysis_on_file(FILE* in_file, int verbose, unsigned char optimisations, FILE* out_file, FILE * runtime_file)
{
    SymbolTable table = symbol_table_create();

    // ** Runtime ** //
    SyntacticAnalyzer runtime_analyzer = { 0 };

    if (runtime_file != NULL)
    {
        char* runtime_content = load_file_content_and_close(runtime_file);
        runtime_analyzer = syntactic_analyzer_create(runtime_content, optimisations);
        syntactic_analyzer_build_tree(&runtime_analyzer);
        assert(runtime_analyzer.syntactic_tree != NULL);
        assert(runtime_analyzer.nb_errors == 0);

        free(runtime_content);

        semantic_analysis(runtime_analyzer.syntactic_tree, &table);
        assert(table.nb_errors == 0);
    }
    // ************ //

    char* usercode_content = load_file_content_and_close(in_file);

    if (verbose)
        printf("File content :\n\n%s\n", usercode_content);

    SyntacticAnalyzer usercode_analyzer = syntactic_analyzer_create(usercode_content, optimisations);
    syntactic_analyzer_build_tree(&usercode_analyzer);
    free(usercode_content);

    if (usercode_analyzer.syntactic_tree == NULL)
    {
        fprintf(stderr, "%s: error. The source file is empty\n", RCC_NAME);
    }
    else
    {
        if (verbose)
        {
            fprintf(out_file, "\n\nSyntactic tree before : \n\n");
            syntactic_node_display_tree(usercode_analyzer.syntactic_tree, 0, out_file);
        }

        semantic_analysis(usercode_analyzer.syntactic_tree, &table);

        if (verbose)
            fprintf(out_file, "\n\nSyntactic tree after : \n\n");

        syntactic_node_display_tree(usercode_analyzer.syntactic_tree, 0, out_file);
    }
}

void lexical_analysis_on_file(FILE* in_file, int verbose, FILE* out_file)
{
    char* usercode_content = load_file_content_and_close(in_file);

    if (verbose)
    {
        printf("File content :\n\n%s\n", usercode_content);
        printf("\nToken list : \n");
    }
    Tokenizer tokenizer = tokenizer_create(usercode_content);
    while (tokenizer.next.type != TOK_EOF)
    {
        tokenizer_step(&tokenizer);
        token_display(tokenizer.current, out_file);
    }

    free(usercode_content);
}

char* load_file_content_and_close(FILE* file)
{
    size_t file_size;
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* file_content = malloc(sizeof(char) * file_size + 1);
    if (file_content == NULL)
    {
        perror("Failed to allocate the buffer for the file content");
        exit(EXIT_FAILURE);
    }
    else
    {
        size_t nb_char_loaded = fread(file_content, sizeof(char), file_size, file);
        // May be different from file_size, e.g. if line breaks are CRLF instead of LF

        if (ferror(file))
        {
            fprintf(stderr, "%s: error. Failed to read content from the file\n", RCC_NAME);
            exit(EXIT_FAILURE);
        }
        fclose(file);

        file_content[nb_char_loaded] = '\0';
    }

    return file_content;
}