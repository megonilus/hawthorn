#ifndef haw_parser
#define haw_parser

#include "chunk/chunk.h"
#include "lexer/lexer.h"
#include "share/string.h"

#include <stdint.h>

typedef struct
{
	Token name;
	int	  depth;
} Local;

typedef struct
{
	Local locals[UINT8_MAX + 1];
	int	  local_count;
	int	  scopes_deep;
} Scopes;

typedef struct
{
	Chunk	  chunk;
	Scopes	  scopes;
	Token	  previous;
	Token	  current;
	LexState* ls;
} Parser;

#undef this
#define this Parser p

void parse(cstr filename);
void parser_init(Parser* p, LexState* ls);
void parser_clean(Parser* p);

typedef enum : uint8_t
{
	PREC_NONE,
	PREC_ASSIGNMENT,   // =
	PREC_OR,		   // or
	PREC_AND,		   // and
	PREC_EQ,		   // == !=
	PREC_COMPARISON,   // < > <= >=
	PREC_TERM,		   // + -
	PREC_FACTOR,	   // * / //
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY,
} Precedence;

typedef void (*ParseFn)(int can_assign);

typedef struct
{
	ParseFn	   prefix;
	ParseFn	   infix;
	Precedence precedence;
} ParseRule;

#define def_parser() extern this

#endif	 // !haw_parser
