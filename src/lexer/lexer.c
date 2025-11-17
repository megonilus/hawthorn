#include "lexer/token.h"
#include "type/type.h"
#include "value/value.h"

#include <assert.h>
#include <ctype.h>
#include <lexer/lexer.h>
#include <share/array.h>
#include <share/error.h>
#include <share/hawthorn.h>
#include <share/string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MODULE_NAME "lexer"

#define save(sls, c) String_appendc(&sls->buffer, c);
// #define save(sls, c) String_appendc(&sls->buffer, c)
#define save_and_next(sls)                                                                         \
	do                                                                                             \
	{                                                                                              \
		save(sls, sls->current);                                                                   \
		advance(sls);                                                                              \
	} while (0)

static inline void advance(this)
{
	if (sls->pos + 1 < sls->file_contents.length)
	{
		sls->current = sls->file_contents.value[++sls->pos];
	}
	else
	{
		sls->current = EOF;
	}
}

cstr_mut get_contents_of_file(cstr file_name)
{
	FILE* file = fopen(file_name, "rb");

	if (file == NULL)
	{
		error("Could not open file");
	}

	fseek(file, 0L, SEEK_END);
	size_t file_size = ftell(file);
	rewind(file);

	char* buffer = (char*) malloc(file_size + 1);

	if (buffer == NULL)
	{
		error("Error allocating memory to file buffer");
	}

	size_t bytes_read = fread(buffer, sizeof(char), file_size, file);

	if (bytes_read < file_size)
	{
		errorf("Could not open file %s", file_name);
	}

	fclose(file);

	buffer[file_size] = '\0';
	return buffer;
}

static noret lex_error(this, cstr msg, TokenType token);

static lexer_char check_next1(this, lexer_char c)
{
	if (sls->current == c)
	{
		advance(sls);
		return 1;
	}
	return 0;
}

static lexer_char is_reserved(const String* str)
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
			case 'u':
				KW_CHECK_PART(2, "n", TK_FUN);
				break;
			case 'a':
				KW_CHECK_PART(2, "lse", TK_BOOL);
				break;
			}
		}
		break;
	case 'g':
		KW_CHECK_FULL("global", TK_GLOBAL);
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
	case 'l':
		KW_CHECK_FULL("local", TK_LOCAL);
		break;
	case 'o':
		KW_CHECK_FULL("or", TK_OR);
		break;
	case 'p':
		KW_CHECK_FULL("pro", TK_PRO);
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
				break;
			}
		}
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
	if (sls->current == set[0] || sls->current == set[1])
	{
		save_and_next(sls);
		return 1;
	}
	return 0;
}

static lexer_char keyword_or_name(this)
{
	lexer_char ch = is_reserved(&sls->buffer);

	if (ch == 0)
	{
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

	assert(isdigit(*p));

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
		haw_int i = strtoll(s, NULL, 10);
		setivalue(result, i);
		return 1;
	}

	// instead haw_number
	haw_number n = strtod(s, NULL);
	setnvalue(result, n);

	return 2;

#undef next
}

static lexer_char read_numeral(this, SemInfo* seminfo)
{
	assert(isdigit(sls->current));
	save_and_next(sls);

	while (isdigit(sls->current) || sls->current == '.')
	{
		save_and_next(sls);
	}

	if (isalpha(sls->current))
	{
		error("Number touching a letter");
	}

	TValue obj;
	int	   t = str_2num(sls->buffer.value, &obj);

	if (t == 1)
	{
		seminfo->int_ = int_value(&obj);
		return TK_INT;
	}
	else
	{
		seminfo->num_ = number_value(&obj);
		return TK_NUMBER;
	}
}

void synlex_init(this, str* source_name)
{
	sls->source_name   = source_name;
	sls->file_contents = make_str(get_contents_of_file(source_name->value));
	sls->pos		   = 0;
	sls->current	   = sls->file_contents.value[sls->pos];

	String_init(&sls->buffer);
}

uint8_t current_is_new_line(this)
{
	return sls->current == '\n' || sls->current == '\r';
}

void synlex_ttype_to_str(lexer_char token, cstr_mut destiny)
{
	if (token < FIRST_RESERVED) // single byte symbols?
	{
		if (isprint(token))
		{
			sprintf(destiny, "%c", token);
		}
		else
		{
			sprintf(destiny, "\\%d", token);
		}
	}
	else
	{
		const char* s = haw_tokens[token - FIRST_RESERVED];
		sprintf(destiny, "%s", s);
	}
}

lexer_char synlex_lex(this, SemInfo* seminfo)
{
	String_clear(&sls->buffer); // clear buffer

	for (;;)
	{
		// #if SLS_DEBUGL
		// 		printf("`%c` ", sls->current);
		// #endif
		switch (sls->current)
		{
		case '\n':
		case '\r':
			sls->line_number++;
			advance(sls);
			break;
		case ' ':
		case '\t':
		case '\f':
		case '\v':
			advance(sls);
			break;
		case '#': // comment
			while (!current_is_new_line(sls) && sls->current != EOF)
			{
				advance(sls);
			}
			break;
		case '=':
			advance(sls);
			if (check_next1(sls, '=')) // ==
			{
				return TK_EQ;
			}
			return '=';
		case '<':
			advance(sls);
			if (check_next1(sls, '='))
			{
				return TK_LE;
			}
			return '<';
		case '>':
			advance(sls);
			if (check_next1(sls, '='))
			{
				return TK_GE;
			}
			return '>';
		case '/':
			advance(sls);
			if (check_next1(sls, '/'))
			{
				return TK_IDIV;
			}
			return '/';
		case '!':
			advance(sls);
			if (check_next1(sls, '='))
			{
				return TK_NOTEQ;
			}
			return '!';
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
			return read_numeral(sls, seminfo);
		}
		default:
		{
			if (isalpha(sls->current))
			{
				save_and_next(sls);
				while (isalpha(sls->current))
				{
					save_and_next(sls);
				}

				return keyword_or_name(sls);
			}
			else
			{
				// single char tokens
				lexer_char c = sls->current;
				advance(sls);

				return c;
			}
		}
		}
	}
}

void synlex_destroy(this)
{
	String_destroy(&sls->buffer);
	free((char*) sls->file_contents.value);
}

void synlex_dislex(lexer_char token)
{
	if (token < FIRST_RESERVED) // single byte symbols?
	{
		if (isprint(token))
		{
			printf("%c ", token);
		}
		else
		{
			printf("\\%d", token);
		}
	}
	else
	{
		const char* s = haw_tokens[token - FIRST_RESERVED];
		printf("%s ", s);
	}
}

#undef assignl
#undef this
#undef this_t
