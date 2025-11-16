#ifndef haw_lexer
#define haw_lexer

#include "token.h"

#include <share/hawthorn.h>
#include <share/util.h>

cstr get_contents_of_file(cstr file_name);

typedef int lexer_char;

/// State of the scanner plus state of the parser when shared by all functions
typedef struct
{
	// LEXER
	str		   file_contents;
	lexer_char current;
	lexer_char look_ahead;
	lexer_size line_number;
	String	   buffer;

	// PARSER
	Token current_token;	// current token
	Token look_ahead_token; // next token

	// BOTH
	str*   source_name;
	size_t pos;
} SynLexState;

#define this SynLexState* sls

HAWI_FUNC void	synlex_init(this, str* source_name);
HAWI_FUNC void	synlex_setinput(this, str* source_name);
HAWI_FUNC void	synlex_next(this);
HAWI_FUNC noret synlex_syntaxerror(this, cstr message);
HAWI_FUNC noret synlex_lexerror(this, cstr message);
HAWI_FUNC cstr	synlex_tokentostr(this, TokenType token);

lexer_char synlex_lex(this, SemInfo* seminfo);
void	   synlex_destroy(this);

#endif // !haw_lexer
