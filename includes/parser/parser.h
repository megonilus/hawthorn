#ifndef haw_parser
#define haw_parser

#include "lexer/lexer.h"
#include "share/string.h"
#include "value/obj.h"

#include <stdint.h>

typedef struct
{
	Token		  previous;
	Token		  current;
	haw_function* functions;
	LexState*	  ls;
} Parser;

#undef this
#define this Parser p

haw_function* parse(cstr filename);
void		  parser_init(Parser* p, LexState* ls);
void		  parser_destroy(Parser* p);

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
