#include "lexer/token.h"
#include "type/type.h"
#include "value/obj.h"
#include "value/value.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <lexer/lexer.h>
#include <limits.h>
#include <share/array.h>
#include <share/error.h>
#include <share/hawthorn.h>
#include <share/string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif
#define MODULE_NAME "lexer"

#undef this
#define this LexState* ls

#define save(ls, c) String_appendc(&ls->buffer, c);
// #define save(ls, c) String_appendc(&sls->buffer, c)
#define save_and_next(ls)                                                                          \
	save(ls, ls->current);                                                                         \
	advance(ls);

static inline void advance(LexState* ls)
{
	if (ls->file_contents.n < buffer_len(ls->file_contents))
	{
		ls->current = buffer_readnext(ls->file_contents);
	}
	else
	{
		ls->current = EOF;
	}
}

static noret lex_error(this, cstr msg, TokenType token);

static lexer_char check_next1(this, lexer_char c)
{
	if (ls->current == c)
	{
		advance(ls);
		return 1;
	}

	return 0;
}

static lexer_char is_reserved(const String* str, SemInfo* seminfo)
{
	const char* s  = str->value;
	size_t		ln = (size_t) str->length;
	if (ln == 0) return (TokenType) 0;
#define KW_CHECK_PART(start, rest_lit, tok)                                                        \
	do                                                                                             \
	{                                                                                              \
		const size_t __kw_len = (sizeof(rest_lit) - 1u);                                           \
		if (ln == (size_t) (start) + __kw_len && memcmp(s + (start), (rest_lit), __kw_len) == 0)   \
			return (tok);                                                                          \
	} while (0)
#define KW_CHECK_FULL(rest_lit, tok) KW_CHECK_PART(0, rest_lit, tok)
	switch ((unsigned char) s[0])
	{
	case 'a':
		KW_CHECK_FULL("and", TK_AND);
		break;
	case 'b':
		if (ln > 1)
		{
			switch (s[1])
			{
			case 'r':
				KW_CHECK_PART(2, "eak", TK_BREAK);
				break;
			case 'i':
				KW_CHECK_PART(2, "nd", TK_BIND);
				break;
			}
		}
		break;
	case 'd':
		KW_CHECK_FULL("do", TK_DO);
		break;
	case 'e':
		KW_CHECK_FULL("else", TK_ELSE);
		break;
	case 'f':
		if (ln > 1)
		{
			switch (s[1])
			{
			case 'o':
				KW_CHECK_PART(1, "or", TK_FOR);
				break;
			case 'a':
				KW_CHECK_PART(2, "lse", TK_BOOL);
				seminfo->int_ = 0;
				break;
			}
		}
		break;
	case 'i':
		if (ln > 1)
		{
			switch (s[1])
			{
			case 'f':
				KW_CHECK_FULL("if", TK_IF);
				break;
			case 'n':
				KW_CHECK_PART(1, "nt", TK_INT);
				break;
			}
		}
		break;
	case 'o':
		KW_CHECK_FULL("or", TK_OR);
		break;
	case 'r':
		KW_CHECK_FULL("return", TK_RETURN);
		break;
	case 'v':
		KW_CHECK_FULL("void", TK_VOID);
		break;
	case 'w':
		KW_CHECK_FULL("while", TK_WHILE);
		break;
	case 't':
		if (ln > 1)
		{
			switch (s[1])
			{
			case 'r':
				KW_CHECK_PART(2, "ue", TK_BOOL);
				seminfo->int_ = 1;
				break;
			}
		}
		break;
	case 's':
		KW_CHECK_FULL("set", TK_SET);
		break;
	case 'p':
		KW_CHECK_FULL("print", TK_PRINT);
		break;
	default:
		break;
	}
#undef KW_CHECK_PART
#undef KW_CHECK_FULL
	return 0;
}
static lexer_char check_next2(this, const char* set)
{
	assert(set[2] == '0');
	if (ls->current == set[0] || ls->current == set[1])
	{
		save_and_next(ls);
		return 1;
	}
	return 0;
}

static lexer_char keyword_or_name(this)
{
	lexer_char ch = is_reserved(&ls->buffer, ls->seminfo);

	if (ch == 0)
	{
		// ls->seminfo->str_ = take_string(String_take_value(&ls->buffer), ls->buffer.length);
		ls->seminfo->str_ = copy_string(ls->buffer.value, ls->buffer.length, NULL);
		return TK_NAME;
	}

	return ch;
}

