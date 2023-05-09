#include "vector.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

vector_t *CreateVector(size_t itemSize) {
	if (!itemSize) return NULL;

	vector_t *vec = malloc(sizeof(vector_t));
	if (!vec) return NULL;

	vec->capacity = 1;
	vec->len = 0;
	vec->itemSize = itemSize;

	vec->data = calloc(vec->capacity, sizeof(itemSize));
	if (!vec->data) {
		free(vec);
		return NULL;
	}

	return vec;
}
void DeleteVector(vector_t *vec) {
	if (!vec) return;
	
	if (vec->data) free(vec->data);
	free(vec);
}

void VectorPushBack(vector_t *vec, void *item) {
	if (!vec || !item) return;

	if (vec->len == vec->capacity) {
		vec->capacity += (size_t)(vec->capacity * 0.5) + 1;
		void *tmp = realloc(vec->data, (vec->itemSize * vec->capacity));
		if (!tmp) return;

		vec->data = tmp;
	}

	memcpy(
		(void *)((uintptr_t)vec->data + (vec->itemSize * vec->len++)),
		item,
		vec->itemSize
	);
}
void VectorInsert(vector_t *vec, size_t index, void *item) {
	if (!vec || !vec->data || !item || index > vec->len) return;
	if (index == vec->len) {
		VectorPushBack(vec, item);
		return;
	}

	if (vec->len == vec->capacity) {
		vec->capacity += (size_t)(vec->capacity * 0.5) + 1;
		void *tmp = realloc(vec->data, vec->capacity * vec->itemSize);
		if (!tmp) return;

		vec->data = tmp;
	}

	void *tmpArr = calloc(vec->capacity, vec->itemSize);
	if (!tmpArr) return NULL;

	memcpy(tmpArr, vec->data, index * vec->itemSize);
	memcpy(
		(void*)((uintptr_t)tmpArr + (vec->itemSize * index)),
		item, 
		vec->itemSize
	);
	memcpy(
		(void *)((uintptr_t)tmpArr + (vec->itemSize * (index + 1))),
		(void *)((uintptr_t)vec->data + (vec->itemSize * index)),
		(vec->len - index) * vec->itemSize
	);

	free(vec->data);

	vec->len++;
	vec->data = tmpArr;
}

void *VectorAt(vector_t *vec, size_t index) {
	if (!vec || index >= vec->len) return NULL;

	return (void *)((uintptr_t)vec->data + (vec->itemSize * index));
}

vector_t *CopyVector(vector_t *original) {
	if (!original) return NULL;

	vector_t *vec = calloc(1, sizeof(vector_t));
	if (!vec) return NULL;

	vec->itemSize = original->itemSize;
	vec->capacity = original->capacity;
	vec->len = original->len;

	if (original->data) {
		vec->data = calloc(original->capacity, original->itemSize);
		if (!vec->data) {
			free(vec);
			return NULL;
		}

		memcpy(vec->data, original->data, vec->itemSize * vec->len);
	}

	return vec;
}

void VectorErase(vector_t *vec, size_t index) {
	if (!vec || index >= vec->len) return;

	memcpy(
		(void *)((uintptr_t)vec->data + (vec->itemSize * index)),
		(void *)((uintptr_t)vec->data + (vec->itemSize * (index + 1))),
		vec->itemSize * (vec->len - index - 1)
	);

	vec->len--;
}
void VectorClear(vector_t *vec) {
	if (!vec) return;

	vec->len = 0;
}