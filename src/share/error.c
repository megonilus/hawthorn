#include <share/error.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static noret exited()
{
	fprintf(stderr, "%s: exited due errors in runtime\n", PROGRAM_NAME);
	exit(1);
}

noret throw_error_with_line(const char* module_name, const char* error_msg, int line)
{
	fflush(stdout);
	fprintf(stderr, "\n%s: %s at %d\n", module_name, error_msg, line);
	exited();
}

noret throw_error(const char* module_name, const char* error_msg)
{
	fflush(stdout);
	fprintf(stderr, "\n%s: %s\n", module_name, error_msg);
	exited();
}

noret throw_errorf(const char* module_name, const char* msg, ...)
{
	fflush(stdout);
	va_list args;
	va_start(args, msg);

	fprintf(stderr, "\n%s: ", module_name);
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");

	va_end(args);
	exited();
}

#undef noret
