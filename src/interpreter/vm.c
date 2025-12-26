#include "interpreter/vm.h"

#include "chunk/chunk.h"
#include "chunk/opcodes.h"
#include "lexer/lexer.h"
#include "share/array.h"
#include "share/table.h"
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
#define constants (v.chunk->constants)
#define code (v.chunk->code)

#define pop() array_pop(v.stack, TValue)
#define push(valu) array_push(v.stack, valu)

#define read_byte() code[v.pc++]

#define read_u24()                                                        \
	((v.pc += 3), (uint32_t) code[v.pc - 3] |                             \
					  ((uint32_t) code[v.pc - 2] << 8) |                  \
					  ((uint32_t) code[v.pc - 1] << 16))

#define read_short() (v.pc += 2, ((code[v.pc - 2] << 8) | code[v.pc - 1]));

#define read_wide()                                                       \
	if (wide)                                                             \
	{                                                                     \
		index = read_u24();                                               \
	}                                                                     \
	else                                                                  \
	{                                                                     \
		index = read_byte();                                              \
	}                                                                     \
	wide = 0;

#define wrongoperandsc(operand)                                           \
	errorf("Wrong operands to binary operator `%c`", operand)

void vm_init(Chunk* chunk)
{
	v.stack	  = array(TValue);
	v.chunk	  = chunk;
	v.pc	  = 0;
	v.objects = NULL;

	table_init(&v.strings);
	table_init(&v.globals);
}

size_t chunk_size()
{
	size_t size = array_size(code);

	return size;
}

