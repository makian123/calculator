#include "parser.h"
#include "../tokens/token.h"
#include <float.h>
#include <errno.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES
#include <string.h>

double Parse(vector_t *vec);

static int Precedence(enum TokenType type) {
	switch (type) {
		case TOK_ADD:
		case TOK_SUB:
			return 0;
		case TOK_MULT:
		case TOK_DIV:
			return 1;
		case TOK_EXP:
			return 2;
	}

	return -1;
}

void PrintVector(vector_t *vec) {
	if (!vec) return;

	for (size_t i = 0; i < vec->len; ++i) {
		PrintToken(*(token_t**)VectorAt(vec, i));
		printf("\n");
	}
}

double ParseRange(vector_t *original, size_t begin, size_t len) {
	if (!original || begin >= original->len) {
		errno = EFAULT;
		return DBL_MAX;
	}

	vector_t *vec = CopyVector(original);
	if (!vec) {
		errno = EFAULT;
		return DBL_MAX;
	}

	vec->len = len;
	memcpy(vec->data, (void *)((uintptr_t)vec->data + (vec->itemSize * begin)), vec->itemSize * len);

	double res = Parse(vec);
	DeleteVector(vec);

	return res;
}

char isValidExpression(vector_t *vec) {
	if (!vec || !vec->len) return 0;

	token_t *lastTok = NULL;
	token_t *currTok = NULL;
	size_t openParenth = 0;

	for (size_t i = 0; i < vec->len; ++i) {
		currTok = *(token_t **)VectorAt(vec, i);
		if (!currTok || currTok->type == TOK_END) return 0;

		if (currTok->type == TOK_OPEN_PARENTH) openParenth++;
		else if (currTok->type == TOK_CLOSED_PARENTH) openParenth--;

		if (lastTok && lastTok->type == TOK_OPEN_PARENTH) {
			if (currTok->type != TOK_NUMBER &&
				currTok->type != TOK_ADD &&
				currTok->type != TOK_SUB &&
				currTok->type != TOK_FUNC_SIN &&
				currTok->type != TOK_FUNC_COS &&
				currTok->type != TOK_FUNC_TG &&
				currTok->type != TOK_FUNC_CTG &&
				currTok->type != TOK_FUNC_LOG &&
				currTok->type != TOK_FUNC_LN &&
				currTok->type != TOK_OPEN_PARENTH)
				return 0;
		}

		lastTok = currTok;
	}

	return openParenth == 0;
}

