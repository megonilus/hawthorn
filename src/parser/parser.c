#include "chunk/opcodes.h"
#include "type/type.h"
#include "value/value.h"

#undef this
#include <chunk/chunk.h>
#include <lexer/lexer.h>
#include <lexer/token.h>
#include <share/error.h>
#include <share/string.h>

#undef MODULE_NAME
#define MODULE_NAME "parser"

#include <parser/parser.h>
#include <stdio.h>

this;

#define set_globalp(pa) p = pa

static void next()
{
	p.previous = p.current;
	p.current  = lex(p.ls);
}

static void consumef(TokenType type, cstr msg)
{
	if (p.current.type == type)
	{
		next();
		return;
	}
	errorf("Expected %s, found %s", tok_2str("", type), tok_2str("", p.current.type));
}

static void consume(TokenType type, cstr msg)
{
	if (p.current.type == type)
	{
		next();
		return;
	}
	errorf("Expected %s %s", tok_2str("", type), msg);
}

// Statements
#define dstmt(s) static void s()
dstmt(stmt);
dstmt(vardecl);
dstmt(printstat);

// Expressions
#define dexpr(s) static void s()
dexpr(expr);
dexpr(binary);
dexpr(unary);
dexpr(literal);
dexpr(varexpr);
dexpr(grouping);
dexpr(postfix);

static void prec(Precedence prec);

#define emptyrule(t) [t] = {NULL, NULL, PREC_NONE}
#define onlyifunc(t, f) [t] = {NULL, f, PREC_NONE}
#define onlypfunc(t, f) [t] = {f, NULL, PREC_NONE}
#define fullrule(t, fi, fp, p) [t] = {fi, fp, p}
#define rule(t, ...) [t] = {__VA_ARGS__}

static ParseRule rules[] = {
	// operators
	// arithmetic
	fullrule('+', unary, binary, PREC_TERM),
	fullrule('-', unary, binary, PREC_TERM),
	rule('*', NULL, binary, PREC_FACTOR),
	rule('/', NULL, binary, PREC_FACTOR),
	rule('^', NULL, binary, PREC_FACTOR),
	rule(TK_IDIV, NULL, binary, PREC_FACTOR),
	// boolean
	rule(TK_AND, NULL, binary, PREC_AND),
	rule(TK_OR, NULL, binary, PREC_AND),
	rule(TK_GE, NULL, binary, PREC_COMPARISON),
	rule(TK_LE, NULL, binary, PREC_COMPARISON),
	rule('>', NULL, binary, PREC_COMPARISON),
	rule('<', NULL, binary, PREC_COMPARISON),
	// unary
	rule(TK_INC, unary, postfix, PREC_UNARY),
	rule(TK_DEC, unary, postfix, PREC_UNARY),
	rule('!', NULL, unary, PREC_UNARY),

	// symbols
	onlypfunc('(', grouping),
	emptyrule(')'),
	emptyrule('{'),
	emptyrule('}'),
	emptyrule(','),
	emptyrule('.'),
	emptyrule('='),
	emptyrule(':'),
	// 2 char symbols
	emptyrule(TK_EQ),
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
	emptyrule(TK_NAME),
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

#define expecteds(t) errorf("Expected %s", tok_2str(t))
#define expected(m) errorf("Expected %s", m)

#define seminf p.ls->seminfo

static void stmt()
{
	switch (p.current.type)
	{
		// TODO
	case ';': // empty statement
		next();
		break;

	case TK_RETURN:
	case TK_BREAK:
	case TK_IF:
	case TK_DO:
	case TK_WHILE:
	case TK_FOR:
	case TK_BIND:
	case TK_SET:
		next();
		break;

	case TK_PRINT:
		printstat();
		break;
	default:
		expected("statement");
	}
}

static void printstat()
{
	next();
	expr();
	emit_byte(&p.chunk, OP_PRINT);
}

static void expr()
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
		expected("expression");
		return;
	}

	prefix();

	while (prec <= getrule(p.current.type)->precedence)
	{
		next();

		ParseFn infix = getrule(p.previous.type)->infix;
		infix();
	}
}

static void binary()
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
		// boolean
		oper(OPR_BAND, OP_AND);
		oper(OPR_BOR, OP_OR);
		oper(OPR_BGE, OP_GE);
		oper(OPR_BLE, OP_LE);
		oper(OPR_BGT, OP_GT);
		oper(OPR_BLT, OP_LT);

#undef oper
	default:
		unreachable();
	}
}

static void unary()
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

static void postfix()
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

static void grouping()
{
	expr();
	consume(')', "after expression");
}

static void literal()
{
	TValue result;
	Value  val;

	switch (p.previous.type)
	{
	case TK_NUMBER:
		val.number_ = seminf->num_;
		result.type = HAW_TNUMBER;
		break;
	case TK_BOOL:
	case TK_INT:
		val.int_	= seminf->int_;
		result.type = HAW_TINT;
		break;
	case TK_STRING:
		val.str_	= seminf->str_;
		result.type = HAW_TSTRING;
		break;
	case TK_CHAR:
		val.int_	= *seminf->str_->value;
		result.type = HAW_TINT;
		break;
	default:
		expected("expression");
	}

	result.value_ = val;
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

	for (dislex(p.ls, p.current.type); p.current.type != TK_EOF; stmt())
	{
	}

	printf("\n"); // END debug seq
	halt();
#ifdef DISASSEMBLE
	disassemble(&p.chunk);
#endif

	lex_destroy(p.ls);
}

void parser_destroy(Parser* p)
{
	chunk_destroy(&p->chunk);
	lex_destroy(p->ls);
	array_free(p->vars);
}

#undef this
