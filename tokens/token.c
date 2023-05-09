#include "token.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

token_t *CreateNumber(double num) {
	token_t *tok = malloc(sizeof(token_t));
	if (!tok) return NULL;

	tok->type = TOK_NUMBER;
	tok->val = num;

	return tok;
}
token_t *CreateFunc(const char *name) {
	token_t *tok = malloc(sizeof(token_t));
	if (!tok) return NULL;
	char isGood = 1;

	if (!strcmp(name, "sin")) tok->type = TOK_FUNC_SIN;
	else if (!strcmp(name, "cos")) tok->type = TOK_FUNC_COS;
	else if (!strcmp(name, "tg")) tok->type = TOK_FUNC_TG;
	else if (!strcmp(name, "ctg")) tok->type = TOK_FUNC_CTG;
	else isGood = 0;

	if (!isGood) {
		free(tok);
		return NULL;
	}

	return tok;
}
token_t *CreateOperation(char op) {
	token_t *tok = malloc(sizeof(token_t));
	if (!tok) return NULL;

	switch (op) {
		case '+':
			tok->type = TOK_ADD;
			break;
		case '-':
			tok->type = TOK_SUB;
			break;
		case '*':
			tok->type = TOK_MULT;
			break;
		case '/':
			tok->type = TOK_DIV;
			break;
		case '^':
			tok->type = TOK_EXP;
			break;
		case '(':
			tok->type = TOK_OPEN_PARENTH;
			break;
		case ')':
			tok->type = TOK_CLOSED_PARENTH;
			break;
		default:
			free(tok);
			return NULL;
			break;
	}

	return tok;
}

void DeleteToken(token_t *tok) {
	if (tok) free(tok);
}

static const char *tokTypeNames[] = {
	"NUMBER",
	"+", "-", "*", "/", "^",
	"(", ")",
	"sin", "cos", "tg", "ctg"
};

void PrintToken(token_t *tok) {
	if (!tok) return;

	printf("Token: %s", tokTypeNames[tok->type]);
	if (tok->type == TOK_NUMBER) {
		printf(": %lf", tok->val);
	}
}