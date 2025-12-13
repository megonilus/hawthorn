#include <interpreter/repl.h>
#include <lexer/lexer.h>
#include <parser/parser.h>
#include <share/string.h>

def_parser();

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		repl_cycle();
	}

	else if (argc == 2)
	{
		LexState sls;
		str		 source_name = make_str(argv[1]);

		parser_init(&p, &sls);
		parse(&source_name);

		parser_destroy(&p);
	}

	return 0;
}
