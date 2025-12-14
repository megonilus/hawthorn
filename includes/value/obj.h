#ifndef hawthorn_obj
#define hawthorn_obj

typedef enum
{
	OBJ_STRING,
} ObjType;

struct Obj
{
	ObjType type;
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

haw_string* copy_string(const char* chars, int length);

#endif
