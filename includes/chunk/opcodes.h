#ifndef haw_bytecode
#define haw_bytecode

#include <share/common.h>
#include <share/error.h>

#define MODULE_NAME "opcodes"

typedef enum : uint8_t
{
	OP_CONSTANT,
	OP_CONSTANT_LONG,

	OP_ADD,	 // +
	OP_MUL,	 // *
	OP_SUB,	 // -
	OP_DIV,	 // /
	OP_MOD,	 // %
	OP_IDIV, // //
	OP_NEG,	 // negate
	OP_NOT,	 // !
	OP_POW,	 // ^
	OP_AND,	 // and
	OP_OR,	 // or
	OP_GE,	 // >=
	OP_LE,	 // <=
	OP_GT,	 // >
	OP_LT,	 // <

	OP_JMP,	 // jump <K>
	OP_JMPF, // jump <K> if pop() false

	OP_SETLOCAL,
	OP_SETGLOBAL,
	OP_LOADLOCAL,
	OP_LOADGLOBAL,

	OP_CALL, // call function

	OP_PRINT,
	OP_RETURN,

	OP_HALT,

	OP_LAST,
} OpCodes;

#define NUM_OPCODES ((int) OP_LAST)

static const char* opnames[NUM_OPCODES] = {
	[OP_CONSTANT]	   = "CONSTANT",
	[OP_CONSTANT_LONG] = "CONSTANT_LONG",
	[OP_ADD]		   = "ADD",
	[OP_MUL]		   = "MUL",
	[OP_SUB]		   = "SUB",
	[OP_DIV]		   = "DIV",
	[OP_MOD]		   = "MOD",
	[OP_IDIV]		   = "IDIV",
	[OP_NEG]		   = "NEG",
	[OP_SETLOCAL]	   = "SETLOCAL",
	[OP_SETGLOBAL]	   = "SETGLOBAL",
	[OP_LOADLOCAL]	   = "LOADLOCAL",
	[OP_LOADGLOBAL]	   = "LOADGLOBAL",
	[OP_CALL]		   = "CALL",
	[OP_RETURN]		   = "RETURN",
	[OP_HALT]		   = "HALT",
	[OP_PRINT]		   = "PRINT",
	[OP_NOT]		   = "NOT",
	[OP_POW]		   = "POW",
	[OP_GE]			   = "GE",
	[OP_GT]			   = "GT",
	[OP_LE]			   = "LE",
	[OP_LT]			   = "LT",
};

typedef enum
{
	// arithmetic
	OPR_BADD,  // a + b
	OPR_BSUB,  // a - b
	OPR_BMUL,  // a * b
	OPR_BDIV,  // a / b
	OPR_BMOD,  // a % b
	OPR_BIDIV, // a // b
	OPR_BPOW,  // a ^ b

	// logical
	OPR_BGE,  // a >= b
	OPR_BLE,  // a <= b
	OPR_BGT,  // a > b
	OPR_BLT,  // a < b
	OPR_BAND, // a and b
	OPR_BOR,  // a or b
} BinOpr;

typedef enum
{
	OPR_NEGATE, // -
	OPR_NOT,	// !
	OPR_INC,	// ++
	OPR_DEC,	// --
} UnOpr;

BinOpr		getbinopr(int op);
UnOpr		getunopr(int op);
const char* op_name(OpCodes op);

#endif
