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
		fprintf(stderr, "Critical memory allocation error");
		exit(1);
	}

	object->type = type;
	object->next = v.objects;
	v.objects	 = object;

	return object;
}

static haw_string* allocate_string(hash hash, int length)
{
	haw_string* string =
		allocate_obj_fam(sizeof(haw_string) + sizeof(char) * (length + 1), haw_string, OBJ_STRING);

	string->length = length;
	string->hash   = hash;
	string->chars  = (char*) (string + 1);

	table_set(&v.strings, string, v_void());

	return string;
}

haw_string* take_string(char* chars, int length)
{
	hash		hash	 = hash_string(chars, length);
	haw_string* interned = table_find_string(&v.strings, chars, length, hash);

	if (interned != NULL)
	{
		return interned;
	}

	haw_string* string = allocate_string(length, hash);
	string->chars	   = chars;

	return string;
}

haw_string* copy_string(char* chars, int length)
{
	hash		hash	 = hash_string(chars, length);
	haw_string* interned = table_find_string(&v.strings, chars, length, hash);

	if (interned != NULL)
	{
		return interned;
	}

	haw_string* string = allocate_string(hash, length);

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
		table_find_string(&v.strings, string->chars, string->length, string->hash);

	if (interned != NULL)
	{
		return interned;
	}

	table_set(&v.strings, string, v_void());

	return string;
}

void free_object(Obj* obj)
{
	switch (obj->type)
	{
	case OBJ_STRING:
		haw_string* string = cast_string(obj);
		free(string);

		break;
	}
}

void free_objects()
{
	Obj* object = v.objects;

	while (object != NULL)
	{
		Obj* next = object->next;
		free_object(object);
		object = next;
	}
}
