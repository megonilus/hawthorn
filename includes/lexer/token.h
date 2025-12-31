#ifndef haw_token
#define haw_token

#include "share/string.h"
#include "value/obj.h"

#include <limits.h>
#include <share/common.h>
#include <type/type.h>

// Single-char tokens (terminal symbols) are represented by their own
// numeric code. Other tokens start at the following value.
#define FIRST_RESERVED (UCHAR_MAX + 1)

typedef enum
{
	TK_RETURN = FIRST_RESERVED,
	TK_BREAK,
	TK_CONTINUE,
	TK_FUN,

	// blocks
	TK_ELSE,
	TK_IF,

	// variables
	TK_BIND,
	TK_SET,

	// cycles
	TK_DO,
	TK_WHILE,
	TK_FOR,

	// operators
	TK_AND,
	TK_OR,
	TK_NOTEQ,	 // !=
	TK_EQ,		 // ==
	TK_LE,		 // <=
	TK_GE,		 // >=
	TK_IDIV,	 // //
	TK_CONCAT,	 // <>

	// literals
	TK_BOOL,
	TK_INT,
	TK_NUMBER,
	TK_STRING,
	TK_CHAR,
	TK_VOID,

	TK_NAME,
	TK_EOF,

	TK_FATARROW,   // =>
	TK_PRINT,

	TK_INC,	  // ++
	TK_DEC,	  // --
} TokenType;

typedef union
{
	haw_number	num_;
	haw_int		int_;
	haw_string* str_;
} SemInfo;

typedef struct
{
	int		type;
	SemInfo seminfo;
} Token;

typedef int32_t lexer_size;

typedef struct
{
	lexer_size line;
	lexer_size column;
} position;

#define tok_pos(t) t - FIRST_RESERVED

static cstr const haw_tokens[] = {[tok_pos(TK_RETURN)] = "return",
								  [tok_pos(TK_BREAK)]  = "break",

								  [tok_pos(TK_ELSE)] = "else",
								  [tok_pos(TK_IF)]	 = "if",

								  [tok_pos(TK_BIND)] = "bind",

								  [tok_pos(TK_VOID)] = "void",

								  [tok_pos(TK_DO)]	  = "do",
								  [tok_pos(TK_WHILE)] = "while",
								  [tok_pos(TK_FOR)]	  = "for",

								  [tok_pos(TK_AND)]	  = "and",
								  [tok_pos(TK_OR)]	  = "or",
								  [tok_pos(TK_NOTEQ)] = "!=",
								  [tok_pos(TK_EQ)]	  = "==",
								  [tok_pos(TK_LE)]	  = "<=",
								  [tok_pos(TK_IDIV)]  = "//",

								  [tok_pos(TK_BOOL)]   = "<bool>",
								  [tok_pos(TK_INT)]	   = "<integer>",
								  [tok_pos(TK_NUMBER)] = "<number>",
								  [tok_pos(TK_STRING)] = "<string>",
								  [tok_pos(TK_NAME)]   = "<name>",
								  [tok_pos(TK_CHAR)]   = "<char>",

								  [tok_pos(TK_GE)]		 = ">=",
								  [tok_pos(TK_EOF)]		 = "\\0",
								  [tok_pos(TK_SET)]		 = "set",
								  [tok_pos(TK_FATARROW)] = "=>",
								  [tok_pos(TK_PRINT)]	 = "print",
								  [tok_pos(TK_INC)]		 = "++",
								  [tok_pos(TK_DEC)]		 = "--",
								  [tok_pos(TK_CONCAT)]	 = "<>",
								  [tok_pos(TK_FUN)]		 = "fun",
								  [tok_pos(TK_CONTINUE)] = "continue"};

#define KW_GROUP(letter, body)                                            \
	case letter:                                                          \
	{                                                                     \
		body;                                                             \
		break;                                                            \
	}
#define KW(keyword, token)                                                \
	if (strcmp(s, keyword) == 0)                                          \
		return token;

#endif	 // !haw_token
