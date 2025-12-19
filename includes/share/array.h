#ifndef megonil_vector
#define megonil_vector

#include <stddef.h>
#define ARRAY_INITIAL_CAPACITY 8
#define ARRAY_GROW_COEFF 2

typedef void(PrintFunction)(void*);

typedef struct
{
	size_t capacity;
	size_t size;
} ArrayHeader;

// graceful array_init wrapper
#define array(T) (T*) array_init(sizeof(T), ARRAY_INITIAL_CAPACITY)
// get the header
#define array_header(ARR) ((ArrayHeader*) (ARR) - 1)
// vector.at c++ analog (indexing with bound checking)
#define array_at(ARR, INDEX, T) (T*) array_get(ARR, INDEX)

#define array_size(ARR) (array_header(ARR)->size)
#define array_capacity(ARR) (array_header(ARR)->capacity)

#define array_empty(ARR) array_size((ARR)) == 0

#define array_pop(ARR, T) (((T*) (ARR))[--array_header(ARR)->size])

#define array_push(ARR, VALUE)                                                                     \
	ARR							   = array_ensure_capacity(ARR, 1, sizeof(VALUE));                 \
	ARR[array_header(ARR)->size++] = (VALUE);

#define array_reserve(ARR, n, T) ARR = array_res(ARR, (n), sizeof(T))

#define array_incsize(ARR) array_size(ARR)++;

#define array_ensure(ARR, count, size) ARR = array_ensure_capacity(ARR, count, size);

// main functions
void* array_init(size_t item_size, size_t capacity);
void  array_pop_back(void* array);

/// ensure that we have enough space to push.
/// also manages reallocation.
void* array_ensure_capacity(void* array, size_t item_count, size_t item_size);

// various utilities
void  array_print(void* array, PrintFunction print);
void  array_free(void* array);
void* array_res(void* array, size_t n, size_t item_size);
void* array_get(void* array, size_t index);

#endif // !megonil_vector
