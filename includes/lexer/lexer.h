#ifndef haw_lexer
#define haw_lexer

#include "share/string.h"
#include "token.h"

#include <share/hawthorn.h>
#include <share/util.h>

typedef int lexer_char;

/// State of the scanner plus state of the parser when shared by all functions
typedef struct
{
	// LEXER
	Buffer	   file_contents;
	lexer_char current;
	lexer_size line_number;
	String	   buffer;
	str*	   source_name;
	SemInfo*   seminfo;
} LexState;

// specify to print or not token which is currently being processed
#define SLS_DEBUGL 1

#undef this
#define this LexState* ls

void lex_init(this, str* source_name, SemInfo* seminfo);
void lex_destroy(this);

Token lex(this);
void  lex_setintput(this, str* source_name);

cstr_mut tok_2str(lexer_char token);

void dislex(this, lexer_char token);

#endif // !haw_lexer
#undef this
