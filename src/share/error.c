#include <share/error.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static noret exited()
{
	fprintf(stderr, "%s: exited due errors in runtime\n", PROGRAM_NAME);
	exit(1);
}

noret throw_error_with_line(char module_name[MAX_MODULE_NAME_LENGTH], const char* error_msg,
							int line)
{
	fprintf(stderr, strcat(module_name, ": %s at %u\n"), error_msg, line);
	exited();
}

noret throw_error(char module_name[MAX_MODULE_NAME_LENGTH], const char* error_msg)
{
	fprintf(stderr, strcat(module_name, ": %s\n"), error_msg);
	exited();
}

noret throw_errorf(char module_name[MAX_MODULE_NAME_LENGTH], const char* msg, ...)
{
	va_list args;
	va_start(args, msg);

	fprintf(stderr, "%s: ", module_name);
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");

	va_end(args);
	exited();
}

#undef noret
