#include "chunk/opcodes.h"
#include "type/type.h"
#include "value/value.h"

#include <chunk/chunk.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

inline void chunk_init(Chunk* chunk)
{
	chunk->code		 = array(hawu_byte);
	chunk->constants = array(TValue);
}

inline void emit_byte(this, hawu_byte byte)
{
	array_push(chunk->code, byte);
}

void emit_some_bytes(this, ...)
{
	va_list bytes;
	va_start(bytes, chunk);

	int byte;
	for (; (byte = va_arg(bytes, int)) != -1;)
	{
		emit_byte(chunk, (uint8_t) byte);
	}

	va_end(bytes);
}

inline void chunk_destroy(this)
{
	array_free(chunk->code);
	array_free(chunk->constants);

	chunk->code		 = NULL;
	chunk->constants = NULL;
}

static void simple_instruction(OpCode opcode, int* offset)
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

static void slot_instruction(Chunk* chunk, int* offset)
{
	printf("%4d %s %u\n", *offset, op_name(chunk->code[*offset]),
		   chunk->code[*offset + 1]);

	*offset += 2;
}

static void constantlong_instruction(Chunk* chunk, int* offset)
{
	uint32_t index =
		from_u24(chunk->code[*offset + 1], chunk->code[*offset + 2],
				 chunk->code[*offset + 3]);

	printf("%4d %s %3u (", *offset, op_name(OP_CONSTANT_LONG), index);

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

static void onearg_instruction(Chunk* chunk, int* offset, int wide)
{
	int index;
	int jump;
	if (wide)
	{
		index =
			from_u24(chunk->code[*offset + 1], chunk->code[*offset + 2],
					 chunk->code[*offset + 3]);
		jump = 4;
	}
	else
	{
		index = chunk->code[*offset + 1];
		jump  = 2;
	}

	printf("%4d %s %hu (", *offset, op_name(chunk->code[*offset]), index);
	if (index < array_size(chunk->constants))
	{
		print_value(&chunk->constants[index]);
	}
	else
	{
		printf("Invalid index: %u", index);
	}

	printf(")\n");
	*offset += jump;
}

static void short_instruction(Chunk* chunk, int* offset)
{
	printf("%4d %s %u\n", *offset, op_name(chunk->code[*offset]),
		   chunk->code[*offset + 1] | chunk->code[*offset + 2] & 0xFF);

	*offset += 3;
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

	int wide = 0;

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
		case OP_WIDE:
			wide = 1;
			simple_instruction(chunk->code[offset], &offset);
			continue;
		case OP_SETGLOBAL:
		case OP_LOADGLOBAL:
			onearg_instruction(chunk, &offset, wide);
			break;
		case OP_SETLOCAL:
		case OP_LOADLOCAL:
			slot_instruction(chunk, &offset);
			break;
		case OP_JMP:
		case OP_NJMP:
		case OP_JMPF:
			short_instruction(chunk, &offset);
			break;
		default:
			simple_instruction(chunk->code[offset], &offset);
			break;
		}
		wide = 0;
	}
}

inline uint32_t add_constant(Chunk* chunk, Constant data)
{
	array_push(chunk->constants, data);

	return array_size(chunk->constants) - 1;
}

uint32_t raw_write_constant(Chunk* chunk, uint32_t index)
{
	if (index <= UINT8_MAX)
	{
		emit_bytes(chunk, OP_CONSTANT, index);
	}
	else
	{
		to_u24(index);
		emit_bytes(chunk, OP_CONSTANT_LONG, major, mid, minor);
	}

	return index;
}

inline uint32_t write_constant(Chunk* chunk, Constant data)
{
	return raw_write_constant(chunk, add_constant(chunk, data));
}

inline void chunk_clear(Chunk* chunk)
{
	array_size(chunk->code) = 0;
}

inline uint32_t emit_jump(Chunk* chunk, uint8_t jmp)
{
	emit_bytes(chunk, jmp, 0xFF, 0xFF);

	return array_size(chunk->code) - 2;
}

inline void patch_jump(this, uint32_t offset)
{
	uint32_t jump = array_size(chunk->code) - offset - 2;

	if (jump > UINT16_MAX)
	{
		error("Too much code to jump over.");
	}

	chunk->code[offset]		= (jump >> 8) & 0xff;
	chunk->code[offset + 1] = jump & 0xff;
}
