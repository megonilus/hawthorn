#ifndef hawthorn_h
#define hawthorn_h

#define HAW_VERSION "0.0.1"
#define HAW_AUTHOR "megonilus"

// For later use
#include "hawconf.h"

// thread status
typedef enum
{
	THREAD_OK,
	THREAD_YIELD,
	THREAD_ERRRUN,
	THREAD_ERRSYNTAX,
	THREAD_ERRMEM,
	THREAD_ERRERR
} ThreadStatus;

#if !defined(HAWI_FUNC)
#if defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 302) &&                             \
	(defined(__ELF__) || defined(__MACH__))
#define HAWI_FUNC __attribute__((visibility("internal"))) extern
#else
#define HAWI_FUNC extern
#endif

#define INT_BITS 32
#endif
#endif // !hawthorn_h
