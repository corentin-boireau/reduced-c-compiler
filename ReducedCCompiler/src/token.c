#include "token.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Token token_create() 
{
    Token token;
    token.type = TOK_NONE;

    return token;
}
Tokenizer tokenizer_create(char* buff)
{
    assert(buff != NULL);

    Tokenizer tokenizer;
    tokenizer.buff    = buff;
    tokenizer.pos     = 0;
    tokenizer.line    = 1;
    tokenizer.col     = 1;
    tokenizer.current = token_create();
    tokenizer.next    = token_create();

    return tokenizer;
}

void set_next(Tokenizer* tokenizer, int type)
{
    assert(tokenizer != NULL);

    tokenizer->next.type = type;
    tokenizer->next.line = tokenizer->line;
    tokenizer->next.col = tokenizer->col;
}

void token_error(Tokenizer* tokenizer)
{
    assert(tokenizer != NULL);

    fprintf(stderr, "Invalid token '%c' at %d:%d", tokenizer->buff[tokenizer->pos + 1], tokenizer->line, tokenizer->col);
    exit(EXIT_FAILURE);
}

static inline int is_numeric(char c)
{
    return c >= '0' && c <= '9';
}

static inline int is_letter(char c)
{
    return (c >= 'A' && c <= 'Z')
        || (c >= 'a' && c <= 'z')
        || c == '_';
}

static inline int is_alphanumeric(char c)
{
    return is_letter(c) || is_numeric(c);
}

