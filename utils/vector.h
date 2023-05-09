#pragma once

#include <stddef.h>

typedef struct vector {
	void *data;
	size_t len;
	size_t capacity;
	size_t itemSize;
}vector_t;

vector_t *CreateVector(size_t itemSize);
void DeleteVector(vector_t *vec);

void VectorPushBack(vector_t *vec, void *item);
void VectorInsert(vector_t *vec, size_t index, void *item);

void *VectorAt(vector_t *vec, size_t index);

vector_t *CopyVector(vector_t *original);

void VectorErase(vector_t *vec, size_t index);
void VectorClear(vector_t *vec);