void vm_execute()
{
	int index;
	int wide = 0;	// wide flag

	for (;;)
	{
		uint8_t instruction = read_byte();
		if (instruction == OP_HALT)
		{
			v.pc = 0;
			return;
		}

		switch (instruction)
		{
		case OP_WIDE:
		{
			wide = 1;
			continue;
		}

		case OP_JMPF:
		{
			uint16_t offset	   = read_short();
			TValue	 condition = pop();

			if (!v_istruth(&condition))
			{
				v.pc += offset;
			}

			break;
		}
		case OP_JMP:
		{
			uint16_t offset = read_short();
			v.pc += offset;

			break;
		}
		case OP_NJMP:
		{
			uint16_t offset = read_short();
			v.pc -= offset;

			break;
		}

		case OP_CONSTANT:
		{
			index = read_byte();
			push(constants[index]);
			break;
		}
		case OP_CONSTANT_LONG:
		{
			index = read_u24();

			push(constants[index]);
			break;
		}

		case OP_PRINT:
		{
			print_value(&pop());
			printf("\n");
			break;
		}

		case OP_SETGLOBAL:
		{
			TValue var_value = pop();
			read_wide();
			haw_string* var_name = string_value(&constants[index]);

			table_set(&v.globals, var_name, var_value);
			push(var_value);
			break;
		}

		case OP_LOADGLOBAL:
		{
			TValue var_value;
			read_wide();
			haw_string* var_name = string_value(&constants[index]);

			if (!table_get(&v.globals, var_name, &var_value))
			{
				error("Unknown variable");
			}

			push(var_value);
			break;
		}

		case OP_SETLOCAL:
		{
			read_wide();
			v.stack[index] = v.stack[array_size(v.stack) - 1];

			break;
		}
		case OP_LOADLOCAL:
		{
			read_wide();
			push(v.stack[index]);

			break;
		}

		case OP_POP:
			pop();
			break;

			// Binary Operators
#define macrostart()                                                      \
	TValue result;                                                        \
	TValue b = pop();                                                     \
	TValue a = pop()

#define macroend()                                                        \
	push(result);                                                         \
	break;

#define binopr(t, op, ch)                                                 \
	case t:                                                               \
	{                                                                     \
		macrostart();                                                     \
                                                                          \
		if (t_isrational(&a) && t_isrational(&b))                         \
		{                                                                 \
			if (t_isint(&a) && t_isint(&b))                               \
			{                                                             \
				result.type = HAW_TINT;                                   \
				setivalue(&result, int_value(&a) op int_value(&b));       \
			}                                                             \
			else                                                          \
			{                                                             \
				result.type = HAW_TNUMBER;                                \
				setnvalue(&result, val_to_num(&a) op val_to_num(&b));     \
			}                                                             \
		}                                                                 \
		else                                                              \
		{                                                                 \
			wrongoperandsc(ch);                                           \
		}                                                                 \
		macroend();                                                       \
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
				setivalue(&result, int_value(&a) * int_value(&b));
			}
			else if (t_isrational(&a) && t_isrational(&b))
			{
				result.type = HAW_TNUMBER;
				setnvalue(&result, val_to_num(&a) * val_to_num(&b));
			}

			else if (t_isstring(&a) && t_isint(&b))
			{
				haw_string* toadd = string_value(&a);
				result.type		  = HAW_TOBJECT;
				int count		  = int_value(&b);

				if (count < 1)
				{
					error("Wrong counter");
				}

				int len	  = string_value(&a)->length;
				int total = len * count;

				size_t chars_size = sizeof(char) * (total + 1);
				char*  new_chars  = (char*) malloc(chars_size);

				for (int i = 0; i < count; i++)
				{
					memcpy(new_chars + (i * len), toadd->chars, len);
				}
				new_chars[total] = '\0';

				haw_string* string = take_string(new_chars, total, NULL);
				string->length	   = total;

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
				setivalue(&result,
						  cast_hawint(int_value(&a) / int_value(&b)));
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

#define compopr(t, f)                                                     \
	case t:                                                               \
	{                                                                     \
		macrostart();                                                     \
		int res = valuecmp(&a, &b);                                       \
		if (res == -3)                                                    \
		{                                                                 \
			error("Incompatible types for comparison");                   \
		}                                                                 \
                                                                          \
		result.type = HAW_TINT;                                           \
		setivalue(&result, f);                                            \
                                                                          \
		macroend();                                                       \
	}

			compopr(OP_EQ, res == 0);

			compopr(OP_GE, res == 0 || res == -1);
			compopr(OP_GT, res == -1);

			compopr(OP_LE, res == 0 || res == 1);
			compopr(OP_LT, res == 1);

#define andoropr(t, op)                                                   \
	case t:                                                               \
	{                                                                     \
		macrostart();                                                     \
		result.type = HAW_TINT;                                           \
		setivalue(&result, v_istruth(&a) op v_istruth(&b));               \
                                                                          \
		macroend();                                                       \
	}

			andoropr(OP_AND, &&);
			andoropr(OP_OR, ||);

			// string concatenation
		case OP_CONCAT:
		{
			macrostart();

			if (!t_isstring(&a) || !t_isstring(&b))
			{
				error("Wrong operands to concatenation operator");
			}

			result.type = HAW_TOBJECT;
			setovalue(&result,
					  concatenate(string_value(&a), string_value(&b)));
			obj_type(&result) = OBJ_STRING;

			macroend();
		}

		case OP_NEG:
		{
			TValue a = pop();

			switch (a.type)
			{
			case HAW_TINT:
				push(v_int(-int_value(&a)));
				break;
			case HAW_TNUMBER:
				push(v_num(-number_value(&a)));
				break;
			default:
				error("Expected number or integer");
			}
			break;
		}

		case OP_NOT:
		{
			TValue a = pop();

			push(v_int(!v_istruth(&a)));
			break;
		}

#undef binopr
#undef andoropr
#undef compopr
#undef macrostart
#undef macroend
		}

		wide = 0;
	}
}

inline void vm_destroy()
{
	table_destroy(&v.strings);
	table_destroy(&v.globals);

	chunk_destroy(v.chunk);
	v.chunk = NULL;

	if (!(array_empty(v.stack)))
	{
		printf("warning: stack is not empty at the end of runtime\n");
	}

	array_free(v.stack);
	v.stack = NULL;

	objects_free();
}

#undef pop
#undef push
