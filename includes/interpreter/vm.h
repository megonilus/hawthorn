#include "chunk/chunk.h"
#include "share/table.h"
#include "value/value.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
	Chunk*	chunk;
	TValue* stack;
	Table	strings;
	Table	globals;
	Obj*	objects;

	size_t pc; // Program Counter
} vm;

extern vm v;

void vm_init(Chunk* chunk);
void vm_execute();
void vm_destroy();
