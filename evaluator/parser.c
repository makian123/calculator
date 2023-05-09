#include "parser.h"
#include "../tokens/token.h"
#include <float.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
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
		case TOK_FUNC_SIN:
		case TOK_FUNC_COS:
		case TOK_FUNC_TG:
		case TOK_FUNC_CTG:
			return 3;
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

double Parse(vector_t *vec) {
	if (!vec) {
		errno = EFAULT;
		return DBL_MAX;
	}
	if (vec->len == 0) return 0;

	if (vec->len == 1) {
		token_t *tmpTok = *(token_t **)VectorAt(vec, 0);
		if (tmpTok->type == TOK_NUMBER) return tmpTok->val;
		else {
			errno = EFAULT;
			return DBL_MAX;
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

	vector_t *bufferTokens = CreateVector(sizeof(token_t *));
	if(!bufferTokens) {
		errno = EFAULT;
		return DBL_MAX;
	}

	vector_t *right = CopyVector(vec);
	if (!right) {
		errno = EFAULT;
		return DBL_MAX;
	}

	vector_t *left = CreateVector(sizeof(token_t *));
	if (!left) {
		errno = EFAULT;
		return DBL_MAX;
	}

	vector_t *buffer = CreateVector(sizeof(token_t *));
	if (!buffer) {
		errno = EFAULT;

		DeleteVector(left);
		return DBL_MAX;
	}

	double res = 0, val = 0;
	//parse functions to tokens
	for (long long i = 0; i < right->len; ++i) {
		token_t *tok = *(token_t**)VectorAt(right, i);
		if (!tok) continue;
		if (tok->type != TOK_FUNC_COS &&
			tok->type != TOK_FUNC_SIN &&
			tok->type != TOK_FUNC_TG &&
			tok->type != TOK_FUNC_CTG) {
			continue;
		}

		token_t *lookahead = *(token_t **)VectorAt(right, i + 1);
		if (!lookahead || lookahead->type != TOK_OPEN_PARENTH) {
			if (bufferTokens->len) {
				while (bufferTokens->len) {
					DeleteToken(*(token_t **)VectorAt(bufferTokens, 0));
				}
			}

			DeleteVector(right);
			DeleteVector(left);
			DeleteVector(buffer);
			DeleteVector(bufferTokens);
			
			errno = EFAULT;
			return DBL_MAX;
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
				if (bufferTokens->len) {
					while (bufferTokens->len) {
						DeleteToken(*(token_t **)VectorAt(bufferTokens, 0));
					}
				}

				DeleteVector(right);
				DeleteVector(left);
				DeleteVector(buffer);
				DeleteVector(bufferTokens);

				errno = EFAULT;
				return DBL_MAX;
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

		token_t *resTok = CreateNumber(res);
		if (!resTok) {
			if (bufferTokens->len) {
				while (bufferTokens->len) {
					DeleteToken(*(token_t **)VectorAt(bufferTokens, 0));
				}
			}

			DeleteVector(right);
			DeleteVector(left);
			DeleteVector(buffer);
			DeleteVector(bufferTokens);

			errno = EFAULT;
			return DBL_MAX;
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
			DeleteVector(left);
			DeleteVector(buffer);

			errno = EFAULT;
			return 0;
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
			return DBL_MAX;
		}

		return tok->val;
	}

	//begins the evaluation
	while (right->len) {
		token_t *tok = *(token_t**)VectorAt(right, 0);

		if (tok->type == TOK_NUMBER) 
			VectorPushBack(buffer, &tok);
		else {
			if (buffer->len < 2) {
				errno = EFAULT;
				break;
			}

			token_t *right = *(token_t **)VectorAt(buffer, buffer->len - 1);
			token_t *left = *(token_t **)VectorAt(buffer, buffer->len - 2);

			switch (tok->type) {
				case TOK_ADD:
					res = left->val + right->val;
					break;
				case TOK_SUB:
					res = left->val - right->val;
					break;
				case TOK_MULT:
					res = left->val * right->val;
					break;
				case TOK_DIV:
					res = left->val / right->val;
					break;
				case TOK_EXP:
					res = pow(left->val, right->val);
					break;
			}
			token_t *tmpTok = CreateNumber(res);
			if (!tmpTok) {
				errno = EFAULT;
				break;
			}

			VectorPushBack(buffer, &tmpTok);

			VectorErase(buffer, buffer->len - 2);
			VectorErase(buffer, buffer->len - 2);
		}

		VectorErase(right, 0);
	}
	while (right->len) VectorErase(right, 0);
	while (left->len) VectorErase(left, 0);
	while (buffer->len) VectorErase(buffer, 0);
	while (bufferTokens->len) {
		DeleteToken(*(token_t**)VectorAt(bufferTokens, 0));
		VectorErase(bufferTokens, 0);
	}

	DeleteVector(right);
	DeleteVector(left);
	DeleteVector(buffer);
	DeleteVector(bufferTokens);

	return (errno == EFAULT ? DBL_MAX : res);
}