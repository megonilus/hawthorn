#ifndef megonil_error
#define megonil_error

#include "common.h"

#define MAX_MODULE_NAME_LENGTH 32

typedef struct
{
	char* name;
} Module;

#define error_at(MSG, AT) throw_error(module.name, MSG, AT)
#define error_with_line(MSG) throw_error(module.name, MSG, __LINE__)
#define error(MSG) throw_error(module.name, MSG)

void throw_error_with_line(char module_name[MAX_MODULE_NAME_LENGTH], const char* error_msg,
						   int line);
void throw_error(char module_name[MAX_MODULE_NAME_LENGTH], const char* error_msg);

#endif
