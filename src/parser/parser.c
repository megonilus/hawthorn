#include "lexer/lexer.h"
#include "lexer/token.h"

#include <parser/parser.h>
#include <stdio.h>

#define MAXVARS 200

// Statements
#define dstmt(s) static void s(SynLexState* sls)
dstmt(stmt);
dstmt(vardecl);

// Expressions
#define dexpr(s) static void s(SynLexState* sls, exprdesc* v)
dexpr(expr);
dexpr(intlit);
dexpr(numlit);
dexpr(strlit);
dexpr(varexpr);

#undef dexpr
#undef dstmt

void parse(str filename)
{
	SynLexState sls;
	synlex_init(&sls, &filename);

	SemInfo	   dummy;
	lexer_char current;

	while ((current = synlex_lex(&sls, &dummy)) != EOF)
	{
		synlex_dislex(current);
	}

	printf("\n"); // END debug seq
}
