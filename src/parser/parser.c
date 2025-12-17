#include "chunk/opcodes.h"
#include "type/type.h"
#include "value/obj.h"
#include "value/value.h"

#include <stdint.h>

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

#define seminf p.ls->seminfo

static void next()
{
	p.previous = p.current;
	p.current  = lex(p.ls);
#if SLS_DEBUGL
	if (p.current.type != TK_EOF)
	{
		dislex(p.ls, p.current.type);
	}
#endif
}

static void consumef(TokenType type, cstr msg)
{
	if (p.current.type == type)
	{
		next();
		return;
	}
	errorf("Expected %s, found %s", tok_2str(type), tok_2str(p.current.type));
}

static void consume(TokenType type)
{
	if (p.current.type == type)
	{
		next();
		return;
	}

	errorf("Expected %s", tok_2str(type));
}

static inline haw_string* parse_name(Token* token)
{
	if (token->type != TK_NAME)
	{
		errorf("Expected %s", tok_2str(TK_NAME));
	}

	return seminf->str_;
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

static inline int name_constant(haw_string* string)
{
	TValue val;
	val.type = HAW_TOBJECT;
	setovalue(&val, take_string(string->chars, string->length));
	obj_type(&val) = OBJ_STRING;

	return write_constant(&p.chunk, val);
}

static inline void def_var()
{
	emit_byte(&p.chunk, OP_SETGLOBAL);
}

// Statements
#define dstmt(s) static void s()
dstmt(decl);
dstmt(stmt);

dstmt(vardeclstat);
dstmt(printstat);

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
	rule('+', unary, binary, PREC_TERM),
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
	rule('!', NULL, unary, PREC_UNARY),

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
	case TK_NAME:
		next();
		name(1);
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
	case ';': // empty statement
		next();
		break;

		// TODO
	case TK_RETURN:
	case TK_BREAK:
	case TK_IF:
	case TK_DO:
	case TK_WHILE:
	case TK_FOR:
	case TK_BIND:
		next();
		break;

	case TK_PRINT:
		printstat();
		break;

	default:
		expr_stmt();
		break;
	}
}

static void vardeclstat()
{
	next(); // skip `set`

	haw_string* var_name = parse_name(&p.current);
	name_constant(var_name);
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

	def_var();
}

static void printstat()
{
	next();
	expr(0);
	emit_byte(&p.chunk, OP_PRINT);
}

static void expr_stmt()
{
	expr(0);
	emit_byte(&p.chunk, OP_POP); // TODO: implicit OP_RETURN
}

// Exprs
static void expr(int can_assign)
{
	prec(PREC_ASSIGNMENT);
}

static void prec(Precedence prec)
{
	next(); // skips 1 in 1 + 1
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

static void name(int can_assign)
{
	haw_string* name = parse_name(&p.previous);
	name_constant(name);

	if (can_assign && match('='))
	{
		expr(0);
		emit_byte(&p.chunk, OP_SETGLOBAL);
	}
	else
	{
		emit_byte(&p.chunk, OP_LOADGLOBAL);
	}
}

static void binary(int can_assign)
{
	lexer_char op	= p.previous.type;
	ParseRule* rule = getrule(op);

	prec((Precedence) rule->precedence + 1);

	switch (getbinopr(op))
	{
#define oper(OP, B)                                                                                \
	case OP:                                                                                       \
		emit_byte(&p.chunk, B);                                                                    \
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
		TValue val;
		val.type = HAW_TINT;
		setivalue(&val, 1);
		write_constant(&p.chunk, val);
		emit_byte(&p.chunk, OP_ADD);
		break;
	case OPR_DEC:
		TValue vala;
		vala.type = HAW_TINT;
		setivalue(&vala, 1);
		write_constant(&p.chunk, vala);
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

	TValue one;
	one.type = HAW_TINT;
	setivalue(&one, 1);
	write_constant(&p.chunk, one);

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
		setnvalue(&result, seminf->num_);
		result.type = HAW_TNUMBER;
		break;
	case TK_BOOL:
	case TK_INT:
		result.type = HAW_TINT;
		setivalue(&result, seminf->int_);

		break;
	case TK_STRING:
		haw_string* string = take_string(seminf->str_->chars, seminf->str_->length);
		setovalue(&result, string);

		result.type		  = HAW_TOBJECT;
		obj_type(&result) = OBJ_STRING;

		break;
	case TK_CHAR:
		setivalue(&result, seminf->str_->chars[0]); // assign the 1st char
		result.type = HAW_TINT;

		break;
	default:
		expecteds("expression");
	}

	write_constant(&p.chunk, result);
}

void parser_init(Parser* p, LexState* sls)
{
	p->ls		   = sls;
	p->scopes_deep = 0;

	chunk_init(&p->chunk);
	p->vars = array(VarDesc);
}

#define halt() emit_byte(&p.chunk, OP_HALT)

void parse(str* filename)
{
	SemInfo seminfo;
	lex_init(p.ls, filename, &seminfo);
	next();

	for (; p.current.type != TK_EOF; decl())
	{
	}

	printf("\n");
#if SLS_DEBUGL
	printf("\n"); // END debug seq
#endif

	disassemble(&p.chunk);
	halt();

	lex_destroy(p.ls);
}

void parser_destroy(Parser* p)
{
	lex_destroy(p->ls);
	array_free(p->vars);
}

#undef this
