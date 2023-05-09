#include <stdio.h>

#include "utils/vector.h"
#include "tokens/token.h"
#include "tokens/tokenizer.h"
#include "evaluator/parser.h"

#include <math.h>

int main(void) {
	tokenizer_t *tokenizer = CreateTokenizer("0.3.5 * 100 ( / 50))");
	if (!tokenizer) return 1;

	token_t *tok = NULL;
	vector_t *vec = CreateVector(sizeof(token_t *));
	while ((tok = TokenizerNextToken(tokenizer)) != NULL) {
		VectorPushBack(vec, &tok);
	}

	double result = Parse(vec);
	if (errno == 0) {
		printf("Value of '%s' is: %lf\n", tokenizer->str, result);
	}
	else {
		printf("Error parsing the expression\n");
	}

	for (size_t i = 0; i < vec->len; ++i) {
		DeleteToken(*(token_t **)VectorAt(vec, i));
	}

	DeleteVector(vec);
	DeleteTokenizer(tokenizer);

	return 0;
}