#ifndef hawthorn_obj
#define hawthorn_obj

#include <stddef.h>
typedef enum
{
	OBJ_STRING,
} ObjType;

struct Obj
{
	ObjType		type;
	struct Obj* next;
};

struct ObjString
{
	struct Obj obj; // type punning. Because C actually supports this kind of thing it enables smart
					// technique. You can safely convert ObjString To Obj* and get the type field
					// and others
	int	  length;
	char* chars;
};

typedef struct Obj		 Obj;
typedef struct ObjString haw_string;

#define allocate(t, c) (t*) malloc(sizeof(t) * c)

#define cast_obj(o) cast(Obj*, o)
#define cast_string(o) cast(haw_string*, o)

// work with strings
haw_string* copy_string(const char* chars, int length);
haw_string* concatenate(haw_string* a, haw_string* b);
haw_string* allocate_string(char* chars, int length);
haw_string* sallocate_string(char* chars, int length, int size);
// general use
void free_objects();
void free_object(Obj* obj);

Obj* allocate_object(size_t size, ObjType type);
#define allocate_obj(type, object_type) (type*) allocate_object(sizeof(type), object_type)

#define allocate_obj_fam(size, type, object_type) (type*) allocate_object(size, object_type)

#endif
