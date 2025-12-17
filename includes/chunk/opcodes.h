#ifndef haw_bytecode
#define haw_bytecode

#include <share/common.h>
#include <share/error.h>

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif
#define MODULE_NAME "opcodes"

typedef enum
{
	OP_CONSTANT,
	OP_CONSTANT_LONG,

	OP_ADD,	   // +
	OP_MUL,	   // *
	OP_SUB,	   // -
	OP_DIV,	   // /
	OP_MOD,	   // %
	OP_IDIV,   // //
	OP_NEG,	   // negate
	OP_NOT,	   // !
	OP_POW,	   // ^
	OP_AND,	   // and
	OP_OR,	   // or
	OP_GE,	   // >=
	OP_LE,	   // <=
	OP_GT,	   // >
	OP_LT,	   // <
	OP_EQ,	   // ==
	OP_CONCAT, // <>

	OP_JMP,	 // jump <K>
	OP_JMPF, // jump <K> if pop() false

	OP_SETLOCAL,
	OP_SETGLOBAL,
	OP_LOADLOCAL,
	OP_LOADGLOBAL,

	OP_CALL, // call function

	OP_PRINT,
	OP_RETURN,

	OP_POP,

	OP_HALT,
} OpCode;

static const char* opnames[] = {
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
	[OP_EQ]			   = "EQ",
	[OP_AND]		   = "AND",
	[OP_OR]			   = "OR",
	[OP_CONCAT]		   = "CONCAT",
	[OP_POP]		   = "POP",
};

typedef enum
{
	// arithmetic
	OPR_BADD,	 // a + b
	OPR_BSUB,	 // a - b
	OPR_BMUL,	 // a * b
	OPR_BDIV,	 // a / b
	OPR_BMOD,	 // a % b
	OPR_BIDIV,	 // a // b
	OPR_BPOW,	 // a ^ b
	OPR_BCONCAT, // <>

	// logical
	OPR_BGE,  // a >= b
	OPR_BLE,  // a <= b
	OPR_BGT,  // a > b
	OPR_BLT,  // a < b
	OPR_BAND, // a and b
	OPR_BOR,  // a or b
	OPR_BEQ,  // ==
	OPR_BNEQ, // !=

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
const char* op_name(OpCode op);

#endif
