#ifndef haw_compiler
#define haw_compiler

#include "chunk/chunk.h"
#include "lexer/token.h"
#include "value/obj.h"

typedef struct
{
	Token name;
	int	  depth;
} Local;

typedef enum
{
	TYPE_FUNCTION,
	TYPE_SCRIPT,
} FunctionType;

typedef struct Compiler
{
	struct Compiler* enclosing;
	haw_function*	 function;
	FunctionType	 type;

	Local locals[UINT8_MAX + 1];
	int	  local_count;
	int	  scopes_deep;
} Compiler;

extern Compiler* current;

Chunk* current_chunk();

void compiler_init(Compiler* c, FunctionType t);

#define cfun current->function
haw_function* compiler_end();

#endif
