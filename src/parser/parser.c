#include "chunk/opcodes.h"
#include "interpreter/vm.h"
#include "share/array.h"
#include "share/hawthorn.h"
#include "share/table.h"
#include "type/type.h"
#include "value/obj.h"
#include "value/value.h"

#include <stdint.h>
#include <string.h>

#undef this
#include <chunk/chunk.h>
#include <lexer/lexer.h>
#include <lexer/token.h>
#include <share/error.h>
#include <share/string.h>

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif
#define MODULE_NAME "parser"

#include <parser/parser.h>
#include <stdio.h>

this;

#define set_globalp(pa) p = pa

// utilities

#define expected(t) errorf("Expected %s", tok_2str(t))
#define expecteds(m) errorf("Expected %s", m)

#define seminf p.current.seminfo
#define tsem(a) (a)->seminfo
#define tstr(a) tsem(a).str_
#define pscopes p.scopes

#define pop() emit_byte(&p.chunk, OP_POP)
#define halt() emit_byte(&p.chunk, OP_HALT)

static inline void next()
{
	p.previous = p.current;
	p.current  = lex(p.ls);

	if (getflag(flags, DBG_LEXER) && p.current.type != TK_EOF)
	{
		dislex(p.ls, p.current.type);
	}
}

static inline void consumef(TokenType type, cstr msg)
{
	if (p.current.type == type)
	{
		next();
		return;
	}

	errorf("Expected %s, found %s", tok_2str(type),
		   tok_2str(p.current.type));
}

static inline void consume(TokenType type)
{
	if (p.current.type == type)
	{
		next();
		return;
	}

	errorf("Expected %s", tok_2str(type));
}

static inline int names_equal(Token* a, Token* b)
{
	return tstr(a) == tstr(b);
}

static inline haw_string* parse_name(Token* token)
{
	if (token->type != TK_NAME)
	{
		errorf("Expected %s", tok_2str(TK_NAME));
	}

	return token->seminfo.str_;
}

static inline int match(TokenType type)
{
	if (p.current.type == type)
	{
		next();
		return 1;
	}

	return 0;
}

static inline int emit_void()
{
	return write_constant(&p.chunk, v_void());
}

static int string_constant(haw_string* string)
{
	TValue idx;
	idx.type	   = HAW_TINT;
	TValue val	   = v_str(string);
	obj_type(&val) = OBJ_STRING;

	haw_string* interned = table_find_string(
		&v.strings, string->chars, string->length, string->hash, &idx);

	if (interned != NULL && idx.type != HAW_TNONE)
	{
		return raw_write_constant(&p.chunk, int_value(&idx));
	}

	int index = write_constant(&p.chunk, val);

	idx = v_int(index);

	table_set(&v.strings, string, idx);

	return index;
}

static int name_constant(haw_string* string)
{
	TValue idx;
	idx.type = HAW_TINT;

	haw_string* interned = table_find_string(
		&v.strings, string->chars, string->length, string->hash, &idx);

	if (interned != NULL && idx.type != HAW_TNONE)
	{
		return int_value(&idx);
	}

	TValue val	   = v_str(string);
	obj_type(&val) = OBJ_STRING;

	int index = add_constant(&p.chunk, val);

	table_set(&v.strings, string, v_int(index));

	return index;
}

static inline int compile_name(Token* token)
{
	haw_string* var_name = parse_name(token);
	return name_constant(var_name);
}

static inline void add_local(Token* name)
{
	if (p.scopes.local_count == UINT8_MAX)
	{
		error("too many local variables");
	}

	Local* local = &pscopes.locals[pscopes.local_count++];
	local->name	 = *name;
	local->depth = -1;
}

static void decl_var(Token* name)
{
	if (pscopes.scopes_deep == 0)
	{
		return;
	}

	for (int i = pscopes.local_count - 1; i >= 0; i--)
	{
		Local* local = &pscopes.locals[i];
		if (local->depth != -1 && local->depth < pscopes.scopes_deep)
		{
			break;
		}

		if (names_equal(name, &local->name))
		{
			error("");
		}
	}

	add_local(name);
}

static inline int parse_variable(Token* token)
{
	int arg = compile_name(token);

	decl_var(token);
	if (pscopes.scopes_deep > 0)
	{
		return -1;
	}

	return arg;
}

static inline void var(int arg, OpCode opcode)
{
	if (arg <= UINT8_MAX)
	{
		emit_bytes(&p.chunk, opcode, arg);
	}
	else
	{
		to_u24(arg);
		emit_bytes(&p.chunk, OP_WIDE, opcode, major, mid, minor);
	}
}

