#ifndef hawthorn_obj
#define hawthorn_obj

#include "chunk/chunk.h"

#include <stddef.h>
#include <stdint.h>

typedef uint32_t hash;

typedef enum
{
	OBJ_STRING,
	OBJ_FUNCTION,
	OBJ_NATIVE,
} ObjType;

struct Obj
{
	ObjType		type;
	struct Obj* next;
};

struct ObjString
{
	struct Obj
		obj;   // type punning. Because C actually supports this kind of
			   // thing it enables smart technique. You can safely convert
			   // ObjString To Obj* and get the type field and others
	int	  length;
	hash  hash;
	char* chars;
};
struct ObjFunction
{
	struct Obj		  obj;
	int				  argc;
	Chunk			  chunk;   // function body
	struct ObjString* name;
};

typedef struct TValue (*Native)(int argc, struct TValue* args);

struct ObjNative
{
	struct Obj obj;
	Native	   function;
};

typedef struct Obj		   Obj;
typedef struct ObjString   haw_string;
typedef struct ObjFunction haw_function;
typedef struct ObjNative   haw_native;

#define allocate(t, c) (t*) malloc(sizeof(t) * c)

#define cast_obj(o) cast(Obj*, o)
#define cast_string(o) cast(haw_string*, o)
#define cast_function(o) cast(haw_function*, o)
#define cast_native(o) cast(haw_native*, o)

// work with strings
haw_string* copy_string(char* chars, int length, int* constant_index);
haw_string* concatenate(haw_string* a, haw_string* b);
haw_string* take_string(char* chars, int length, int* constant_index);

haw_function* new_function();
haw_native*	  new_native(Native fun);

// general use
void objects_free();
void object_free(Obj* obj);

Obj* allocate_object(size_t size, ObjType type);

#define allocate_obj(type, object_type)                                   \
	(type*) allocate_object(sizeof(type), object_type)

#define allocate_obj_fam(size, type, object_type)                         \
	(type*) allocate_object(size, object_type)

#endif
