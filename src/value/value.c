#include <complex.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <value/obj.h>
#include <value/value.h>

static void print_object(const TValue* value)
{
	switch (obj_value(value)->type)
	{
	case OBJ_STRING:
		printf("%s", cstring_value(value));
		break;
	default:
		unreachable();
	}
}
void print_value(const TValue* value)
{
	switch (value->type)
	{
	case HAW_TNONE:
		printf("<none>");
		break;
	case HAW_TVOID:
		printf("<void>");
		break;
	case HAW_TINT:
		printf("%d", int_value(value));
		break;
	case HAW_TNUMBER:
		printf("%g", number_value(value));
		break;
	case HAW_TOBJECT:
		print_object(value);
		break;
	default:
		unreachable();
	}
}

///  0  if values equal
/// -1 if left is bigger
///  1  if right is bigger
/// -2 if left or right is an object
/// -3 if none of this
int valuecmp(const TValue* left, const TValue* right)
{
	if (valueeq(left, right))
	{
		return 0;
	}
	else
	{
		if (t_isint(left) && t_isint(right))
		{
			return int_value(left) > int_value(right) ? -1 : 1;
		}

		else if (t_isrational(left) && t_isrational(right))
		{
			return val_to_num(left) > val_to_num(right) ? -1 : 1;
		}
		else if (t_isobject(left) || t_isobject(right))
		{
			return -2;
		}

		return -3;
	}
}

int valueeq(const TValue* left, const TValue* right)
{
	if (!t_issame(left, right))
	{
		return 0;
	}

	else
	{
		switch (left->type)
		{
		case HAW_TINT:
			return int_value(left) == int_value(right);
		case HAW_TNUMBER:
			return number_value(left) == number_value(right);
		case HAW_TOBJECT:
		{
			switch (obj_type(left))
			{
			case OBJ_STRING:
				haw_string* a = string_value(left);
				haw_string* b = string_value(right);

				return a->length == b->length && memcmp(a, b, a->length) == 0;
			}
		}

		// TODO
		case HAW_TFN:
		case HAW_TTHREAD:
			break;
		default:
			unreachable();
		}
	}
	unreachable();
}
