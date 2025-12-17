#ifndef haw_value_h
#define haw_value_h

#include "obj.h"

#include <stdint.h>
#include <type/type.h>

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif
#define MODULE_NAME "value"

// values of all types are first-class values: we can store them
// in global variables, local variables and table fields, pass them as arguments to
// functions, return them from functions, etc.
typedef union
{
	// gcobject in bright future...
	// function...
	haw_int	   int_;	// integer
	haw_number number_; // float/double
	Obj*	   obj_;
} Value;

// tagged values like in lua
typedef struct
{
	Value	value_;
	HawType type;
} TValue;

#define checktype(t1, t2) ((t1) == (t2))
#define val_(o) ((o))

#define t_objtype(o) (obj_value(o)->type)
#define t_isobjtype(o, t) (t_objtype(o) == t)

#define t_isint(o) checktype(val_(o)->type, HAW_TINT)
#define t_isnumber(o) checktype(val_(o)->type, HAW_TNUMBER)
#define t_isobject(o) checktype(val_(o)->type, HAW_TOBJECT)
#define t_isvoid(o) checktype(val_(o)->type, HAW_TVOID)

#define t_isvalid(o) (val_(o)->type > HAW_TNONE)

#define t_isrational(o) (t_isnumber(o) || t_isint(o))

#define int_value(o) (o)->value_.int_
#define number_value(o) (o)->value_.number_
#define string_value(o) ((haw_string*) (o)->value_.obj_)
#define cstring_value(o) string_value(o)->chars

#define obj_value(o) (o)->value_.obj_
#define t_issame_objtype(a, b) (obj_value(a)->type == obj_value(b)->type)
#define t_issame(a, b)                                                                             \
	(t_isvalid(a) && t_isvalid(b)) &&                                                              \
		(checktype(val_(a)->type, val_(b)->type) || t_issame_objtype(a, b))

#define obj_type(o) obj_value(o)->type

#define t_isstring(o) is_objtype(o, OBJ_STRING)

#define setnvalue(o, v) number_value(o) = v
#define setivalue(o, v) int_value(o) = v
#define setovalue(o, v) obj_value(o) = (Obj*) v

#define set_objtype(o, t) obj_value(o)->type = t

#define v_void() ((TValue) {.type = HAW_TVOID})

void print_value(const TValue* value);

static inline haw_number val_to_num(const TValue* v)
{
	return t_isint(v) ? (haw_number) int_value(v) : number_value(v);
}

static inline int is_objtype(const TValue* value, ObjType type)
{
	return t_isobject(value) && obj_value(value)->type == type;
}

int valuecmp(const TValue* left, const TValue* right);
int valueeq(const TValue* left, const TValue* right);

static inline int t_istruth(const TValue* v)
{
	switch (v->type)
	{
	case HAW_TINT:
		return int_value(v) != 0;
	case HAW_TNUMBER:
		return number_value(v) != 0.0;
	case HAW_TOBJECT:
		return 1;
	default:
		return 0;
	}
}

#endif //! haw_value_h
