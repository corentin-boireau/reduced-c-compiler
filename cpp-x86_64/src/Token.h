#pragma once

#include <cstdint>
#include <string_view>
#include <string>
#include <memory>

struct Token
{
	Token(std::string& filename, uint32_t line, uint32_t col)
		: filename(filename), line(line), col(col) {}

	std::string_view filename;
	uint32_t line;
	uint32_t col;

	virtual std::string toString() = 0;
};

using token_t = std::unique_ptr<Token>;

struct Plus  : Token { using Token::Token; std::string toString(); };
struct Minus : Token { using Token::Token; std::string toString(); };
struct Star  : Token { using Token::Token; std::string toString(); };
struct Slash : Token { using Token::Token; std::string toString(); };
