#ifndef haw_value_h
#define haw_value_h

#include <type/type.h>

// values of all types are first-class values: we can store them
// in global variables, local variables and table fields, pass them as arguments to
// functions, return them from functions, etc.
// strings like userdata and gcobject will stored as a reference
typedef union
{
	// gcobject in bright future...
	// function...
	void* user_; // userdata are essentially pointers to user memory blocks, and come in 2 flavors:
				 // heavy(allocated by haw), light(allocated and freed by the user)
	haw_int	   int_;	// integer
	haw_number number_; // float/double
} Value;

// tagged values like in lua
typedef struct
{
	Value	 value_;
	HawTypes type_tag_;
} TValue;

#define HAW_VARIANT_INT (HAW_TNUMBER | (0 << 4))
#define HAW_VARIANT_NUMBER (HAW_TNUMBER | (1 << 4))

#define val_(o) ((o)->value_)
#define valraw(o) (val_(o))

#define novariant(t) ((t) & 0x0F)
#define withvariant(t) ((t) & 0x3F)

#define rawtt(o) ((o)->type_tag_)
#define ttypetag(o) withvariant(rawtt(o))
#define ttype(o) (novariant(rawtt(o)))

#define HAW_VARIANT(o) (withvariant(rawtt(o)))

// test your types
#define checktag(o, t) (rawtt(o) == (t))
#define checktype(o, t) (ttype(o) == (t))

#define cast_value(val) ((Value) val)

#define tt_isint(o) (HAW_VARIANT(o) == HAW_VARIANT_INT)
#define tt_isnumber(o) (HAW_VARIANT(o) == HAW_VARIANT_NUMBER)

#define int_value(o) val_(o).int_
#define number_value(o) val_(o).number_

#endif //! haw_value_h