static int isneg(cstr* string)
{
	if (**string == '-')
	{
		(*string)++;
		return 1;
	}
	else if (**string == '+')
	{
		(*string)++;
	}

	return 0;
}

// 1 -> int
// 2 -> float
static int str_2num(const char* s, TValue* result)
{
#define next() p++

	int dot = 0;

	const char* p = s;

	if (*p == '+' || *p == '-')
	{
		next();
	}

	massert(isdigit(*p), "Numbers must be starting with number, found '%c'", *p);

	for (; isdigit(*p) || *p == '.'; next())
	{
		if (*p == '.')
		{
			if (dot)
			{
				error("Multiple dots in number");
			}
			dot = 1;
		}
	}

	if (isalpha(*p))
	{
		error("Number touching a letter");
	}

	if (!dot) // just integer value
	{
		long long val = strtoll(s, NULL, 10);

		if (errno == ERANGE)
		{
			// errorf("Integer literal %s is too large", s);
			goto convert_to_number;
		}

		if (val > (long long) HAW_INT_MAX || val < (long long) HAW_INT_MIN)
		{
			// errorf("Integer literal %s exceeds limits of haw_int (%lld to %lld)", s,
			// (long long) HAW_INT_MIN, (long long) HAW_INT_MAX);
			goto convert_to_number;
		}

		setivalue(result, val);
		return 1;
	}
	else
	{
		goto convert_to_number;
	}
convert_to_number:
	// instead haw_number
	haw_number n = strtod(s, NULL);
	setnvalue(result, n);

	return 2;

#undef next
}

static void read_escape(this)
{
	char c;		 // result
	advance(ls); // start of escape sequence
	switch (ls->current)
	{
	case 'a':
		c = '\a'; // bell
		advance(ls);
		break;
	case 'b':
		c = '\b';
		advance(ls);
		break;
	case 'f':
		c = '\f';
		advance(ls);
		break;
	case 'e': // escape character
		c = '\e';
		advance(ls);
		break;
	case 'r': // caret return
	case 'n': // new line
		c = '\n';
		ls->line_number++;
		advance(ls);
		break;
	case 't': // horizontal tab
		c = '\t';
		advance(ls);
		break;
	case 'v': // vertical tab
		c = '\v';
		advance(ls);
		break;
	case '\\': // backslash
	case '"':  // "
	case '\'': // "
		c = ls->current;
		advance(ls);
		break;
	case EOF:
		return;
	default:
		save_and_next(ls);
		return;
	}

	save(ls, c);
}

static void read_string(this)
{
	assert(ls->current == '"');
	advance(ls); // "

	while (ls->current != '"')
	{
		switch (ls->current)
		{
		case EOF:
		case '\n':
		case '\r':
			error("Unfinished string");
			break;
		case '\\':
		{
			read_escape(ls);
			break;
		}
		default:
			save_and_next(ls);
		}
	}

	advance(ls); // "
	ls->seminfo->str_ = copy_string(ls->buffer.value, ls->buffer.length, NULL);
}

static lexer_char read_numeral(this)
{
	assert(isdigit(ls->current));
	save_and_next(ls);

	while (isdigit(ls->current) || ls->current == '.')
	{
		save_and_next(ls);
	}

	if (isalpha(ls->current))
	{
		error("Number touching a letter");
	}

	TValue obj;
	int	   t = str_2num(ls->buffer.value, &obj);

	if (t == 1)
	{
		ls->seminfo->int_ = int_value(&obj);
		return TK_INT;
	}
	else
	{
		ls->seminfo->num_ = number_value(&obj);
		return TK_NUMBER;
	}
}

void lex_init(this, str* source_name, SemInfo* seminfo)
{
	ls->source_name = source_name;
	ls->seminfo		= seminfo;

	buffer_init(&ls->file_contents);
	buffer_readfile(&ls->file_contents, source_name->value);

	advance(ls);
	String_init(&ls->buffer);
}

uint8_t current_is_new_line(this)
{
	return ls->current == '\n' || ls->current == '\r';
}

