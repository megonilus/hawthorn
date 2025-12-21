#ifndef haw_lexer
#define haw_lexer

#include "share/string.h"
#include "token.h"

#include <share/hawthorn.h>
#include <share/util.h>

typedef int lexer_char;

typedef struct
{
	lexer_char current;
	lexer_size line_number;

	const char* source;
	size_t		pos;

	String buffer;

	cstr	 source_name;
	SemInfo* seminfo;
} LexState;

#undef this
#define this LexState* ls

void lex_init(this, cstr source_name, SemInfo* seminfo);
void lex_destroy(this);

Token lex(this);
void  lex_setintput(this, str* source_name);

cstr_mut tok_2str(lexer_char token);

void dislex(this, lexer_char token);

#endif	 // !haw_lexer
#undef this
