#pragma once

enum TokenType {
	TOK_END = -1,

	TOK_NUMBER,

	TOK_ADD,
	TOK_SUB,
	TOK_MULT,
	TOK_DIV,
	TOK_EXP,

	TOK_OPEN_PARENTH,
	TOK_CLOSED_PARENTH,

	TOK_FUNC_SIN,
	TOK_FUNC_COS,
	TOK_FUNC_TG,
	TOK_FUNC_CTG
};

typedef struct token {
	enum TokenType type;

	double val;
}token_t;

token_t *CreateEndToken();
token_t *CreateNumber(double num);
token_t *CreateFunc(const char *name);
token_t *CreateOperation(char op);

void DeleteToken(token_t *tok);

void PrintToken(token_t *tok);