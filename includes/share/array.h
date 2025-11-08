#ifndef megonil_vector
#define megonil_vector

#include "common.h"

#define ARRAY_INITIAL_CAPACITY 8
#define ARRAY_GROW_COEFF 2

typedef void* (*PrintFunction)(void*);

typedef struct
{
	size_t capacity;
	size_t size;
} ArrayHeader;

// array_init wrapper
#define array(T) (T*) array_init(sizeof(T), ARRAY_INITIAL_CAPACITY)
// get the header
#define array_header(ARR) ((ArrayHeader*) (ARR) - 1)
// vector.at c++ analog (with bound checking)
// also array_get wrapper
#define array_at(ARR, INDEX, T)                                                                    \
	(index >= 0 && index < array_size(ARR) ? (T*) ARR[INDEX] : raise(SIGTRAP))

#define array_size(ARR) (array_header(ARR)->size)
#define array_capacity(ARR) (array_header(ARR)->capacity)

#define array_empty(ARR) array_size((ARR)) == 0

#define array_pop(ARR) ()

#define array_push(ARR, VALUE)                                                                     \
	ARR							   = array_ensure_capacity(ARR, 1, sizeof(VALUE));                 \
	ARR[array_header(ARR)->size++] = (VALUE);

// main functions
void* array_init(size_t item_size, size_t capacity);
void* array_get(void* array, size_t index);
void  array_pop_back(void* array);

/// ensure that we have enough space to push.
/// also manages reallocation.
void* array_ensure_capacity(void* array, size_t item_count, size_t item_size);

// various utilities
void array_print(void* array, PrintFunction print);
void array_free(void* array);

#endif // !megonil_vector
