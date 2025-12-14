#include <stdlib.h>
#include <string.h>
#include <value/obj.h>

#define allocate_obj(type, objectType) (type*) allocate_object(sizeof(type), objectType)

static Obj* allocate_object(size_t size, ObjType type)
{
	Obj* object = allocate(Obj, 1);

	object->type = type;
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
