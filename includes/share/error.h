#ifndef megonil_error
#define megonil_error

#include "common.h"
#include "util.h"

#define error(MSG) throw_error(MODULE_NAME, (MSG))

#define error_with_line(MSG) throw_error_with_line(MODULE_NAME, (MSG), __LINE__)

#define error_at(MSG, AT) throw_error_with_line(MODULE_NAME, (MSG), (AT))

#define errorf(...) throw_errorf(MODULE_NAME, __VA_ARGS__)

noret throw_error_with_line(const char* module_name, const char* error_msg, int line);
noret throw_error(const char* module_name, const char* error_msg);
noret throw_errorf(const char* module_name, const char* msg, ...);

#endif
