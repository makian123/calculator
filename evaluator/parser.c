#include "parser.h"
#include "../tokens/token.h"
#include <float.h>
#include <errno.h>
#include <stdio.h>

static int Precedence(enum TokenType type) {
	switch (type) {
		case TOK_ADD:
		case TOK_SUB:
			return 0;
		case TOK_MULT:
		case TOK_DIV:
			return 1;
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

double Parse(vector_t *vec) {
	if (!vec) {
		errno = EFAULT;
		return DBL_MAX;
	}
	if (vec->len == 0) return 0;

	if (vec->len == 1) {
		token_t *tmpTok = *(token_t **)VectorAt(vec, 0);
		if (tmpTok->type == TOK_NUMBER) return tmpTok->val;
		DeleteToken(tmpTok);
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

	//parse everything to postfix
	while (vec->len > 0) {
		token_t *tok = *(token_t **)VectorAt(vec, 0);
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
			case TOK_DIV: {
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

		VectorErase(vec, 0);
	}

	while (buffer->len) {
		VectorPushBack(left, VectorAt(buffer, buffer->len - 1));
		VectorErase(buffer, buffer->len - 1);
	}

	//moves everything to the right vector
	while (left->len) {
		token_t *tok = *(token_t **)VectorAt(left, 0);
		VectorPushBack(vec, &tok);
		VectorErase(left, 0);
	}

	//begins the evaluation
	while (vec->len) {
		token_t *tok = *(token_t**)VectorAt(vec, 0);

		if (tok->type == TOK_NUMBER) 
			VectorPushBack(buffer, &tok);
		else {
			if (buffer->len < 2) {
				goto badParse;
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
			}
			token_t *tmpTok = CreateNumber(res);
			if (!tmpTok) {
				goto badParse;
			}

			VectorPushBack(buffer, &tmpTok);

			DeleteToken(right);
			DeleteToken(left);
			DeleteToken(tok);

			VectorErase(buffer, buffer->len - 2);
			VectorErase(buffer, buffer->len - 2);
		}

		VectorErase(vec, 0);
	}

	if (!vec->len && !left->len && buffer->len <= 1) {
		goto goodParse;
	}

	badParse:
	errno = EFAULT;
	while (vec->len) VectorErase(vec, 0);
	while (left->len) VectorErase(left, 0);
	while (buffer->len) VectorErase(buffer, 0);

	DeleteVector(left);
	DeleteVector(buffer);

	return DBL_MAX;

	goodParse:
	while (vec->len) VectorErase(vec, 0);
	while (left->len) VectorErase(left, 0);
	while (buffer->len) VectorErase(buffer, 0);

	DeleteVector(left);
	DeleteVector(buffer);

	return res;
}