void tokenizer_step(Tokenizer* tokenizer)
{
    assert(tokenizer != NULL);

    // str_val is dynamically allocated when a token requiring a string is created so it must be freed before overwriting tokenizer.current
    // str_val is not freed for TOK_IDENTIFIER because the pointer is given to the syntactic node (NODE_DECL or NODE_REF)
    if (tokenizer->current.type == TOK_INVALID_SEQ)
        free(tokenizer->current.value.str_val);

    tokenizer->current = tokenizer->next;
    int found = 0;
    while ( ! found)
    {
        switch (tokenizer->buff[tokenizer->pos])
        {
            case '\n':
            {
                tokenizer->line++;
                tokenizer->col = 1;
                tokenizer->pos++;
                break;
            }
            case ' ':
            case '\t':
            case '\r':
            {
                tokenizer->col++;
                tokenizer->pos++;
                break;
            }
            case '&' :
            {
                found = 1;
                if (tokenizer->buff[tokenizer->pos + 1] == '&')
                {
                    set_next(tokenizer, TOK_2_AMPERSAND);
                    tokenizer->col += 2;
                    tokenizer->pos += 2;
                }
                else
                {
                    set_next(tokenizer, TOK_AMPERSAND);
                    tokenizer->col++;
                    tokenizer->pos++;
                }
                break;
            }
            case '=':
            {
                found = 1;
                if (tokenizer->buff[tokenizer->pos + 1] == '=')
                {
                    set_next(tokenizer, TOK_2_EQUAL);
                    tokenizer->col += 2;
                    tokenizer->pos += 2;
                }
                else
                {
                    set_next(tokenizer, TOK_EQUAL);
                    tokenizer->col++;
                    tokenizer->pos++;
                }
                break;
            }
            case '!':
            {
                found = 1;
                if (tokenizer->buff[tokenizer->pos + 1] == '=')
                {
                    set_next(tokenizer, TOK_NOT_EQUAL);
                    tokenizer->col += 2;
                    tokenizer->pos += 2;
                }
                else
                {
                    set_next(tokenizer, TOK_NOT);
                    tokenizer->col++;
                    tokenizer->pos++;
                }
                break;
            }
            case '<':
            {
                found = 1;
                if (tokenizer->buff[tokenizer->pos + 1] == '=')
                {
                    set_next(tokenizer, TOK_LESS_OR_EQUAL);
                    tokenizer->col += 2;
                    tokenizer->pos += 2;
                }
                else
                {
                    set_next(tokenizer, TOK_LESS);
                    tokenizer->col++;
                    tokenizer->pos++;
                }
                break;
            }
            case '>':
            {
                found = 1;
                if (tokenizer->buff[tokenizer->pos + 1] == '=')
                {
                    set_next(tokenizer, TOK_GREATER_OR_EQUAL);
                    tokenizer->col += 2;
                    tokenizer->pos += 2;
                }
                else
                {
                    set_next(tokenizer, TOK_GREATER);
                    tokenizer->col++;
                    tokenizer->pos++;
                }
                break;
            }
            case '|':
            {
                found = 1;
                if (tokenizer->buff[tokenizer->pos + 1] == '|')
                {
                    set_next(tokenizer, TOK_2_PIPE);
                    tokenizer->col += 2;
                    tokenizer->pos += 2;
                }
                else
                {
                    char* invalid_sequence = malloc(sizeof(char) * 2);
                    if (invalid_sequence == NULL)
                    {
                        perror("Failed to allocate memory for an invalid_sequence of an invalid token");
                        exit(EXIT_FAILURE);
                    }
                    invalid_sequence[0] = '|';
                    invalid_sequence[1] = '\0';

                    // Creating the invalid sequence token
                    tokenizer->next.type = TOK_INVALID_SEQ;
                    tokenizer->next.value.str_val = invalid_sequence;
                    tokenizer->next.line = tokenizer->line;
                    tokenizer->next.col = tokenizer->col;

                    tokenizer->col++;
                    tokenizer->pos++;
                }
                break;
            }

            case '+' : set_next(tokenizer, TOK_PLUS);			   found = 1; tokenizer->col++; tokenizer->pos++; break;
            case '-' : set_next(tokenizer, TOK_MINUS);			   found = 1; tokenizer->col++; tokenizer->pos++; break;
            case '*' : set_next(tokenizer, TOK_STAR);			   found = 1; tokenizer->col++; tokenizer->pos++; break;
            case '/' : set_next(tokenizer, TOK_SLASH);			   found = 1; tokenizer->col++; tokenizer->pos++; break;
            case '%' : set_next(tokenizer, TOK_PERCENT);		   found = 1; tokenizer->col++; tokenizer->pos++; break;

            case ',' : set_next(tokenizer, TOK_COMMA);			   found = 1; tokenizer->col++; tokenizer->pos++; break;
            case ';' : set_next(tokenizer, TOK_SEMICOLON);		   found = 1; tokenizer->col++; tokenizer->pos++; break;
            case '(' : set_next(tokenizer, TOK_OPEN_PARENTHESIS);  found = 1; tokenizer->col++; tokenizer->pos++; break;
            case ')' : set_next(tokenizer, TOK_CLOSE_PARENTHESIS); found = 1; tokenizer->col++; tokenizer->pos++; break;
            case '[' : set_next(tokenizer, TOK_OPEN_BRACKET);	   found = 1; tokenizer->col++; tokenizer->pos++; break;
            case ']' : set_next(tokenizer, TOK_CLOSE_BRACKET);	   found = 1; tokenizer->col++; tokenizer->pos++; break;
            case '{' : set_next(tokenizer, TOK_OPEN_BRACE);		   found = 1; tokenizer->col++; tokenizer->pos++; break;
            case '}' : set_next(tokenizer, TOK_CLOSE_BRACE);	   found = 1; tokenizer->col++; tokenizer->pos++; break;

            case '\0': set_next(tokenizer, TOK_EOF);			   found = 1; tokenizer->col++; tokenizer->pos++; break;

            default  :
            {
                found = 1;
                if (is_numeric(tokenizer->buff[tokenizer->pos])) // looking for a constant token
                {
                    int size = 0;
                    while (is_numeric(tokenizer->buff[tokenizer->pos + size]))
                        size++;

                    if (is_letter(tokenizer->buff[tokenizer->pos + size])) // invalid sequence found
                    {
                        while (is_alphanumeric(tokenizer->buff[tokenizer->pos + size]))
                            size++;

                        char* invalid_sequence = malloc(sizeof(char) * size + 1);
                        if (invalid_sequence == NULL)
                        {
                            perror("Failed to allocate memory for an invalid_sequence of an invalid token");
                            exit(EXIT_FAILURE);
                        }
                        memcpy(invalid_sequence, &(tokenizer->buff[tokenizer->pos]), sizeof(char)* size);
                        invalid_sequence[size] = '\0';

                        // Creating the invalid sequence token
                        tokenizer->next.type = TOK_INVALID_SEQ;
                        tokenizer->next.value.str_val = invalid_sequence;
                        tokenizer->next.line = tokenizer->line;
                        tokenizer->next.col = tokenizer->col;

                    }
                    else // it is in fact a constant token
                    {

                        char* ascii_value = malloc(sizeof(char) * size + 1);
                        if (ascii_value == NULL)
                        {
                            perror("Failed to allocate memory for a constant ascii value");
                            exit(EXIT_FAILURE);
                        }
                        memcpy(ascii_value, &(tokenizer->buff[tokenizer->pos]), sizeof(char)* size);
                        ascii_value[size] = '\0';

                        // Creating the constant token
                        tokenizer->next.type = TOK_CONST;
                        tokenizer->next.value.int_val = atoi(ascii_value);
                        tokenizer->next.line = tokenizer->line;
                        tokenizer->next.col = tokenizer->col;

                        free(ascii_value);
                    }

                    tokenizer->pos += size;
                    tokenizer->col += size;
                }
                else if (is_letter(tokenizer->buff[tokenizer->pos])) // looking for an identifier or keyword
                {
                    int size = 0;
                    while (is_alphanumeric(tokenizer->buff[tokenizer->pos + size]))
                        size++;

                    char* text = malloc(sizeof(char) * size + 1);
                    if (text == NULL)
                    {
                        perror("Failed to allocate memory for a constant ascii value");
                        exit(EXIT_FAILURE);
                    }
                    memcpy(text, &(tokenizer->buff[tokenizer->pos]), size);
                    text[size] = '\0';

                    // Testing if it is one of the supported keywords
                    if      (strcmp(text, "int") == 0)       tokenizer->next.type = TOK_INT;
                    else if (strcmp(text, "if") == 0)        tokenizer->next.type = TOK_IF;
                    else if (strcmp(text, "else") == 0)      tokenizer->next.type = TOK_ELSE;
                    else if (strcmp(text, "for") == 0)       tokenizer->next.type = TOK_FOR;
                    else if (strcmp(text, "while") == 0)     tokenizer->next.type = TOK_FOR;
                    else if (strcmp(text, "do") == 0)        tokenizer->next.type = TOK_DO;
                    else if (strcmp(text, "break") == 0)     tokenizer->next.type = TOK_BREAK;
                    else if (strcmp(text, "continue") == 0)  tokenizer->next.type = TOK_CONTINUE;
                    else if (strcmp(text, "return") == 0)    tokenizer->next.type = TOK_RETURN;
                    else if (strcmp(text, "print") == 0)     tokenizer->next.type = TOK_PRINT;

                    else // It is an identifier
                    {
                        tokenizer->next.type = TOK_IDENTIFIER;
                        tokenizer->next.value.str_val = text;
                    }

                    if (tokenizer->next.type != TOK_IDENTIFIER) // If it is a keyword, we don't need the text
                        free(text);

                    tokenizer->next.line = tokenizer->line;
                    tokenizer->next.col = tokenizer->col;

                    tokenizer->pos += size;
                    tokenizer->col += size;
                }
                else // It is an invalid character
                {
                    // Creating the invalid character token
                    tokenizer->next.type = TOK_INVALID_CHAR;
                    tokenizer->next.value.invalid_char = tokenizer->buff[tokenizer->pos];
                    tokenizer->next.line = tokenizer->line;
                    tokenizer->next.col = tokenizer->col;

                    tokenizer->pos++;
                    tokenizer->col++;
                }
            }
        }
    }
}

