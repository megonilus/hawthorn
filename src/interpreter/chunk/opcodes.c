#include "chunk/opcodes.h"

#include "lexer/token.h"
#include "share/error.h"

#define opcase(ch, opr)                                                                            \
	case ch:                                                                                       \
		return opr;

const char* op_name(OpCodes op)
{
	return opnames[op];
}
BinOpr getbinopr(int op)
{
	switch (op)
	{
		opcase('+', OPR_BADD);
		opcase('-', OPR_BSUB);
		opcase('*', OPR_BMUL);
		opcase('/', OPR_BDIV);
		opcase(TK_IDIV, OPR_BIDIV);
		opcase(TK_AND, OPR_BAND);
		opcase(TK_OR, OPR_BOR);
		opcase(TK_GE, OPR_BGE);
		opcase(TK_LE, OPR_BLE);
		opcase('>', OPR_BGT);
		opcase('<', OPR_BLT);
		opcase('^', OPR_BPOW);
		opcase('%', OPR_BMOD);
	default:
		error("Expected BinaryOperator");
	}
}

UnOpr getunopr(int op)
{
	switch (op)
	{
		opcase('-', OPR_NEGATE);
		opcase('!', OPR_NOT);

		opcase(TK_INC, OPR_INC);
		opcase(TK_DEC, OPR_DEC);
	default:
		error("Expected UnaryOperator");
	}
}
#undef opcase