static inline void def_var(int arg)
{
	if (arg == -1)	 // local
	{
		pscopes.locals[pscopes.local_count - 1].depth =
			pscopes.scopes_deep;
		return;
	}

	var(arg, OP_SETGLOBAL);
	pop();
}

#define load_var(a) var(a, OP_LOADGLOBAL)

// Statements
#define dstmt(s) static void s()
dstmt(decl);
dstmt(stmt);

dstmt(vardeclstat);
dstmt(printstat);
dstmt(scopestat);
dstmt(ifstat);
dstmt(whilestat);
dstmt(forstat);

// Expressions
#define dexpr(s) static void s(int can_assign)
dexpr(expr);
dexpr(binary);
dexpr(unary);
dexpr(literal);
dexpr(varexpr);
dexpr(grouping);
dexpr(postfix);

static void expr_stmt();
static void name(int can_assign);

static void prec(Precedence prec);

#define emptyrule(t) [t] = {NULL, NULL, PREC_NONE}

#define onlypfunc(t, f) [t] = {f, NULL, PREC_NONE}
#define onlyifunc(t, f) [t] = {NULL, f, PREC_NONE}

#define rule(t, ...) [t] = {__VA_ARGS__}

static ParseRule rules[] = {
	// operators
	// arithmetic
	rule('+', NULL, binary, PREC_TERM),
	rule('-', unary, binary, PREC_TERM),
	rule(TK_CONCAT, NULL, binary, PREC_TERM),

	rule('*', NULL, binary, PREC_FACTOR),
	rule('/', NULL, binary, PREC_FACTOR),
	rule('^', NULL, binary, PREC_FACTOR),
	rule(TK_IDIV, NULL, binary, PREC_FACTOR),
	rule('%', NULL, binary, PREC_FACTOR),
	// boolean
	rule(TK_AND, NULL, binary, PREC_AND),
	rule(TK_OR, NULL, binary, PREC_AND),
	rule(TK_GE, NULL, binary, PREC_COMPARISON),
	rule(TK_LE, NULL, binary, PREC_COMPARISON),
	rule(TK_EQ, NULL, binary, PREC_EQ),
	rule(TK_NOTEQ, NULL, binary, PREC_EQ),
	rule('>', NULL, binary, PREC_COMPARISON),
	rule('<', NULL, binary, PREC_COMPARISON),
	// unary
	rule(TK_INC, unary, postfix, PREC_UNARY),
	rule(TK_DEC, unary, postfix, PREC_UNARY),
	rule('!', unary, NULL, PREC_UNARY),

	onlypfunc(TK_NAME, name),

	// symbols
	onlypfunc('(', grouping),
	emptyrule(')'),
	emptyrule('{'),
	emptyrule('}'),
	emptyrule(','),
	emptyrule('.'),
	emptyrule(':'),
	emptyrule('='),
	// 2 char symbols
	emptyrule(TK_FATARROW),
	// keywords
	emptyrule(TK_BIND),
	emptyrule(TK_SET),
	emptyrule(TK_WHILE),
	emptyrule(TK_FOR),
	emptyrule(TK_DO),
	emptyrule(TK_IF),
	emptyrule(TK_ELSE),
	emptyrule(TK_RETURN),
	// literals
	onlypfunc(TK_STRING, literal),
	onlypfunc(TK_NUMBER, literal),
	onlypfunc(TK_INT, literal),
	onlypfunc(TK_CHAR, literal),
	onlypfunc(TK_BOOL, literal),
	onlypfunc(TK_VOID, literal),
	// others
	emptyrule(TK_EOF),
};

#undef emptyrule
#undef onlyifunc
#undef onlypfunc
#undef fullrule
#undef rule

#define getrule(op) (&rules[op])

#undef dexpr
#undef dstmt

static void decl()
{
	switch (p.current.type)
	{
	case TK_SET:
		vardeclstat();
		break;
	default:
		stmt();
		break;
	}
}

static void stmt()
{
	switch (p.current.type)
	{
	case ';':	// empty statement
		next();
		break;

		// TODO
	case TK_RETURN:

	case TK_BIND:
		next();
		break;

	case TK_PRINT:
		printstat();
		break;
	case TK_WHILE:
		whilestat();
		break;
	case TK_FOR:
		forstat();
		break;
	case TK_IF:
		ifstat();
		break;
	case '{':
		scopestat();
		break;
	default:
		expr_stmt();
		break;
	}
}

