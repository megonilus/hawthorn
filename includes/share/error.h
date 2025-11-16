#ifndef megonil_error
#define megonil_error

#include "common.h"
#include "util.h"

#define MAX_MODULE_NAME_LENGTH 32

typedef struct
{
	char* name;
} Module;

#define error_at(MSG, AT) throw_error(MODULE_NAME, MSG, AT)
#define error_with_line(MSG) throw_error(MODULE_NAME, MSG, __LINE__)
#define error(MSG) throw_error(MODULE_NAME, MSG)
#define errorf(msg...) throw_errorf(MODULE_NAME, msg)

noret throw_error_with_line(char module_name[MAX_MODULE_NAME_LENGTH], const char* error_msg,
							int line);
noret throw_error(char module_name[MAX_MODULE_NAME_LENGTH], const char* error_msg);

noret throw_errorf(char module_name[MAX_MODULE_NAME_LENGTH], const char* msg, ...);

#endif
