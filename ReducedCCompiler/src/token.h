#ifndef TOKEN_H
#define TOKEN_H

typedef struct
{
	int type;
	union
	{
		int int_val;
		char* str_val;
	} value;
	int line;
	int col;
} Token;

typedef struct
{
	char* buff;
	int   pos;
	int   line;
	int   col;
	Token current;
	Token next;
} Tokenizer;

void step(Tokenizer* tokenizer);
int  check(Tokenizer* tokenizer);
void accept(Tokenizer* tokenizer);

void display_token(Token token);

enum
{
	TOK_NONE,

	TOK_CONST,				// Numeric value
	TOK_IDENTIFIER,			// Identifier

	// Operators
	TOK_PLUS,				// +
	TOK_MINUS,				// -
	TOK_STAR,				// *
	TOK_SLASH,				// /
	TOK_PERCENT,			// %
	TOK_AMPERSAND,			// &
	TOK_EQUAL,				// =
	TOK_2_EQUAL,			// ==
	TOK_NOT,				// !
	TOK_NOT_EQUAL,			// !=
	TOK_LESS,				// <
	TOK_GREATER,			// >
	TOK_LESS_OR_EQUAL,		// <=
	TOK_GREATER_OR_EQUAL,	// >=
	TOK_2_AMPERSAND,		// &&
	TOK_2_PIPE,				// ||
	
	// Ponctuation
	TOK_COMMA,				// ,
	TOK_SEMICOLON,			// ;
	TOK_OPEN_PARENTHESIS,	// (
	TOK_CLOSE_PARENTHESIS,	// )
	TOK_OPEN_BRACKET,		// [
	TOK_CLOSE_BRACKET,		// ]
	TOK_OPEN_BRACE,			// {
	TOK_CLOSE_BRACE,		// }

	// Keywords
	TOK_INT,				// int
	TOK_IF,					// if
	TOK_ELSE,				// else
	TOK_FOR,				// for
	TOK_WHILE,				// while
	TOK_DO,					// do
	TOK_BREAK,				// break
	TOK_CONTINUE,			// continue
	TOK_RETURN,				// return

	TOK_EOF					// End of file
};

static const Token EMPTY_TOKEN = { TOK_NONE, 0, 0, 0 };
static const Tokenizer EMPTY_TOKENIZER = { 0, 0, 0, 0, { TOK_NONE, 0, 0, 0 }, { TOK_NONE, 0, 0, 0 } };

#endif
