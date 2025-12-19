#ifndef hawthorn_table
#define hawthorn_table

#include "value/obj.h"
#include "value/value.h"

// TODO: tune this
#define TABLE_MAX_LOAD 0.75

typedef struct
{
	haw_string* key;
	TValue		value;
} Entry;

typedef struct
{
	Entry* entries;
} Table;

void table_init(Table* table);
void table_destroy(Table* table);
void table_set(Table* table, haw_string* key, TValue value);
void table_copy(Table* to, Table* from);
void table_destroy(Table* table);

int table_get(Table* table, haw_string* key, TValue* value);
int table_delete(Table* table, haw_string* key);

haw_string* table_find_string(Table* table, const char* chars, size_t length, hash hash,
							  TValue* write);

#endif
