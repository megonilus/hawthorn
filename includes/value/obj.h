#ifndef hawthorn_obj
#define hawthorn_obj

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

#define cast_obj(o) cast(haw_string*, o)
#define cast_string(o) cast(haw_string*, o)

// work with strings
haw_string* copy_string(const char* chars, int length);
haw_string* concatenate(haw_string* a, haw_string* b);

// general use
void free_objects();

#endif
