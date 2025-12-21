#ifndef hawthorn_h
#define hawthorn_h

#include <stdint.h>

#define HAW_INTERP_NAME "hawthorn"
#define HAW_VERSION "0.0.1"
#define HAW_AUTHOR "megonilus"

// For later use
#include "hawconf.h"

// thread status
// bright future...
typedef enum
{
	THREAD_OK,
	THREAD_YIELD,
	THREAD_ERRRUN,
	THREAD_ERRSYNTAX,
	THREAD_ERRMEM,
	THREAD_ERRERR
} ThreadStatus;

typedef uint8_t flags_t;

typedef enum : flags_t
{
	DBG_DISASM,
	DBG_LEXER,
	SKIP_RUN,
	REPL,
} Flags;

#define setflag(f) flags = (1 << f) | flags
#define unsetflag(f) flags = flags & (~(1 << f))
#define getflag(fls, f) ((fls >> f) & 1)

#if !defined(HAWI_FUNC)
#if defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 302) &&    \
	(defined(__ELF__) || defined(__MACH__))
#define HAWI_FUNC __attribute__((visibility("internal"))) extern
#else
#define HAWI_FUNC extern
#endif

HAWI_FUNC flags_t flags;

#define INT_BITS 32
#endif
#endif	 // !hawthorn_h
