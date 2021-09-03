#include <stdio.h>
#include <stdlib.h>

#include "token.h"

// Loads the source file content as a null terminated buffer allocated dynamically
// TO BE FREED
char* load_file_content(char* path); 
void test_lexical_analysis_on_file(char* path);

int main(int argc, char **argv)
{
    char valid_filepath[] = "res/valid.c";
    char junk_filepath[] = "res/junk.c";
    char mix_filepath[] = "res/mix.c";
    
    test_lexical_analysis_on_file(valid_filepath);
    test_lexical_analysis_on_file(junk_filepath);
    test_lexical_analysis_on_file(mix_filepath);
    
    return 0;
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
        display_token(tokenizer.current);
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
    int file_size = ftell(src_file);
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
