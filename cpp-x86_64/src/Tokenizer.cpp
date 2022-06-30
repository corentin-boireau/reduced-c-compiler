#include "Tokenizer.h"

token_t Tokenizer::step()
{
	return std::make_unique<Plus>(this->m_filename, 0, 0);
}

token_t Tokenizer::begin()
{
	return step();
}

token_t Tokenizer::end()
{
	return nullptr;
}
