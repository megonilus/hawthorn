#include <interpreter/vm.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <value/obj.h>

#define endstring(s, l) (s)[(l)] = '\0'

Obj* allocate_object(size_t size, ObjType type)
{
	Obj* object = (Obj*) malloc(size);

	object->type = type;
	object->next = v.objects;
	v.objects	 = object;

	return object;
}

haw_string* allocate_string(char* chars, int length)
{
	haw_string* string =
		allocate_obj_fam(sizeof(haw_string) + sizeof(char) * length, haw_string, OBJ_STRING);

	string->length = length;
	string->chars  = chars;

	return string;
}

haw_string* copy_string(const char* chars, int length)
{
	size_t chars_size = sizeof(char) * (length + 1);

	haw_string* string = allocate_obj_fam(sizeof(*string) + chars_size, haw_string, OBJ_STRING);

	string->length = length;
	string->chars  = (char*) (string + 1);

	memcpy(string->chars, chars, length);
	endstring(string->chars, length);

	return string;
}

haw_string* concatenate(haw_string* a, haw_string* b)
{
	int			length	   = a->length + b->length;
	size_t		chars_size = sizeof(char) * (length + 1);
	haw_string* string =
		allocate_obj_fam(sizeof(*string) + chars_size, haw_string, OBJ_STRING); // allocation

	string->length = length;
	string->chars  = (char*) (string + 1);

	memcpy(string->chars, a->chars, a->length);				// copy
	memcpy(string->chars + a->length, b->chars, b->length); // copy

	endstring(string->chars, length);

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
