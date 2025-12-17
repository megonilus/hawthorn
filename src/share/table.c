#include "share/array.h"
#include "type/type.h"
#include "value/obj.h"
#include "value/value.h"

#include <share/table.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static Entry* find_entry(Entry* entries, haw_string* key)
{
	hash index = key->hash % array_capacity(entries);

	for (;;)
	{
		Entry* entry = &entries[index];
		if (entry->key == key || entry->key == NULL)
		{
			return entry;
		}

		index = (index + 1) % array_capacity(entries);
	}
}

static void adjust_capacity(Table* table, size_t new_capacity)
{
	Entry* new_entries = array(Entry);
	array_reserve(new_entries, new_capacity, Entry);

	for (size_t i = 0; i < new_capacity; i++)
	{
		new_entries[i].key		  = NULL;
		new_entries[i].value.type = HAW_TNONE;
	}

	array_size(new_entries) = 0;
	for (size_t i = 0; i < array_capacity(table->entries); i++)
	{
		Entry* entry = &table->entries[i];
		if (entry->key != NULL)
		{
			Entry* dest = find_entry(new_entries, entry->key);
			dest->key	= entry->key;
			dest->value = entry->value;
			array_incsize(new_entries);
		}
	}

	array_free(table->entries);
	table->entries = new_entries;
}

void table_set(Table* table, haw_string* key, TValue value)
{
	if (array_size(table->entries) + 1 > array_capacity(table->entries) * TABLE_MAX_LOAD)
	{
		adjust_capacity(table, array_capacity(table->entries) * 2);
	}

	Entry* entry = find_entry(table->entries, key);

	if (entry->key == NULL)
	{
		array_incsize(table->entries);
	}

	entry->key	 = key;
	entry->value = value;
}

void table_init(Table* table)
{
	table->entries = array(Entry);
}

void table_destroy(Table* table)
{
	array_free(table->entries);
}

void table_copy(Table* to, Table* from)
{
	for (size_t i = 0; i < array_capacity(from->entries); i++)
	{
		Entry* entry = &from->entries[i];
		if (entry->key != NULL)
		{
			table_set(to, entry->key, entry->value);
		}
	}
}

int table_get(Table* table, haw_string* key, TValue* value)
{
	if (array_empty(table->entries))
	{
		return 0;
	}

	Entry* entry = find_entry(table->entries, key);

	if (entry->key == NULL || entry->key != key)
	{
		return 0;
	}

	*value = entry->value;

	return 1;
}

haw_string* table_find_string(Table* table, const char* chars, size_t length, hash hash)
{
	if (array_empty(table->entries))
	{
		return NULL;
	}

	uint32_t index = hash % array_capacity(table->entries);
	for (;;)
	{
		Entry* entry = &table->entries[index];
		if (entry->key == NULL)
		{
			return NULL;
		}
		else if (entry->key->length == length && entry->key->hash == hash &&
				 memcmp(entry->key->chars, chars, length) == 0)
		{
			return entry->key;
		}

		index = (index + 1) % array_capacity(table->entries);
	}
}
