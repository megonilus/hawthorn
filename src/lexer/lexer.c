#include "lexer/token.h"
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

#define peek(i) (sls->file_contents.value[sls->pos + i])
#define advance(sls)                                                                               \
	sls->current	= sls->file_contents.value[sls->pos++];                                        \
	sls->look_ahead = peek(1);
#define push

cstr get_contents_of_file(cstr file_name)
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

	return buffer;
}

#define save(sls, c) String_appendc(&sls->buffer, c)
#define save_and_next(sls) save(sls, sls->current), advance(sls)

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

static int str_tonum(String* a, TValue* obj) {}

static lexer_char read_numeral(this, SemInfo* seminfo)
{
	TValue	   obj;
	lexer_char first = sls->current;
	assert(isdigit(sls->current));
	save_and_next(sls);
	const char* prefix = "\0";

	for (;;)
	{
		if (isdigit(sls->current) || sls->current == '.')
		{
			save_and_next(sls);
		}
		else
		{
			break;
		}
	}

	if (isalpha(sls->current))
	{
		synlex_lexerror(sls, "Number touching a letter");
	}

	if (str_tonum(&sls->buffer, &obj) == 0)
	{
		synlex_lexerror(sls, "Malform number");
	}

	if (tt_isint(&obj))
	{
		seminfo->int_ = int_value(&obj);
		return TK_INT;
	}

	else
	{
		assert(tt_isnumber(&obj));
		seminfo->num_ = number_value(&obj);

		return TK_NUMBER;
	}
}

static int is_reserved(String* string) {}

void synlex_init(this, str* source_name)
{
	sls->source_name   = source_name;
	sls->file_contents = make_str(get_contents_of_file(source_name->value));
	sls->current	   = sls->file_contents.value[0];
}

uint8_t current_is_new_line(this)
{
	return sls->current == '\n' || sls->current == '\r';
}

lexer_char synlex_lex(this, SemInfo* seminfo)
{

	for (;;)
	{
		switch (sls->current)
		{
		case '\n':
		case '\r':
			sls->line_number++;
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
			if (sls->look_ahead == '=')
			{
				return TK_EQ;
			}
			return '=';
			break;
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
				do
				{
					save_and_next(sls)
				} while (isalnum(sls->current));

				if (isreserved(sls->buffer))
				{
				}
			}
			else
			{ // single char tokens
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
}

#undef assignl
#undef this
#undef this_t
