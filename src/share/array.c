#include <share/array.h>
#include <share/error.h>
#include <stdlib.h>

Module module = {.name = "vector"};

void* array_init(size_t item_size, size_t capacity)
{
	// future pointer to the array
	void* ptr = 0;
	// needed size of the array
	size_t		 size	= (item_size * capacity) + sizeof(ArrayHeader);
	ArrayHeader* header = malloc(size);

	if (header)
	{
		header->capacity = capacity;
		header->size	 = 0;

		// array stays in front of the header
		ptr = header + 1;
	}

	else
	{
		error("failed to create header");
	}

	return ptr;
}

void* array_ensure_capacity(void* array, size_t item_count, size_t item_size)
{
	// get array header
	ArrayHeader* header			 = array_header(array);
	size_t		 needed_capacity = header->size + item_count;

	// check sizes
	// and reallocate
	if (header->capacity < needed_capacity)
	{
		size_t new_capacity = header->capacity * 2;
		size_t new_size		= header->size + item_count * item_size;

		while (new_capacity <= needed_capacity)
		{
			new_capacity *= 2;
		}

		header = realloc(header, sizeof(ArrayHeader) + new_capacity * item_size);
	}

	// array starts in front of header
	return header + 1;
}

void array_pop_back(void* array)
{
	ArrayHeader* header = array_header(array);

	if (array_empty(array))
	{
		error("tried to pop on the empty array!");
	}
}

void array_free(void* array)
{
	free(array_header(array));
}