static void vardeclstat()
{
	next();	  // skip `set`

	int arg = parse_variable(&p.current);
	next();

	int init = match('=');

	if (init)
	{
		expr(0);
	}
	else
	{
		emit_void();
	}

	def_var(arg);
}

static void printstat()
{
	next();
	expr(0);
	emit_byte(&p.chunk, OP_PRINT);
}

#define scope_begin() pscopes.scopes_deep++

static void scope_end()
{
	pscopes.scopes_deep--;

	while (pscopes.local_count > 0 &&
		   pscopes.locals[pscopes.local_count - 1].depth >
			   pscopes.scopes_deep)
	{
		pop();
		pscopes.local_count--;
	}
}

static inline void scopestat()
{
	next();
	scope_begin();

	while (p.current.type != TK_EOF && p.current.type != '}')
	{
		decl();
	}

	scope_end();

	consume('}');
}

static void emit_njump(int start)
{
	int offset = array_size(p.chunk.code) + 3 - start;

	if (offset > UINT16_MAX)
	{
		error("Loop body too large");
	}

	emit_bytes(&p.chunk, OP_NJMP, (offset >> 8) & 0xFF, offset & 0xFF);
}

static void whilestat()
{
	next();	  // while

	int start = array_size(p.chunk.code);

	int exit = -1;

	expr(1);

	int exit_jump = emit_jump(&p.chunk, OP_JMPF);

	stmt();

	emit_njump(start);

	patch_jump(&p.chunk, exit_jump);
}

static void forstat()
{
	scope_begin();
	next();	  // for

	if (!match(';'))
	{
		decl();
		consume(';');
	}

	uint32_t condition_start = array_size(p.chunk.code);

	int exit_jump = -1;
	if (!match(';'))
	{
		expr(1);
		consume(';');
		exit_jump = emit_jump(&p.chunk, OP_JMPF);
	}

	int body_jump = emit_jump(&p.chunk, OP_JMP);

	uint32_t increment_start = array_size(p.chunk.code);

	if (!match(';'))
	{
		expr_stmt();
		consume(';');
	}

	emit_njump(condition_start);

	patch_jump(&p.chunk, body_jump);
	stmt();
	emit_njump(increment_start);

	if (exit_jump != -1)
	{
		patch_jump(&p.chunk, exit_jump);
	}

	scope_end();
}

static void ifstat()
{
	next();	  // if

	expr(1);

	int then_jump = emit_jump(&p.chunk, OP_JMPF);

	stmt();	  // then body

	uint32_t else_jump = emit_jump(&p.chunk, OP_JMP);
	patch_jump(&p.chunk, then_jump);

	if (match(TK_ELSE))
	{
		stmt();
	}

	patch_jump(&p.chunk, else_jump);
}

static inline void expr_stmt()
{
	expr(1);

	if (getflag(flags, REPL))
	{
		emit_byte(&p.chunk, OP_PRINT);
	}
	else
	{
		pop();	 // TODO: implicit OP_RETURN
	}
}

// Exprs
static inline void expr(int can_assign)
{
	prec(PREC_ASSIGNMENT);
}

static void prec(Precedence prec)
{
	next();	  // skips 1 in 1 + 1
	ParseFn prefix = getrule(p.previous.type)->prefix;

	// so is not a literal(and not unary operator)
	if (prefix == NULL)
	{
		expecteds("expression");
		return;
	}

	int canAssign = prec <= PREC_ASSIGNMENT;
	prefix(canAssign);

	while (prec <= getrule(p.current.type)->precedence)
	{
		next();

		ParseFn infix = getrule(p.previous.type)->infix;
		infix(canAssign);
	}

	if (canAssign && match('='))
	{
		error("Invalid assignment target");
	}
}

static int resolve_local(Token* name)
{
	for (int i = pscopes.local_count - 1; i >= 0; i--)
	{
		Local* local = &pscopes.locals[i];
		if (names_equal(name, &local->name))
		{
			if (local->depth == -1)
			{
				error(
					"Cannot read local variable in it's own initializer");
			}

			return i;
		}
	}

	return -1;
}

static void name(int can_assign)
{
	int		is_local = resolve_local(&p.previous);
	int		arg;
	uint8_t get, set;

	if (is_local != -1)
	{
		get = OP_LOADLOCAL;
		set = OP_SETLOCAL;
		arg = is_local;
	}
	else
	{
		get = OP_LOADGLOBAL;
		set = OP_SETGLOBAL;
		arg = compile_name(&p.previous);
	}

	if (can_assign && match('='))
	{
		expr(1);

		var(arg, set);
	}
	else
	{
		var(arg, get);
	}
}

