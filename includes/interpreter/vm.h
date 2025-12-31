#include "chunk/chunk.h"
#include "share/table.h"
#include "value/obj.h"
#include "value/value.h"

#include <stddef.h>
#include <stdint.h>

typedef struct
{
	haw_function* function;
	uint8_t*	  ip;
	TValue*		  slots;
} CallFrame;

#define FRAMES_MAX 256
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct
{
	CallFrame frames[FRAMES_MAX];
	int		  frame_count;

	Chunk*	chunk;
	TValue* stack;
	TValue* stack_top;
	Table	strings;
	Table	globals;
	Obj*	objects;
} vm;

extern vm v;

void vm_init();
void vm_execute();
void interpret(haw_function* fun);
void vm_destroy();
