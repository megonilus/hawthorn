#include "share/table.h"
#include "value/value.h"

#include <interpreter/vm.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <value/obj.h>
#define endstring(s, l) (s)[(l)] = '\0'

static uint32_t hash_string(const char* key, int length)
{
	uint32_t hash = 2166136261u;

	for (int i = 0; i < length; i++)
	{
		hash ^= (uint8_t) key[i];
		hash *= 16777619;
	}

	return hash;
}

Obj* allocate_object(size_t size, ObjType type)
{
	Obj* object = (Obj*) malloc(size);

	if (object == NULL)
	{
		fprintf(stderr, "Critical memory allocation error! obj.c\n");
		exit(EXIT_FAILURE);
	}

	object->type = type;
	object->next = v.objects;
	v.objects	 = object;

	return object;
}

static haw_string* allocate_string(hash hash, int length, int* constant_index)
{
	haw_string* string =
		allocate_obj_fam(sizeof(haw_string) + sizeof(char) * (length + 1), haw_string, OBJ_STRING);

	string->length = length;
	string->hash   = hash;
	string->chars  = (char*) (string + 1);

	if (constant_index != NULL)
	{
		table_set(&v.strings, string, v_int(*constant_index));
	}
	else
	{
		table_set(&v.strings, string, v_void());
	}

	return string;
}

haw_string* take_string(char* chars, int length, int* constant_index)
{
	hash		hash	 = hash_string(chars, length);
	haw_string* interned = table_find_string(&v.strings, chars, length, hash, NULL);

	if (interned != NULL)
	{
		return interned;
	}

	haw_string* string = allocate_string(hash, length, constant_index);
	memmove(string->chars, chars, length);

	return string;
}

haw_string* copy_string(char* chars, int length, int* constant_index)
{
	hash		hash	 = hash_string(chars, length);
	haw_string* interned = table_find_string(&v.strings, chars, length, hash, NULL);

	if (interned != NULL)
	{
		return interned;
	}

	haw_string* string = allocate_string(hash, length, constant_index);

	memcpy(string->chars, chars, string->length);
	endstring(string->chars, string->length);

	return string;
}

haw_string* concatenate(haw_string* a, haw_string* b)
{
	int	   length	  = a->length + b->length;
	size_t chars_size = sizeof(char) * (length + 1);

	haw_string* string =
		allocate_obj_fam(sizeof(*string) + chars_size, haw_string, OBJ_STRING); // allocation

	string->length = length;
	string->chars  = (char*) (string + 1);

	memcpy(string->chars, a->chars, a->length);				// copy
	memcpy(string->chars + a->length, b->chars, b->length); // copy

	endstring(string->chars, length);

	string->hash = hash_string(string->chars, string->length);

	haw_string* interned =
		table_find_string(&v.strings, string->chars, string->length, string->hash, NULL);

	if (interned != NULL)
	{
		return interned;
	}

	table_set(&v.strings, string, v_void());

	return string;
}

void object_free(Obj* obj)
{
	switch (obj->type)
	{
	case OBJ_STRING:
		haw_string* string = cast_string(obj);
		free(string);

		break;
	}
}

void objects_free()
{
	Obj* object = v.objects;

	while (object != NULL)
	{
		Obj* next = object->next;
		object_free(object);
		object = next;
	}
}
