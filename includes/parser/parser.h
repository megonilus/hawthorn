#ifndef haw_parser
#define haw_parser

#include "share/string.h"
#include "type/type.h"
#include "value/value.h"

typedef enum : uint8_t
{
	E_VOID,
	E_TRUE,
	E_FALSE,
	E_K,

	// literals
	E_NUM,
	E_INT,
	E_STR,

	// variables
	E_LOCAL,
	E_GLOBAL,

	E_UPVAL, // upvalue var; info = index of upvalue in upvalues

	E_CONST, // compile-time const variable

	E_INDEXED,	  // indexed variable; a[i]
				  // idxv.t = table register
				  // idxv.idx = key's R index;
				  // idxv.ro = true if read-only global
				  // idxv.keystr = if key is a string, In 'k' of that string;
				  // -1 if key is not a string;
	E_INTINDEX,	  // indexed with constant integer
	E_UPINDEXED,  // indexed upvalue
				  // idxv.idx = key's K index;
				  // idxv.* as in E_INDEXED
	E_STRINDEXED, // indexed variable with literal string
				  // idxv.idx = key's K index;
				  // idxv.* as in E_INDEXED

	E_BRANCH, // if/else/else if
	E_CALL,
} exprkind;

void parse(str filename);

#define e_isvar(e) (E_LOCAL <= (e) && (e) <= E_STRINDEXED)
#define e_isindexed(e) (E_INDEXED <= (e) && (e) <= E_STRINDEXED)

typedef struct
{
	exprkind k;
	union
	{
		haw_int	   ival;   // E_INT
		haw_number nval;   // E_NUM
		String*	   strval; // E_STR
		int		   info;   // generic use

		struct // indexed variables
		{
			short	  idx;	  // index (R or "long" K)
			hawu_byte t;	  // table (register or upvalue)
			hawu_byte ro;	  // true if variable is read-only
			int		  keystr; // index in 'k' of string key, or -1 if not a string
		} idxv;

		struct // local variables
		{
			hawu_byte reg;	// register holding the variable
			short	  vidx; // index in actvar.arr
		} var;
	} u;

	int t;
	int f;
} exprdesc;

// kinds of variables
typedef enum
{
	REGLOCAL,
	CONSTLOCAL,
	COMPLOCAL,
	REGGLOBAL,
	CONSTGLOBAL
} varkind;

typedef union
{
	struct
	{
		TValueFields; // constant value (if it is a compile-time constant)
		varkind	  kind;
		hawu_byte reg;	// register holding the value
		String*	  name; // variable name
	} vard;

	TValue k; // constant value if any
} VarDesc;

// check if the variable live in the register
#define regvar(v) ((v)->vard.kind <= CONSTLOCAL)

// check if the variable is global
#define globvar(v) ((v)->vard.kind >= REGGLOBAL)

typedef struct
{
	VarDesc* vars; // list of all active local variables
	int		 scopes_deep;
} ParserData;

#endif // !haw_parser
