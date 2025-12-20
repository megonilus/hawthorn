#include "share/common.h"
#include "share/hawthorn.h"

#include <assert.h>
#include <interpreter/repl.h>
#include <interpreter/vm.h>
#include <lexer/lexer.h>
#include <parser/parser.h>
#include <share/string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <unistd.h>

def_parser();

static noret usage()
{
	printf("usage: %s <filename> %s", PROGRAM_NAME,
		   "[-d(show bytecode)] [-l(show lexemes)]\n");
	exit(0);
}

static flags_t getflags(int argc, char* argv[])
{
	flags_t flags = 0;
	char	c;

	while ((c = getopt(argc, argv, "d::l::")) != -1)
	{
		switch (c)
		{
		case 'd':
			setflag(DBG_DISASM);
			break;
		case 'l':
			setflag(DBG_LEXER);
			break;
		case '?':
			printf("unrecognised option %c", optopt);
			break;
		}
	}

	return flags;
}

void run_file(cstr filename, flags_t flags)
{
	LexState ls;

	vm_init(&p.chunk);
	parser_init(&p, &ls, flags);
	parse(filename);

	vm_execute();
	parser_destroy(&p);
	vm_destroy();
}

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		repl_cycle();
	}

	else if (argc >= 2)
	{
		cstr filename = argv[1];

		if (access(filename, F_OK) != 0)
		{
			fprintf(stderr, "can not open file %s\n", filename);
			usage();
			exit(1);
		}

		run_file(filename, getflags(argc, argv));
	}

	return 0;
}
