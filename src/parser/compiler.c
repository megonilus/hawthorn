#include "parser/compiler.h"

#include "parser/parser.h"

def_parser();

#include "value/obj.h"

inline Chunk* current_chunk()
{
	return &current->function->chunk;
}

inline void compiler_init(Compiler* c, FunctionType t)
{
	c->enclosing = current;
#define cfun2 c->function
	cfun2		   = NULL;
	c->scopes_deep = 0;
	c->local_count = 0;
	c->type		   = t;
	cfun2		   = new_function();

	// for later use
	Local* local			 = &c->locals[c->local_count];
	local->depth			 = 0;
	local->name.seminfo.str_ = copy_string("", 0, NULL);
	c->local_count			 = 1;

	current = c;
	if (t != TYPE_SCRIPT)
	{
		current->function->name =
			copy_string(p.previous.seminfo.str_->chars,
						p.previous.seminfo.str_->length, NULL);
	}
#undef cfun2
}

#define cfun current->function
inline haw_function* compiler_end()
{
	haw_function* fun = cfun;

	current = current->enclosing;
	return fun;
}
