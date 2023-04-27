#pragma once

#include "../utils/vector.h"
#include "token.h"
#include <stddef.h>

typedef struct tokenizer{
	char *str;
	size_t index;
}tokenizer_t;

tokenizer_t *CreateTokenizer(const char *str);
void DeleteTokenizer(tokenizer_t *tokenizer);

token_t *TokenizerNextToken(tokenizer_t *tokenizer);
token_t *TokenizerCurrentToken(tokenizer_t *tokenizer);