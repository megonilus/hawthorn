#include "chunk/opcodes.h"
#include "type/type.h"
#include "value/value.h"

#include <chunk/chunk.h>
#include <stdint.h>
#include <stdio.h>

void chunk_init(Chunk* chunk)
{
	chunk->code		 = array(hawu_byte);
	chunk->constants = array(TValue);
}

void emit_byte(this, hawu_byte byte)
{
	array_push(chunk->code, byte);
}

void chunk_destroy(this)
{
	array_free(chunk->code);
	array_free(chunk->constants);
}

static void print_value(const TValue* v)
{
	if (t_isint(v))
	{
		printf("%d", int_value(v));
	}
	else if (t_isnumber(v))
	{
		printf("%f", (double) number_value(v));
	}
	else if (t_isstring(v))
	{
		String_print(string_value(v));
	}
	else if (t_isvoid(v))
	{
		printf("<void>");
	}
	else
	{
		printf("<unknown type %d>", v->type);
	}
}

static void simple_instruction(OpCodes opcode, int* offset)
{
	printf("%4d %s\n", *offset, op_name(opcode));
	(*offset)++;
}

static void constant_instruction(Chunk* chunk, int* offset)
{
	uint8_t index = chunk->code[*offset + 1];

	printf("%4d %s %hu (", *offset, op_name(OP_CONSTANT), index);
	print_value(&chunk->constants[index]);
	printf(")\n");

	(*offset) += 2;
}

static void constantlong_instruction(Chunk* chunk, int* offset)
{
	uint32_t index = (uint32_t) chunk->code[*offset + 1] |
					 (uint32_t) chunk->code[*offset + 2] << 8 |
					 (uint32_t) chunk->code[*offset + 3] << 16;

	printf("%04d %s %3u (", *offset, op_name(OP_CONSTANT_LONG), index);

	if (index < array_size(chunk->constants))
	{
		print_value(&chunk->constants[index]);
	}
	else
	{
		printf("Invalid index: %u", index);
	}
	printf(")\n");

	(*offset) += 4;
}

void disassemble(Chunk* chunk)
{
	printf("-- Bytecode\n");
	printf("   ");
	for (int i = 0; i < array_size(chunk->code); i++)
	{
		printf("%02X ", chunk->code[i]);
	}

	printf("\n");

	for (int offset = 0; offset < array_size(chunk->code);)
	{
		switch (chunk->code[offset])
		{
		case OP_CONSTANT:
			constant_instruction(chunk, &offset);
			break;
		case OP_CONSTANT_LONG:
			constantlong_instruction(chunk, &offset);
			break;
		default:
			simple_instruction(chunk->code[offset], &offset);
			break;
		}
	}
}

uint32_t add_constant(Chunk* chunk, Constant data)
{
	array_push(chunk->constants, data);

	return array_size(chunk->constants) - 1;
}

uint32_t write_constant(Chunk* chunk, Constant data)
{
	uint32_t index = add_constant(chunk, data);

	if (index <= UINT8_MAX)
	{
		emit_byte(chunk, OP_CONSTANT);
		emit_byte(chunk, index);

		return index;
	}
	else
	{
		emit_byte(chunk, OP_CONSTANT_LONG);
		emit_byte(chunk, index & 0xFF);
		emit_byte(chunk, (index >> 8) & 0xFF);
		emit_byte(chunk, (index >> 16) & 0xFF);
	}

	return index;
}