Token lex(this)
{
	Token result;
	String_clear(&ls->buffer); // clear buffer
#define result_tset(t)                                                                             \
	result.type = t;                                                                               \
	goto done;

	for (;;)
	{
		// #if ls_DEBUGL
		// 		printf("`%c` ", ls->current);
		// #endif
		switch (ls->current)
		{
		case EOF:
		case '\0':
			result_tset(TK_EOF);
			break;
		case '\n':
		case '\r':
			ls->line_number++;
			advance(ls);
			break;
		case ' ':
		case '\t':
		case '\f':
		case '\v':
			advance(ls);
			break;
#define doubletok(t, single_t, dt, double_t)                                                       \
	case t:                                                                                        \
	{                                                                                              \
		advance(ls);                                                                               \
		if (check_next1(ls, dt))                                                                   \
		{                                                                                          \
			result_tset(double_t);                                                                 \
		}                                                                                          \
		else                                                                                       \
		{                                                                                          \
			result_tset(single_t)                                                                  \
		}                                                                                          \
	}
			doubletok('+', '+', '+', TK_INC);
			doubletok('-', '-', '-', TK_DEC);
			doubletok('&', '&', '&', TK_AND);
			doubletok('|', '|', '|', TK_OR);
			doubletok('>', '>', '=', TK_GE);
			doubletok('/', '/', '/', TK_IDIV);
			doubletok('!', '!', '=', TK_NOTEQ);
		case '\\':
			advance(ls);
			if (check_next1(ls, '\\')) // comment
			{
				while (!current_is_new_line(ls) && ls->current != EOF)
				{
					advance(ls);
				}
				break;
			}
			else
			{
				result_tset('\\');
			}
		case '=':
			advance(ls);			  // =
			if (check_next1(ls, '=')) // ==
			{
				result_tset(TK_EQ);
			}
			else if (check_next1(ls, '>'))
			{
				result_tset(TK_FATARROW);
			}
			else
			{
				result_tset('=');
			}
		case '<':
			advance(ls);
			if (check_next1(ls, '=')) // <=
			{
				result_tset(TK_LE);
			}
			else if (check_next1(ls, '>'))
			{
				result_tset(TK_CONCAT);
			}
			else
			{
				result_tset('<');
			}
#undef doubletok
		case '"':
			read_string(ls);
			result_tset(TK_STRING);
		case '\'':
			advance(ls); // '

			if (check_next1(ls, '\''))
			{
				error("Expected char");
			}

			if (ls->current == '\\')
			{
				read_escape(ls);
			}
			else
			{
				save_and_next(ls); // save the char payload(which contained in '')
			}

			if (!check_next1(ls, '\''))
			{
				error("Expected end of char");
			}

			ls->seminfo->str_ = copy_string(ls->buffer.value, ls->buffer.length, NULL);
			result_tset(TK_CHAR);
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			result_tset(read_numeral(ls));
		}

		default:
		{
			if (isalpha(ls->current) || ls->current == '_')
			{
				save_and_next(ls);
				while (isalnum(ls->current) || ls->current == '_')
				{
					save_and_next(ls);
				}

				result_tset(keyword_or_name(ls));
			}
			else
			{
				// single char tokens
				lexer_char c = ls->current;
				advance(ls);

				result_tset(c);
			}
		}
		}
	}

done:
	result.seminfo = *ls->seminfo;
	return result;

#undef result_tset
}

void lex_destroy(this)
{
	String_destroy(&ls->buffer);
	buffer_destroy(&ls->file_contents);
}

int dislex_lastline = 0;

void dislex(this, lexer_char token)
{
	if (ls->line_number > dislex_lastline)
	{
		printf("\n");
		dislex_lastline = ls->line_number;
	}

	if (token < FIRST_RESERVED && isprint(token)) // single byte symbols?
	{
		printf("%c ", token);
	}

	else
	{
		const char* s = tok_2str(token);
		printf("%s ", s);
	}
}

#define MAX_TOKEN_BUFF 32

cstr_mut tok_2str(lexer_char token)
{
	static char s[MAX_TOKEN_BUFF];
	if (token < FIRST_RESERVED) // single byte symbols?
	{
		if (isprint(token))
		{
			snprintf(s, MAX_TOKEN_BUFF, "%c", token);
		}
		else
		{
			snprintf(s, MAX_TOKEN_BUFF, "\\%d", token);
		}
	}
	else
	{
		const char* str = haw_tokens[token - FIRST_RESERVED];
		sprintf(s, "%s", str);
	}

	return s;
}

#undef assignl
#undef this
