#ifndef megonil_string
#define megonil_string

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef int32_t str_size;

typedef const char base_char;
typedef char	   base_char_mut;

typedef const char* cstr;
typedef char*		cstr_mut;

typedef const unsigned char* ucstr;
typedef unsigned char*		 ucstr_mut;

typedef struct
{
	str_size   length;
	base_char* value;
} str;

#define this str* string
#define this_t str

str make_str(cstr c_string);

#define STR_EQ 1  // result value when strings are equal
#define STR_NEQ 0 // result value when string are not equal
#define STR_DL -1 // result value when string have different lengths

int8_t str_eq(this, const this_t* other_str);

typedef struct
{
	str_size	   length;
	str_size	   capacity;
	base_char_mut* value;
} String;

#define CAP_MULTIPLIER 2
#define CAP_INITIAL 8

String make_String(cstr initial_string);
void   String_init(String* string);
void   String_append(String* string, cstr append_str);
void   String_appendc(String* string, char c);
void   String_destroy(String* string);

#define String_eq(str1, str2) (strcmp(str1->value, str2->value) == 0)

#undef this
#undef this_t
#endif // !megonil_string
