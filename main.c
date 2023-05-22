#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>

#include "utils/vector.h"
#include "tokens/token.h"
#include "tokens/tokenizer.h"
#include "evaluator/parser.h"

void Swap(double *a, double *b) {
	if (!a || !b) return;

	double tmp = *a;
	*a = *b;
	*b = tmp;
}

void BubbleSort(double *arrCmp, double *arrOriginal, size_t n, char greater) {
	if (!arrCmp || !arrOriginal || !n) return;

	for (size_t i = 0; i < n - 1; ++i) {
		for (size_t j = i + 1; j < n; ++j) {
			if (greater && arrCmp[j] > arrCmp[i]) {
				Swap(&arrCmp[j], &arrCmp[i]);
				Swap(&arrOriginal[j], &arrOriginal[i]);
			}
			else if (!greater && arrCmp[j] < arrCmp[i]) {
				Swap(&arrCmp[j], &arrCmp[i]);
				Swap(&arrOriginal[j], &arrOriginal[i]);
			}
		}
	}
}

void ExpressionParser() {
	char arr[256] = { 0 };

	printf("Unesite izraz:\n");
	scanf(" %256[^\n]", arr);
	getc(stdin);

	tokenizer_t *tokenizer = CreateTokenizer(arr);
	if (!tokenizer) return;

	token_t *tok = NULL;
	vector_t *vec = CreateVector(sizeof(token_t *));
	while ((tok = TokenizerNextToken(tokenizer)) != NULL && tok->type != TOK_END) {
		VectorPushBack(vec, &tok);
	}
	if (tok == NULL) {
		printf("Neispravan unos\n");

		for (size_t i = 0; i < vec->len; ++i) {
			DeleteToken(*(token_t **)VectorAt(vec, i));
		}

		DeleteVector(vec);
		DeleteTokenizer(tokenizer);

		return;
	}
	if (tok->type == TOK_END) {
		free(tok);
	}

	double result = Parse(vec);
	if (errno == 0) {
		printf("Rjesenje je %lg\n", result);
	}
	else {
		printf("Neispravan unos\n");
	}

	for (size_t i = 0; i < vec->len; ++i) {
		DeleteToken(*(token_t **)VectorAt(vec, i));
	}

	DeleteVector(vec);
	DeleteTokenizer(tokenizer);
}
void FindNumber() {
	char file[256] = { 0 };
	double target;
	
	while (getc(stdin) != '\n');
	printf("Unesite putanju datoteke: ");
	scanf("%256[^\n]", file);
	getc(stdin);

	printf("Unesite trazeni broj: ");
	scanf(" %lf", &target);

	FILE *fp = fopen(file, "r");
	if (!fp) {
		printf("Greska u otvaranju dokumenta\n");
		return;
	}

	size_t ctr = 0;
	double tmp = 0;
	while (fscanf(fp, " %lf", &tmp) == 1) {
		if (tmp == target) {
			printf("Broj %lg je pronaden na indeksu: %zu\n", target, ctr);
			fclose(fp);
			return;
		}

		ctr++;
	}
	
	printf("Broj %lg nije pronaden\n", target);
	fclose(fp);
}
void SortNumbers() {
	char file[256] = { 0 };
	char exprBuffer[1024] = { 0 };

	while (getc(stdin) != '\n');
	printf("Unesite putanju datoteke: ");
	scanf("%256[^\n]", file);
	getc(stdin);

	FILE *fp = fopen(file, "r");
	if (!fp) {
		printf("Greska u otvaranju dokumenta\n");
		return;
	}

	printf("Unesite izraz za sortiranje ((</>) y), y je varijabla poslije x\n");
	printf("Primjer:\n");
	printf("\tPrije sortiranja : [1, 2, 4, 6] \n");
	printf("\tIzraz: 'x < y'\n");
	printf("\tNakon sortiranja [6, 4, 2, 1]\n");
	printf("x ");
	scanf(" %1023[^\n]", exprBuffer);
	getc(stdin);

	size_t ctr = 0;
	double tmp = 0;
	while (fscanf(fp, " %lf", &tmp) == 1) ctr++;
	if (ctr < 2) {
		printf("Nedovoljno brojeva\n");
		fclose(fp);
		return;
	}

	//Postavlja na pocetak
	fseek(fp, 0, SEEK_SET);

	double *brojevi = calloc(ctr, sizeof(double));
	if (!brojevi) {
		printf("Sistemska pogreska, nedovoljno memorije\n");
		fclose(fp);
		return;
	}

	for (size_t i = 0; i < ctr; ++i) {
		fscanf(fp, " %lf", &brojevi[i]);
	}
	fclose(fp);

	double *brojeviRacunati = calloc(ctr, sizeof(double));
	if (!brojeviRacunati) {
		free(brojevi);
		return;
	}
	tokenizer_t *tokenizer = CreateTokenizer(exprBuffer);
	if (!tokenizer) {
		free(brojevi);
		free(brojeviRacunati);
		return;
	}
	vector_t *vec = CreateVector(sizeof(token_t*));
	if (!vec) {
		free(tokenizer);
		free(brojeviRacunati);
		return;
	}

	token_t *tok = NULL;
	char greater = 0;
	
	for (size_t i = 0; i < ctr; ++i) {
		char hadTok = 0;
		while ((tok = TokenizerNextToken(tokenizer)) && tok->type != TOK_END) {
			if(tok->type == TOK_VAR_Y) {
				tok->val = brojevi[i];
				tok->type = TOK_NUMBER;
			}
			else if (tok->type == TOK_GREATER || tok->type == TOK_LOWER) {
				if (hadTok) {
					free(brojevi);
					free(brojeviRacunati);
					DeleteTokenizer(tokenizer);

					DeleteVector(vec);

					printf("Pogreska pri unosu\n");
					return;
				}
				greater = tok->type == TOK_GREATER;
				hadTok = 1;
				continue;
			}

			VectorPushBack(vec, &tok);
		}
		if (!hadTok) {
			free(brojevi);
			free(brojeviRacunati);
			DeleteTokenizer(tokenizer);

			while (vec->len) VectorErase(vec, 0);
			DeleteVector(vec);

			printf("Pogreska pri unosu\n");
			return;
		}

		double tmp = Parse(vec);
		brojeviRacunati[i] = tmp;

		VectorClear(vec);
		tokenizer->index = 0;

		if (errno == EFAULT) {
			free(brojevi);
			free(brojeviRacunati);
			DeleteTokenizer(tokenizer);

			while (vec->len) VectorErase(vec, 0);
			DeleteVector(vec);

			printf("Pogreska pri sortiranju\n");
			return;
		}
	}

	BubbleSort(brojeviRacunati, brojevi, ctr, greater);

	fp = fopen(file, "w");
	if (!fp) {
		printf("Greska u otvaranju dokumenta\n");
	}
	else {
		size_t index = 0;
		while (ctr-- > 0) {
			fprintf(fp, "%lg ", brojevi[index++]);
		}
	}
	
	while (vec->len) VectorErase(vec, 0);
	free(brojevi);
	free(brojeviRacunati);
	DeleteTokenizer(tokenizer);
	DeleteVector(vec);
	if(fp) fclose(fp);
}

int main(void) {
	int choice;

	while(1) {
		printf("Unesite:\n");
		printf("\t[1] Proslijedivanje izraza\n");
		printf("\t[2] Trazenje broja\n");
		printf("\t[3] Sortiranje brojeva\n");
		printf("\t[0] Kraj\n> ");


		if (scanf(" %d", &choice) == 0) {
			printf("Neispravan unos\n");
			choice = 0;
			int tmpC;
			while ((tmpC = getchar()) != EOF && tmpC != '\n');

			continue;
		}
		if (!choice) break;

		if (choice == 1) {
			ExpressionParser();
		}
		else if (choice == 2) {
			FindNumber();
		}
		else if (choice == 3) {
			SortNumbers();
		}

		printf("\n");
		errno = 0;
	}

	return 0;
}