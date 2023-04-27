#include <stdio.h>

#include "utils/vector.h"
#include "tokens/token.h"
#include "tokens/tokenizer.h"
#include "evaluator/parser.h"

int main(void) {
	tokenizer_t *tokenizer = CreateTokenizer("12");
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

	DeleteVector(vec);
	DeleteTokenizer(tokenizer);

	return 0;
}