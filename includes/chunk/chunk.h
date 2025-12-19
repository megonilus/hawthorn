#ifndef hawthorn_chunk
#define hawthorn_chunk

#include <share/array.h>
#include <share/common.h>
#include <stdint.h>
#include <type/type.h>
#include <value/value.h>

typedef TValue Constant;

typedef struct
{
	hawu_byte* code;
	Constant*  constants;
} Chunk;

// util
// will be undefined
#define this Chunk* chunk

#define MAX_CONSTANT_LEN UINT8_MAX
#define BYTE 0xFF
#define BYTE_DEC 8

#define pass_chunk(c) (&c)

void	 chunk_init(this);
uint32_t raw_write_constant(Chunk* chunk, uint32_t index);
uint32_t add_constant(Chunk* chunk, Constant data);
uint32_t write_constant(this, Constant data);
uint32_t write_idxarg(this, uint32_t index);

void emit_byte(this, hawu_byte byte);

#define emit_bytes(c, ...) emit_some_bytes(c, __VA_ARGS__, -1)
void emit_some_bytes(this, ...);

void chunk_destroy(this);

void disassemble(this);

#define emit(v) (emit_byte(chunk, v))

#define from_u24(ma, md, mi) ((ma) | (uint32_t) (md) << 8 | (uint32_t) (mi) << 16)
#define to_u24(v)                                                                                  \
	uint8_t major = v & 0xFF;                                                                      \
	uint8_t mid	  = (v >> 8) & 0xFF;                                                               \
	uint8_t minor = (v >> 16) & 0xFF;

#endif // !hawthorn_chunk
