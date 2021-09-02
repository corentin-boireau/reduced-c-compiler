#include "token.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Token create_token() 
{
	Token token;
	token.type = TOK_NONE;

	return token;
}
Tokenizer create_tokenizer(char* buff)
{
	Tokenizer tokenizer;
	tokenizer.buff    = buff;
	tokenizer.pos     = 0;
	tokenizer.line    = 1;
	tokenizer.col     = 1;
	tokenizer.current = create_token();
	tokenizer.next    = create_token();

	return tokenizer;
}

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

inline int is_letter(char c)
{
	return (c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| c == '_';
}

inline int is_alphanumeric(char c)
{
	return is_letter(c) || is_numeric(c);
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
					token_error(tokenizer);
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
				if (is_numeric(tokenizer->buff[tokenizer->pos]))
				{
					int size = 0;
					while (is_numeric(tokenizer->buff[tokenizer->pos + size]))
						size++;
					if (is_alphanumeric(tokenizer->buff[tokenizer->pos + size]))
					{
						token_error(tokenizer);
					}

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

					tokenizer->pos += size;
					tokenizer->col += size;
					free(ascii_value);
				}
				else if (is_letter(tokenizer->buff[tokenizer->pos]))
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

					if      (strcmp(text, "int") == 0)       tokenizer->next.type = TOK_INT;
					else if (strcmp(text, "if") == 0)        tokenizer->next.type = TOK_IF;
					else if (strcmp(text, "else") == 0)      tokenizer->next.type = TOK_ELSE;
					else if (strcmp(text, "for") == 0)       tokenizer->next.type = TOK_FOR;
					else if (strcmp(text, "while") == 0)     tokenizer->next.type = TOK_FOR;
					else if (strcmp(text, "do") == 0)        tokenizer->next.type = TOK_DO;
					else if (strcmp(text, "break") == 0)     tokenizer->next.type = TOK_BREAK;
					else if (strcmp(text, "continue") == 0)  tokenizer->next.type = TOK_CONTINUE;
					else if (strcmp(text, "return") == 0)    tokenizer->next.type = TOK_RETURN;

					else
					{
						tokenizer->next.type = TOK_IDENTIFIER;
						tokenizer->next.value.str_val = text;
					}

					if (tokenizer->next.type != TOK_IDENTIFIER)
						free(text);

					tokenizer->next.line = tokenizer->line;
					tokenizer->next.col = tokenizer->col;

					tokenizer->pos += size;
					tokenizer->col += size;
				}
				else
				{
					token_error(tokenizer);
				}
			}
		}
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
	case TOK_NONE:              printf("NONE\n"); break;
	case TOK_CONST:             printf("CONST : %d\n", token.value.int_val); break;
	case TOK_IDENTIFIER:        printf("IDENTIFIER : %s\n", token.value.str_val); break;
	case TOK_PLUS:              printf("PLUS\n"); break;
	case TOK_MINUS:             printf("MINUS\n"); break;
	case TOK_STAR:              printf("STAR\n"); break;
	case TOK_SLASH:				printf("SLASH\n"); break;
	case TOK_PERCENT:			printf("PERCENT\n"); break;
	case TOK_AMPERSAND:			printf("AMPERSAND\n"); break;
	case TOK_EQUAL:				printf("EQUAL\n"); break;
	case TOK_2_EQUAL:			printf("DOUBLE EQUAL\n"); break;
	case TOK_NOT:				printf("NOT\n"); break;
	case TOK_NOT_EQUAL:			printf("NOT EQUAL\n"); break;
	case TOK_LESS:				printf("LESS\n"); break;
	case TOK_GREATER:			printf("GREATER\n"); break;
	case TOK_LESS_OR_EQUAL:		printf("LESS OR EQUAL\n"); break;
	case TOK_GREATER_OR_EQUAL: 	printf("GREATER OR EQUAL\n"); break;
	case TOK_2_AMPERSAND:		printf("DOUBLE AMPERSAND\n"); break;
	case TOK_2_PIPE:			printf("DOUBLE PIPE\n"); break;
	case TOK_COMMA:				printf("COMMA\n"); break;
	case TOK_SEMICOLON:			printf("SEMICOLON\n"); break;
	case TOK_OPEN_PARENTHESIS:	printf("OPEN PARENTHESIS\n"); break;
	case TOK_CLOSE_PARENTHESIS: printf("CLOSE PARENTHESIS\n"); break;
	case TOK_OPEN_BRACKET:      printf("OPEN BRACKET\n"); break;
	case TOK_CLOSE_BRACKET:		printf("CLOSE BRACKET\n"); break;
	case TOK_OPEN_BRACE:		printf("OPEN BRACE\n"); break;
	case TOK_CLOSE_BRACE:		printf("CLOSE BRACE\n"); break;
	case TOK_INT:				printf("INT\n"); break;
	case TOK_IF:				printf("IF\n"); break;
	case TOK_ELSE:				printf("ELSE\n"); break;
	case TOK_FOR:				printf("FOR\n"); break;
	case TOK_WHILE:				printf("WHILE\n"); break;
	case TOK_DO:				printf("DO\n"); break;
	case TOK_BREAK:				printf("BREAK\n"); break;
	case TOK_CONTINUE:			printf("CONTINUE\n"); break;
	case TOK_RETURN:			printf("RETURN\n"); break;
	case TOK_EOF:               printf("EOF\n"); break;
		break;
	}
}