static void binary(int can_assign)
{
	lexer_char op	= p.previous.type;
	ParseRule* rule = getrule(op);

	prec((Precedence) rule->precedence + 1);

	switch (getbinopr(op))
	{
#define oper(OP, B)                                                       \
	case OP:                                                              \
		emit_byte(&p.chunk, B);                                           \
		break;

		// arithmetic
		oper(OPR_BADD, OP_ADD);
		oper(OPR_BSUB, OP_SUB);
		oper(OPR_BMUL, OP_MUL);
		oper(OPR_BDIV, OP_DIV);
		oper(OPR_BIDIV, OP_IDIV);
		oper(OPR_BPOW, OP_POW);
		oper(OPR_BMOD, OP_MOD);
		oper(OPR_BCONCAT, OP_CONCAT);
		// boolean
		oper(OPR_BAND, OP_AND);
		oper(OPR_BOR, OP_OR);
		oper(OPR_BGE, OP_GE);
		oper(OPR_BLE, OP_LE);
		oper(OPR_BGT, OP_GT);
		oper(OPR_BLT, OP_LT);
		oper(OPR_BEQ, OP_EQ);
	case OPR_BNEQ:
		emit_byte(&p.chunk, OP_EQ);
		emit_byte(&p.chunk, OP_NOT);
		break;

#undef oper
	default:
		unreachable();
	}
}

static void unary(int can_assign)
{
	lexer_char op = p.previous.type;

	prec(getrule(p.previous.type)->precedence + 1);

	if (op == '+')
	{
		return;
	}

	switch (getunopr(op))
	{
	case OPR_NEGATE:
		emit_byte(&p.chunk, OP_NEG);
		break;
	case OPR_INC:
		write_constant(&p.chunk, v_one());
		emit_byte(&p.chunk, OP_ADD);
		break;
	case OPR_DEC:
		write_constant(&p.chunk, v_one());
		emit_byte(&p.chunk, OP_SUB);
		break;
	case OPR_NOT:
		emit_byte(&p.chunk, OP_NOT);
		break;
	default:
		unreachable();
	}
}

static void postfix(int can_assign)
{
	lexer_char op = p.previous.type;

	write_constant(&p.chunk, v_one());

	if (op == TK_INC)
	{
		emit_byte(&p.chunk, OP_ADD);
	}
	else
	{
		emit_byte(&p.chunk, OP_SUB);
	}
}

static void grouping(int can_assign)
{
	expr(can_assign);
	consume(')');
}

static void literal(int can_assign)
{
	TValue result;
	switch (p.previous.type)
	{
	case TK_NUMBER:
		setnvalue(&result, p.previous.seminfo.num_);
		result.type = HAW_TNUMBER;
		break;
	case TK_BOOL:
	case TK_INT:
		result.type = HAW_TINT;
		setivalue(&result, p.previous.seminfo.int_);

		break;
	case TK_STRING:
		haw_string* string =
			take_string(p.previous.seminfo.str_->chars,
						p.previous.seminfo.str_->length, NULL);

		string_constant(string);
		return;
	case TK_CHAR:
		setivalue(
			&result,
			p.previous.seminfo.str_->chars[0]);	  // assign the 1st char
		result.type = HAW_TINT;
		break;
	default:
		expecteds("expression");
	}

	write_constant(&p.chunk, result);
}

void parser_init(Parser* p, LexState* ls)
{
	p->scopes.scopes_deep = 0;
	p->scopes.local_count = 0;
	p->ls				  = ls;

	chunk_init(&p->chunk);
}

inline void parser_clean(Parser* p)
{
	chunk_clear(&p->chunk);
	p->scopes.scopes_deep = 0;
}

void parse(cstr source)
{
	SemInfo seminfo;
	int		printlexems = getflag(flags, DBG_LEXER);
	lex_init(p.ls, source, &seminfo);

	if (printlexems)
	{
		printf("-- Lexemes\n");
	}

	next();

	for (; p.current.type != TK_EOF; decl())
	{
	}

	if (printlexems)
	{
		printf("\n\n");
	}

	if (getflag(flags, DBG_DISASM))
	{
		disassemble(&p.chunk);
	}

	halt();
}

#undef pop
#undef halt
#undef this