int tokenizer_check(Tokenizer* tokenizer, int token_type)
{
    assert(tokenizer != NULL);

    int checked = 0; 
    
    if (tokenizer->next.type == token_type)
    {
        tokenizer_step(tokenizer);
        checked = 1;
    }

    return checked;
}

void tokenizer_accept(Tokenizer* tokenizer, int token_type)
{
    assert(tokenizer != NULL);

    if ( ! tokenizer_check(tokenizer, token_type))
    {
        fprintf(stderr, "Unexpected token at %d:%d\n", tokenizer->next.line, tokenizer->next.col);
        exit(EXIT_FAILURE);
    }
}

void token_display(Token token, FILE* out_file)
{
    switch (token.type)
    {
        case TOK_INVALID_CHAR:      fprintf(out_file, "INVALID CHARACTER : %c\n", token.value.invalid_char); break;
        case TOK_INVALID_SEQ:       fprintf(out_file, "INVALID SEQUENCE : %s\n", token.value.str_val);       break;
        case TOK_NONE:              fprintf(out_file, "NONE\n");                                             break;
        case TOK_CONST:             fprintf(out_file, "CONST : %d\n", token.value.int_val);                  break;
        case TOK_IDENTIFIER:        fprintf(out_file, "IDENTIFIER : %s\n", token.value.str_val);             break;
        case TOK_PLUS:              fprintf(out_file, "PLUS\n");                                             break;
        case TOK_MINUS:             fprintf(out_file, "MINUS\n");                                            break;
        case TOK_STAR:              fprintf(out_file, "STAR\n");                                             break;
        case TOK_SLASH:				fprintf(out_file, "SLASH\n");                                            break;
        case TOK_PERCENT:			fprintf(out_file, "PERCENT\n");                                          break;
        case TOK_AMPERSAND:			fprintf(out_file, "AMPERSAND\n");                                        break;
        case TOK_EQUAL:				fprintf(out_file, "EQUAL\n");                                            break;
        case TOK_2_EQUAL:			fprintf(out_file, "DOUBLE EQUAL\n");                                     break;
        case TOK_NOT:				fprintf(out_file, "NOT\n");                                              break;
        case TOK_NOT_EQUAL:			fprintf(out_file, "NOT EQUAL\n");                                        break;
        case TOK_LESS:				fprintf(out_file, "LESS\n");                                             break;
        case TOK_GREATER:			fprintf(out_file, "GREATER\n");                                          break;
        case TOK_LESS_OR_EQUAL:		fprintf(out_file, "LESS OR EQUAL\n");                                    break;
        case TOK_GREATER_OR_EQUAL: 	fprintf(out_file, "GREATER OR EQUAL\n");                                 break;
        case TOK_2_AMPERSAND:		fprintf(out_file, "DOUBLE AMPERSAND\n");                                 break;
        case TOK_2_PIPE:			fprintf(out_file, "DOUBLE PIPE\n");                                      break;
        case TOK_COMMA:				fprintf(out_file, "COMMA\n");                                            break;
        case TOK_SEMICOLON:			fprintf(out_file, "SEMICOLON\n");                                        break;
        case TOK_OPEN_PARENTHESIS:	fprintf(out_file, "OPEN PARENTHESIS\n");                                 break;
        case TOK_CLOSE_PARENTHESIS: fprintf(out_file, "CLOSE PARENTHESIS\n");                                break;
        case TOK_OPEN_BRACKET:      fprintf(out_file, "OPEN BRACKET\n");                                     break;
        case TOK_CLOSE_BRACKET:		fprintf(out_file, "CLOSE BRACKET\n");                                    break;
        case TOK_OPEN_BRACE:		fprintf(out_file, "OPEN BRACE\n");                                       break;
        case TOK_CLOSE_BRACE:		fprintf(out_file, "CLOSE BRACE\n");                                      break;
        case TOK_INT:				fprintf(out_file, "INT\n");                                              break;
        case TOK_IF:				fprintf(out_file, "IF\n");                                               break;
        case TOK_ELSE:				fprintf(out_file, "ELSE\n");                                             break;
        case TOK_FOR:				fprintf(out_file, "FOR\n");                                              break;
        case TOK_WHILE:				fprintf(out_file, "WHILE\n");                                            break;
        case TOK_DO:				fprintf(out_file, "DO\n");                                               break;
        case TOK_BREAK:				fprintf(out_file, "BREAK\n");                                            break;
        case TOK_CONTINUE:			fprintf(out_file, "CONTINUE\n");                                         break;
        case TOK_RETURN:			fprintf(out_file, "RETURN\n");                                           break;
        case TOK_EOF:               fprintf(out_file, "EOF\n");                                              break;
    }
}