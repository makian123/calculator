#include "tokenizer.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

tokenizer_t *CreateTokenizer(const char *str) {
	tokenizer_t *tokenizer = malloc(sizeof(tokenizer_t));
	if (!tokenizer) return NULL;
	tokenizer->index = 0;

	size_t len = strlen(str) + 1;
	tokenizer->str = malloc(len);
	if (!tokenizer->str) {
		free(tokenizer);
		return NULL;
	}

	memcpy(tokenizer->str, str, len);

	return tokenizer;
}
void DeleteTokenizer(tokenizer_t *tokenizer) {
	if (!tokenizer) return;

	if (tokenizer->str) free(tokenizer->str);
	free(tokenizer);
}

token_t *TokenizerNextToken(tokenizer_t *tokenizer) {
	if (!tokenizer || !tokenizer->str) return NULL;
	if (tokenizer->index >= strlen(tokenizer->str)) return CreateEndToken();

	char *ptr = tokenizer->str;
	while (ptr[tokenizer->index] && isspace(ptr[tokenizer->index])) {
		tokenizer->index++;
	}

	char buffer[1024] = { 0 };
	if (isalpha(ptr[tokenizer->index])) {
		while (isalpha(ptr[tokenizer->index])) {
			buffer[strlen(buffer)] = ptr[tokenizer->index++];
		}

		return CreateFunc(buffer);
	}
	if (isalnum(ptr[tokenizer->index])) {
		while (isalnum(ptr[tokenizer->index]) || ptr[tokenizer->index] == '.') {
			buffer[strlen(buffer)] = ptr[tokenizer->index++];
		}
		return CreateNumber(atof(buffer));
	}

	//Deals with repeating additions and subtractions
	//Eg: 1 ++ - 2 -> 1 - 2
	if (ptr[tokenizer->index] == '+' || ptr[tokenizer->index] == '-') {
		size_t negatives = 0;

		while (ptr[tokenizer->index] == '+' || ptr[tokenizer->index] == '-') {
			negatives += ptr[tokenizer->index] == '-';
			
			tokenizer->index++;
			while (ptr[tokenizer->index] && isspace(ptr[tokenizer->index])) {
				tokenizer->index++;
			}
		}
		return CreateOperation((negatives & 1) ? '-' : '+');
	}

	return CreateOperation(ptr[tokenizer->index++]);;
}
token_t *TokenizerCurrentToken(tokenizer_t *tokenizer) {
	size_t index = tokenizer->index;

	token_t *ret = TokenizerNextToken(tokenizer);
	
	tokenizer->index = index;

	return ret;
}