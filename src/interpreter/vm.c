#include "interpreter/vm.h"

#include "chunk/chunk.h"
#include "chunk/opcodes.h"
#include "lexer/lexer.h"
#include "share/array.h"
#include "type/type.h"
#include "value/obj.h"
#include "value/value.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif
#define MODULE_NAME "vm"

vm v;

#define pop() array_pop(v.stack, TValue)
#define push(valu) array_push(v.stack, valu)

#define read_byte() v.chunk->code[v.pc++]

#define wrongoperandsc(operand) errorf("Wrong operands to binary operator `%c`", operand)

void vm_init(Chunk* chunk)
{
	v.stack	  = array(TValue);
	v.chunk	  = chunk;
	v.pc	  = 0;
	v.objects = NULL;
}

size_t chunk_size()
{
	size_t size = array_size(v.chunk->code);

	return size;
}

void vm_execute()
{
	for (;;)
	{
		uint8_t instruction = read_byte();
		if (instruction == OP_HALT)
		{
			return;
		}

		switch (instruction)
		{
		case OP_CONSTANT:
		{
			uint8_t index = read_byte();
			push(v.chunk->constants[index]);
			break;
		}
		case OP_CONSTANT_LONG:
		{
			uint8_t	 major = read_byte();
			uint8_t	 mid   = read_byte();
			uint8_t	 minor = read_byte();
			uint32_t index = major | (uint32_t) mid << 8 | (uint32_t) minor << 16;
			push(v.chunk->constants[index]);
			break;
		}
		case OP_PRINT:
		{
			print_value(&pop());
			printf("\n");
			break;
		}

		// Binary Operators
#define macrostart()                                                                               \
	TValue result;                                                                                 \
	TValue b = pop();                                                                              \
	TValue a = pop()

#define macroend()                                                                                 \
	push(result);                                                                                  \
	break;

#define binopr(t, op, ch)                                                                          \
	case t:                                                                                        \
	{                                                                                              \
		macrostart();                                                                              \
                                                                                                   \
		if (t_isrational(&a) && t_isrational(&b))                                                  \
		{                                                                                          \
			if (t_isint(&a) && t_isint(&b))                                                        \
			{                                                                                      \
				result.type = HAW_TINT;                                                            \
				setivalue(&result, int_value(&a) op int_value(&b));                                \
			}                                                                                      \
			else                                                                                   \
			{                                                                                      \
				result.type = HAW_TNUMBER;                                                         \
				setnvalue(&result, val_to_num(&a) op val_to_num(&b));                              \
			}                                                                                      \
		}                                                                                          \
		else                                                                                       \
		{                                                                                          \
			wrongoperandsc(ch);                                                                    \
		}                                                                                          \
		macroend();                                                                                \
	}
			binopr(OP_ADD, +, '+');
			binopr(OP_SUB, -, '-');
			binopr(OP_DIV, /, '/');

		case OP_MUL:
		{
			macrostart();

			if (t_isint(&a) && t_isint(&b))
			{
				result.type = HAW_TINT;
				setivalue(&result, int_value(&a) % int_value(&b));
			}
			else if (t_isrational(&a) && t_isrational(&b))
			{
				result.type = HAW_TNUMBER;
				setnvalue(&result, val_to_num(&a) * val_to_num(&b));
			}

			else if (t_isstring(&a) && t_isint(&b))
			{
				result.type = HAW_TOBJECT;
				int count	= int_value(&b);

				if (count < 1)
				{
					error("Wrong counter");
				}

				int len	  = string_value(&a)->length;
				int total = len * count;

				size_t chars_size = sizeof(char) * (total + 1);

				haw_string* string =
					allocate_obj_fam(sizeof(*string) + chars_size, haw_string, OBJ_STRING);

				string->length = total;
				string->chars  = (char*) (string + 1);

				char* dest = string->chars;
				for (int i = 0; i < count; i++)
				{
					if (len > 0)
					{
						memcpy(dest, string_value(&a)->chars, len);
						dest += len;
					}
				}

				string->chars[total] = '\0';

				setovalue(&result, string);
				obj_type(&result) = OBJ_STRING;
			}
			else
			{
				wrongoperandsc('*');
			}

			macroend();
		}

		case OP_MOD:
		{
			macrostart();

			if (t_isint(&a) && t_isint(&b))
			{
				result.type = HAW_TINT;
				setivalue(&result, int_value(&a) % int_value(&b));
			}
			else if (t_isrational(&a) && t_isrational(&b))
			{
				result.type = HAW_TNUMBER;
				setnvalue(&result, fmod(val_to_num(&a), val_to_num(&b)));
			}
			else
			{
				wrongoperandsc('%');
			}
			macroend();
		}
		case OP_IDIV:
		{
			macrostart();

			if (t_isint(&a) && t_isint(&b))
			{
				result.type = HAW_TINT;
				setivalue(&result, cast_hawint(int_value(&a) / int_value(&b)));
			}
			else if (t_isrational(&a) && t_isrational(&b))
			{
				result.type = HAW_TNUMBER;
				setnvalue(&result, fdiv(val_to_num(&a), val_to_num(&b)));
			}
			else
			{
				error("Wrong operands to binary operator `//`");
			}

			macroend();
		}
		case OP_POW:
		{
			macrostart();

			if (t_isrational(&a) && t_isrational(&b))
			{
				result.type = HAW_TNUMBER;
				setnvalue(&result, pow(val_to_num(&a), val_to_num(&b)));
			}

			else
			{
				wrongoperandsc('^');
			}

			macroend();
		}

#define compopr(t, f)                                                                              \
	case t:                                                                                        \
	{                                                                                              \
		macrostart();                                                                              \
		int res = valuecmp(&a, &b);                                                                \
		if (res == -3)                                                                             \
		{                                                                                          \
			error("Incompatible types for comparison");                                            \
		}                                                                                          \
                                                                                                   \
		result.type = HAW_TINT;                                                                    \
		setivalue(&result, f);                                                                     \
                                                                                                   \
		macroend();                                                                                \
	}

			compopr(OP_EQ, res == 0);

			compopr(OP_GE, res == 0 || res == -1);
			compopr(OP_GT, res == -1);

			compopr(OP_LE, res == 0 || res == 1);
			compopr(OP_LT, res == 1);

#define andoropr(t, op)                                                                            \
	case t:                                                                                        \
	{                                                                                              \
		macrostart();                                                                              \
		result.type = HAW_TINT;                                                                    \
		setivalue(&result, t_istruth(&a) op t_istruth(&b));                                        \
                                                                                                   \
		macroend();                                                                                \
	}

			andoropr(OP_AND, &&);
			andoropr(OP_OR, ||);

			// string concatenation
		case OP_CONCAT:
		{
			macrostart();
			result.type = HAW_TOBJECT;
			setovalue(&result, concatenate(string_value(&a), string_value(&b)));
			obj_type(&result) = OBJ_STRING;

			macroend();
		}

#undef binopr
#undef andoropr
#undef compopr
#undef macrostart
#undef macroend
		}
	}
}

void vm_destroy()
{
	chunk_destroy(v.chunk);
	free_objects();
	array_free(v.stack);
}

#undef pop
#undef push
