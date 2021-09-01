#include "token.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void set_next(Tokenizer* tokenizer, int type)
{
	tokenizer->next.type = type;
	tokenizer->next.line = tokenizer->line;
	tokenizer->next.col = tokenizer->col;
}

void token_error(Tokenizer* tokenizer)
{
	fprintf(stderr, "Invalid token '%c' at %d:%d", tokenizer->buff[tokenizer->pos + 1], tokenizer->line, tokenizer->col);
	exit(EXIT_FAILURE);
}

inline int is_numeric(char c)
{
	return c >= '0' && c <= '9';
}

inline int is_alphanumeric(char c)
{
	return (c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| is_numeric(c);
}

void step(Tokenizer* tokenizer)
{
	if (tokenizer->current.type == TOK_IDENTIFIER)
		free(tokenizer->current.value.str_val);
	//  str_val is dynamically allocated when an identifier token is created
	//  so it must be freed before overwriting tokenizer.current

	tokenizer->current = tokenizer->next;
	int found = 0;
	while ( ! found)
	{
		switch (tokenizer->buff[tokenizer->pos])
		{
			case '\n':
			{
				tokenizer->line++;
				tokenizer->col = 0;
				break;
			}
			case ' ':
			case '\t':
			case '\r':
			{
				tokenizer->col++;
				break;
			}
			case '&' :
			{
				found = 1;
				if (tokenizer->buff[tokenizer->pos + 1] == '&')
				{
					set_next(tokenizer, TOK_2_AMPERSAND);
					tokenizer->pos++;
				}
				else
					set_next(tokenizer, TOK_AMPERSAND);
				break;
			}
			case '=':
			{
				found = 1;
				if (tokenizer->buff[tokenizer->pos + 1] == '=')
				{
					set_next(tokenizer, TOK_2_EQUAL);
					tokenizer->pos++;
				}
				else
					set_next(tokenizer, TOK_EQUAL);
				break;
			}
			case '!':
			{
				found = 1;
				if (tokenizer->buff[tokenizer->pos + 1] == '=')
				{
					set_next(tokenizer, TOK_NOT_EQUAL);
					tokenizer->pos++;
				}
				else
					set_next(tokenizer, TOK_NOT);
				break;
			}
			case '<':
			{
				found = 1;
				if (tokenizer->buff[tokenizer->pos + 1] == '=')
				{
					set_next(tokenizer, TOK_LESS_OR_EQUAL);
					tokenizer->pos++;
				}
				else
					set_next(tokenizer, TOK_LESS);
				break;
			}
			case '>':
			{
				found = 1;
				if (tokenizer->buff[tokenizer->pos + 1] == '=')
				{
					set_next(tokenizer, TOK_GREATER_OR_EQUAL);
					tokenizer->pos++;
				}
				else
					set_next(tokenizer, TOK_GREATER);
				break;
			}
			case '|':
			{
				found = 1;
				if (tokenizer->buff[tokenizer->pos + 1] == '|')
				{
					set_next(tokenizer, TOK_2_PIPE);
					tokenizer->pos++;
				}
				else
				{
					token_error(tokenizer);
				}
				break;
			}

			case '+' : found = 1; set_next(tokenizer, TOK_PLUS);			  break;
			case '-' : found = 1; set_next(tokenizer, TOK_MINUS);			  break;
			case '*' : found = 1; set_next(tokenizer, TOK_STAR);			  break;
			case '/' : found = 1; set_next(tokenizer, TOK_SLASH);			  break;
			case '%' : found = 1; set_next(tokenizer, TOK_PERCENT);			  break;

			case ',' : found = 1; set_next(tokenizer, TOK_COMMA);			  break;
			case ';' : found = 1; set_next(tokenizer, TOK_SEMICOLON);		  break;
			case '(' : found = 1; set_next(tokenizer, TOK_OPEN_PARENTHESIS);  break;
			case ')' : found = 1; set_next(tokenizer, TOK_CLOSE_PARENTHESIS); break;
			case '[' : found = 1; set_next(tokenizer, TOK_OPEN_BRACKET);	  break;
			case ']' : found = 1; set_next(tokenizer, TOK_CLOSE_BRACKET);	  break;
			case '{' : found = 1; set_next(tokenizer, TOK_OPEN_BRACE);		  break;
			case '}' : found = 1; set_next(tokenizer, TOK_CLOSE_BRACE);		  break;
			case '\0': found = 1; set_next(tokenizer, TOK_EOF);				  break;

			default  :
			{
				found = 1;
				if (is_numeric(tokenizer->buff[tokenizer->pos]))
				{
					int size = 0;
					while (is_numeric(tokenizer->buff[tokenizer->pos + size]))
						size++;
					char* ascii_value = malloc(sizeof(char) * size + 1);
					if (ascii_value == NULL)
					{
						perror("Failed to allocate memory for a constant ascii value");
						exit(EXIT_FAILURE);
					}
					memcpy(ascii_value, &(tokenizer->buff[tokenizer->pos]), size);
					ascii_value[size] = '\0';
					tokenizer->next.type = TOK_CONST;
					tokenizer->next.value.int_val = atoi(ascii_value);
					tokenizer->next.line = tokenizer->line;
					tokenizer->next.col = tokenizer->col;

					tokenizer->pos += size - 1;
					tokenizer->col += size - 1;
					free(ascii_value);
				}
				else if (is_alphanumeric(tokenizer->buff[tokenizer->pos]))
				{
					int size = 0;
					while (is_alphanumeric(tokenizer->buff[tokenizer->pos + size]))
						size++;
					char* identifier_text = malloc(sizeof(char) * size + 1);
					if (identifier_text == NULL)
					{
						perror("Failed to allocate memory for a constant ascii value");
						exit(EXIT_FAILURE);
					}
					memcpy(identifier_text, &(tokenizer->buff[tokenizer->pos]), size);
					identifier_text[size] = '\0';
					tokenizer->next.type = TOK_IDENTIFIER;
					tokenizer->next.value.str_val = identifier_text;
					tokenizer->next.line = tokenizer->line;
					tokenizer->next.col = tokenizer->col;

					tokenizer->pos += size - 1;
					tokenizer->col += size - 1;
				}
				else
				{
					token_error(tokenizer);
				}
			}
		}
		tokenizer->pos++;
	}
}

int check(Tokenizer* tokenizer)
{

}

void accept(Tokenizer* tokenizer)
{

}

void display_token(Token token)
{
	switch (token.type)
	{
	case TOK_NONE: printf("NONE\n"); break;
	case TOK_CONST: printf("CONST : %d\n", token.value.int_val); break;
	case TOK_IDENTIFIER: printf("IDENTIFIER : %s\n", token.value.str_val); break;
	case TOK_PLUS: 
	case TOK_MINUS:
	case TOK_STAR:
	case TOK_SLASH:
	case TOK_PERCENT:
	case TOK_AMPERSAND:
	case TOK_EQUAL:
	case TOK_2_EQUAL:
	case TOK_NOT:
	case TOK_NOT_EQUAL:
	case TOK_LESS:
	case TOK_GREATER:
	case TOK_LESS_OR_EQUAL:
	case TOK_GREATER_OR_EQUAL:
	case TOK_2_AMPERSAND:
	case TOK_2_PIPE:
	case TOK_COMMA:
	case TOK_SEMICOLON:
	case TOK_OPEN_PARENTHESIS:
	case TOK_CLOSE_PARENTHESIS:
	case TOK_OPEN_BRACKET:
	case TOK_CLOSE_BRACKET:
	case TOK_OPEN_BRACE:
	case TOK_CLOSE_BRACE:
	case TOK_INT:
	case TOK_IF:
	case TOK_ELSE:
	case TOK_FOR:
	case TOK_WHILE:
	case TOK_DO:
	case TOK_BREAK:
	case TOK_CONTINUE:
	case TOK_RETURN:
	case TOK_EOF: printf("EOF\n"); break;
		break;
	}
}