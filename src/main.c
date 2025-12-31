#include "parser/compiler.h"
#include "share/common.h"
#include "share/hawthorn.h"

#include <assert.h>
#include <interpreter/vm.h>
#include <lexer/lexer.h>
#include <parser/parser.h>
#include <setjmp.h>
#include <share/string.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

flags_t flags = 0;

def_parser();

static noret usage()
{
	printf("usage: %s <filename> %s", PROGRAM_NAME,
		   "[-d(show bytecode)] [-l(show lexemes)]\n");
	exit(0);
}

static void getflags(int argc, char* argv[])
{
	char c;

	while ((c = getopt(argc, argv, "d::l::s::")) != -1)
	{
		switch (c)
		{
		case 'd':
			setflag(DBG_DISASM);
			break;
		case 'l':
			setflag(DBG_LEXER);
			break;
		case 's':
			setflag(SKIP_RUN);
			break;
		case '?':
			printf("unrecognised option %c", optopt);
			break;
		}
	}
}

char* const readfile(cstr filename)
{
	FILE* file = fopen(filename, "rb");

	if (file == NULL)
	{
		fprintf(stderr, "Could not open file");
		exit(1);
	}

	fseek(file, 0L, SEEK_END);
	size_t file_size = ftell(file);

	char* value = (char*) malloc(sizeof(char) * (file_size + 1));

	rewind(file);

	size_t bytes_read = fread(value, sizeof(char), file_size, file);

	if (bytes_read < file_size)
	{
		fprintf(stderr, "Could not open file");
		exit(1);
	}

	fclose(file);
	value[file_size + 1] = '\0';

	return value;
}

static void run(cstr code)
{
	LexState ls;
	Compiler compiler;

	vm_init();
	compiler_init(&compiler, TYPE_SCRIPT);
	parser_init(&p, &ls);
	haw_function* fun = parse(code);

	if (!getflag(flags, SKIP_RUN))
	{
		interpret(fun);
	}

	lex_destroy(&ls);
	vm_destroy();
}

static inline void run_file(cstr filename)
{
	char* const source = readfile(filename);

	run(source);

	free(source);
}

extern jmp_buf repl_panic;

static volatile int repl_running = 1;

static void stop_repl(int a)
{
	repl_running = 0;
	longjmp(repl_panic, 1);
}

#define BUFFER_SIZE 128

static void repl_cycle()
{
	// set repl on the track
	setflag(REPL);
	signal(SIGINT, stop_repl);
	LexState ls;
	Compiler compiler;
	vm_init();
	compiler_init(&compiler, TYPE_SCRIPT);
	parser_init(&p, &ls);

	char buffer[BUFFER_SIZE];
	printf("%s %s by %s\n\n", HAW_INTERP_NAME, HAW_VERSION, HAW_AUTHOR);

	for (;;)
	{
		if (setjmp(repl_panic) != 0)
		{
			if (!repl_running)
			{
				break;
			}
			printf("\033[31;1;4m[!]\033[0m Error received.\n");
			continue;
		}

		printf("\033[36;1;4m[>]\033[0m ");

		if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
		{
			break;
		}

		if (buffer[0] == ':' && strcmp(buffer + 1, "clear\n") == 0)
		{
			printf("\033[2J\033[H");
			fflush(stdout);
			continue;
		}

		haw_function* fun = parse(buffer);

		if (!getflag(flags, SKIP_RUN))
		{
			interpret(fun);
		}
	}

	lex_destroy(&ls);
	compiler_end();
	vm_destroy();
}

#undef BUFFER_SIZE

int main(int argc, char* argv[])
{
	getflags(argc, argv);

	if (optind >= argc)
	{
		repl_cycle();
	}

	else
	{
		cstr filename = argv[optind];

		if (access(filename, F_OK) != 0)
		{
			fprintf(stderr, "cannot open file %s\n", filename);

			usage();
			exit(1);
		}

		run_file(filename);
	}

	return 0;
}