double Parse(vector_t *vec) {
	vector_t *bufferTokens = NULL;
	vector_t *buffer = NULL;
	vector_t *right = NULL;
	vector_t *left = NULL;

	if (!vec) {
		errno = EFAULT;
		goto cleanup;
	}
	if (vec->len == 0) return 0;
	if (!isValidExpression(vec)) {
		errno = EFAULT;
		goto cleanup;
	}

	//PrintVector(vec);

	if (vec->len == 1) {
		token_t *tmpTok = *(token_t **)VectorAt(vec, 0);
		if (tmpTok->type == TOK_NUMBER) return tmpTok->val;
		else {
			errno = EFAULT;
			goto cleanup;
		}
	}
	
	if(vec->len >= 2)
	{
		token_t *sign = *(token_t **)VectorAt(vec, 0);
		token_t *num = *(token_t **)VectorAt(vec, 1);
		
		if ((sign->type == TOK_ADD || sign->type == TOK_SUB) && num->type == TOK_NUMBER) {
			token_t *tmpTok = CreateNumber(0);
			if (!tmpTok) {
				errno = EFAULT;
				return DBL_MAX;
			}

			VectorInsert(vec, 0, &tmpTok);
		}

	}

	bufferTokens = CreateVector(sizeof(token_t *));
	if(!bufferTokens) {
		errno = EFAULT;
		goto cleanup;
	}

	right = CopyVector(vec);
	if (!right) {
		errno = EFAULT;
		goto cleanup;
	}

	left = CreateVector(sizeof(token_t *));
	if (!left) {
		errno = EFAULT;
		goto cleanup;
	}

	buffer = CreateVector(sizeof(token_t *));
	if (!buffer) {
		errno = EFAULT;
		goto cleanup;
	}

	double res = 0, val = 0;
	//parse functions to tokens
	for (long long i = 0; i < right->len; ++i) {
		token_t *tok = *(token_t**)VectorAt(right, i);
		if (!tok) continue;
		if (tok->type != TOK_FUNC_COS &&
			tok->type != TOK_FUNC_SIN &&
			tok->type != TOK_FUNC_TG &&
			tok->type != TOK_FUNC_CTG &&
			tok->type != TOK_FUNC_LOG &&
			tok->type != TOK_FUNC_LN &&
			tok->type != TOK_FUNC_SQRT &&
			tok->type != TOK_FUNC_ABS) {
			continue;
		}

		token_t *lookahead = *(token_t **)VectorAt(right, i + 1);
		if (!lookahead || lookahead->type != TOK_OPEN_PARENTH) {
			errno = EFAULT;
			goto cleanup;
		}

		size_t ctr = 0;
		size_t openParenth = 1;
		//Figure out which range the parentheses are
		while (openParenth != 0 && ((i + 2 + ctr) < right->len)) {
			lookahead = *(token_t **)VectorAt(right, i + 2 + ctr);
			if (lookahead->type == TOK_CLOSED_PARENTH) openParenth--;
			else if (lookahead->type == TOK_OPEN_PARENTH) openParenth++;
			if (!openParenth) break;

			if (!lookahead) {
				errno = EFAULT;
				goto cleanup;
			}
			ctr++;
		}

		//Gets the result of the inside of function
		double res = ParseRange(right, i + 2, ctr);
		//Do operation
		if (tok->type == TOK_FUNC_SIN) res = sin(res);
		else if (tok->type == TOK_FUNC_COS) res = cos(res);
		else if (tok->type == TOK_FUNC_TG) res = tan(res);
		else if (tok->type == TOK_FUNC_CTG) res = 1.0 / tan(res);
		else if (tok->type == TOK_FUNC_LOG) res = log(res);
		else if (tok->type == TOK_FUNC_LN) res = log(res) / log(M_E);
		else if (tok->type == TOK_FUNC_ABS) res = fabs(res);
		else if (tok->type == TOK_FUNC_SQRT) {
			if (res < 0.0) {
				errno = EFAULT;
				goto cleanup;
			}

			res = sqrt(res);
		}

		token_t *resTok = CreateNumber(res);
		if (!resTok) {
			errno = EFAULT;
			goto cleanup;
		}

		for (long long j = i + ctr + 2; (j >= i) && right->len; --j) {
			VectorErase(right, j);
		}

		VectorInsert(right, i, &resTok);
		VectorPushBack(bufferTokens, &resTok);
	}

	//parse everything to postfix
	while (right->len > 0) {
		token_t *tok = *(token_t **)VectorAt(right, 0);
		if (!tok) {
			errno = EFAULT;
			goto cleanup;
		}

		switch (tok->type) {
			case TOK_NUMBER:
				VectorPushBack(left, &tok);
				break;

			case TOK_ADD:
			case TOK_SUB:
			case TOK_MULT:
			case TOK_DIV:
			case TOK_EXP: {
				while (buffer->len) {
					token_t *buffTok = *(token_t **)VectorAt(buffer, buffer->len - 1);
					if (!buffTok) continue;
					if (Precedence(buffTok->type) >= Precedence(tok->type)) {
						VectorPushBack(left, &buffTok);
						VectorErase(buffer, buffer->len - 1);
					}
					else break;
				}

				VectorPushBack(buffer, &tok);
				break;
			}
			case TOK_OPEN_PARENTH:
				VectorPushBack(buffer, &tok);
				break;
			case TOK_CLOSED_PARENTH: {
				while (buffer->len) {
					token_t *tok = *(token_t **)VectorAt(buffer, buffer->len - 1);
					if (tok->type == TOK_OPEN_PARENTH) {
						VectorErase(buffer, buffer->len - 1);
						break;
					}

					VectorPushBack(left, &tok);
					VectorErase(buffer, buffer->len - 1);
				}
			}
		}

		VectorErase(right, 0);
	}

	while (buffer->len) {
		VectorPushBack(left, VectorAt(buffer, buffer->len - 1));
		VectorErase(buffer, buffer->len - 1);
	}

	//moves everything to the right vector
	right = CopyVector(left);
	VectorClear(left);

	if (right->len == 1) {
		token_t *tok = *(token_t **)VectorAt(right, 0);
		if (!tok) {
			errno = EFAULT;
			goto cleanup;
		}

		return tok->val;
	}

	//printf("Postfix:\n");
	//PrintVector(right);
	//printf("\n");

	//begins the evaluation
	while (right->len) {
		token_t *tok = *(token_t**)VectorAt(right, 0);

		if (tok->type == TOK_NUMBER) 
			VectorPushBack(buffer, &tok);
		else {
			if (buffer->len < 2) {
				errno = EFAULT;
				goto cleanup;
			}
			token_t *rightTok = *(token_t **)VectorAt(buffer, buffer->len - 1);
			token_t *leftTok = *(token_t **)VectorAt(buffer, buffer->len - 2);

			switch (tok->type) {
				case TOK_ADD:
					res = leftTok->val + rightTok->val;
					break;
				case TOK_SUB:
					res = leftTok->val - rightTok->val;
					break;
				case TOK_MULT:
					res = leftTok->val * rightTok->val;
					break;
				case TOK_DIV:
					res = leftTok->val / rightTok->val;
					break;
				case TOK_EXP:
					res = pow(leftTok->val, rightTok->val);
					break;
			}
			token_t *tmpTok = CreateNumber(res);
			if (!tmpTok) {
				errno = EFAULT;
				goto cleanup;
			}

			VectorPushBack(buffer, &tmpTok);

			VectorErase(buffer, buffer->len - 2);
			VectorErase(buffer, buffer->len - 2);
		}

		VectorErase(right, 0);
	}
	
	cleanup:
	while (bufferTokens && bufferTokens->len) {
		DeleteToken(*(token_t**)VectorAt(bufferTokens, 0));
		VectorErase(bufferTokens, 0);
	}

	DeleteVector(right);
	DeleteVector(left);
	DeleteVector(buffer);
	DeleteVector(bufferTokens);

	return (errno == EFAULT ? DBL_MAX : res);
}