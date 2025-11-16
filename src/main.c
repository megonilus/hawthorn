#include "interpreter/repl.h"
#include "lexer/lexer.h"
#include "share/string.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
	String string = make_String("1234");
	String_append(&string, "5678");
	printf("%s\n", string.value);

	if (argc == 1)
	{
		repl_cycle();
	}
	else if (argc == 2)
	{
		SynLexState sl;
		str			source_name = make_str(argv[1]);
		synlex_init(&sl, &source_name);
	}

	String_destroy(&string);
	return 0;
}
