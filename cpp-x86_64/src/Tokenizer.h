#pragma once

#include "Token.h"

class Tokenizer
{
public:
	Tokenizer(std::string&& filename, std::istream& inputStream)
		: m_filename(filename), m_inputStream(inputStream) {}

	struct iterator;
	token_t step();
	iterator begin();
	iterator end();

private:
	std::string   m_filename;
	std::istream& m_inputStream;
};