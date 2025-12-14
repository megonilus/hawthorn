#include <interpreter/vm.h>
#include <stdlib.h>
#include <string.h>
#include <value/obj.h>

#define allocate_obj(type, objectType) (type*) allocate_object(sizeof(type), objectType)

static Obj* allocate_object(size_t size, ObjType type)
{
	Obj* object = (Obj*) malloc(size);

	object->type = type;
	object->next = v.objects;
	v.objects	 = object;

	return object;
}

static haw_string* allocate_string(char* chars, int length)
{
	haw_string* string = allocate_obj(haw_string, OBJ_STRING);
	string->length	   = length;
	string->chars	   = chars;

	return string;
}

haw_string* copy_string(const char* chars, int length)
{
	char* heap_chars = allocate(char, length + 1);
	memcpy(heap_chars, chars, length);

	heap_chars[length] = '\0';
	return allocate_string(heap_chars, length);
}

haw_string* concatenate(haw_string* a, haw_string* b)
{
	int	  length = a->length + b->length;
	char* chars	 = allocate(char, length + 1);

	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);
	chars[length] = '\0';

	haw_string* result = allocate_string(chars, length);

	return result;
}

static void free_object(Obj* obj)
{
	switch (obj->type)
	{
	case OBJ_STRING:
		haw_string* string = cast_string(obj);

		free(string->chars);
		free(obj);
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
