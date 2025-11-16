#include <share/string.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int8_t str_eq(str* string, const str* other_str)
{
	if (string->length != other_str->length)
	{
		return STR_NEQ;
	}

	for (str_size i = 0; i < string->length; i++)
	{
		if (string->value[i] == other_str->value[i])
		{
			continue;
		}

		else
		{
			return STR_NEQ;
		}
	}

	return STR_EQ;
}

str make_str(cstr c_string)
{
	str_size   length = strlen(c_string);
	base_char* value  = c_string;

	return (str) {.length = length, .value = value};
}

String make_String(cstr initial_string)
{
	str_size	   length	= strlen(initial_string);
	base_char_mut* value	= strndup(initial_string, length);
	str_size	   capacity = CAP_INITIAL;

	if (length > CAP_INITIAL)
	{
		while (capacity <= length)
		{
			capacity *= CAP_MULTIPLIER;
		}
	}

	return (String) {.length = length, .capacity = capacity, .value = value};
}

void String_init(String* string)
{
	string->length	 = 0;
	string->capacity = CAP_INITIAL;
	string->value	 = NULL;
}

void String_append(String* string, cstr append_str)
{
	str_size append_size	 = strlen(append_str) + 1; // \0 for string.h compat
	str_size needed_capacity = string->length + append_size;

	if (string->capacity < needed_capacity)
	{
		while (string->capacity <= needed_capacity)
		{
			string->capacity *= CAP_MULTIPLIER;
		}

		string->value = realloc(string->value, string->capacity);
		string->value = strcat(string->value, append_str);
	}
	else
	{
		string->value = strcat(string->value, append_str);
	}

	string->length += append_size;
	string->value[string->length] = '\0';
}

void String_appendc(String* string, char c)
{
	if (string->length + 1 >= string->capacity)
	{
		string->capacity *= CAP_MULTIPLIER;
		string->value = realloc(string->value, string->capacity);
	}

	string->value[string->length++] = c;
}

void String_destroy(String* string)
{
	string->capacity = 0;
	string->length	 = 0;

	free(string->value);
	string->value = NULL;
}
