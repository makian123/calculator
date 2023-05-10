#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "utils/vector.h"
#include "tokens/token.h"
#include "tokens/tokenizer.h"
#include "evaluator/parser.h"

void ShellSort(double *arr, size_t n) {
	for (size_t gap = n / 2; gap > 0; gap /= 2) {
		for (size_t i = gap; i < n; ++i) {
			double tmp = arr[i];

			size_t j;
			for (j = i; j >= gap && arr[j - gap] > tmp; j -= gap) {
				arr[j] = arr[j - gap];
			}

			arr[j] = tmp;
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
	double target;

	while (getc(stdin) != '\n');
	printf("Unesite putanju datoteke: ");
	scanf("%256[^\n]", file);
	getc(stdin);

	FILE *fp = fopen(file, "r");
	if (!fp) {
		printf("Greska u otvaranju dokumenta\n");
	}

	size_t ctr = 0;
	double tmp = 0;
	while (fscanf(fp, " %lf", &tmp) == 1)ctr++;

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

	ShellSort(brojevi, ctr);

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
	free(brojevi);
	fclose(fp);
}

int main(void) {
	int choice;

	while(1) {
		printf("Unesite:\n");
		printf("\t[1] Proslijedivanje izraza\n");
		printf("\t[2] Trazenje broja\n");
		printf("\t[3] Sortiranje brojeva\n");
		printf("\t[0] Kraj\n> ");
		scanf(" %d", &choice);
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
	}

	return 0;
}