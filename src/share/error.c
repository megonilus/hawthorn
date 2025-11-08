#include <share/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void exited()
{
	fprintf(stderr, "%s: exited due errors in runtime\n", PROGRAM_NAME);
}

void throw_error_with_line(char module_name[MAX_MODULE_NAME_LENGTH], const char* error_msg,
						   int line)
{
	fprintf(stderr, strcat(module_name, ": %s at %u\n"), error_msg, line);
	exited();
	exit(1);
}

void throw_error(char module_name[MAX_MODULE_NAME_LENGTH], const char* error_msg)
{
	fprintf(stderr, strcat(module_name, ": %s\n"), error_msg);
	exited();
	exit(1);
}
