#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "debug.h"

static int hash_size;

static size_t hash(const char *string)
{
	size_t hashval = 0;
	size_t seed = 131;	/* 31 131 1313 13131 131313 .. */

	while (*string)
		hashval = hashval * seed + (size_t)*string++;

	return hashval;
}

static struct hash_node *new_hash_node(const char *key, void *value)
{
	struct hash_node *node;

	if (!(node = malloc(sizeof(struct hash_node))))
		return NULL;
	if (!(node->key = strdup(key)))
		return NULL;
	node->value = value;
	hlist_node_init(&node->node);

	return node;
}

struct hlist_head *hash_init(int size)
{
	int i;
	struct hlist_head *hash_table;

	hash_size = size;

	if (!(hash_table = malloc(sizeof(struct hlist_head) * hash_size)))
		return NULL;
	
	for (i = 0; i < hash_size; i++)
		INIT_HLIST_HEAD(hash_table + i);

	return hash_table;
}

int hash_add(struct hlist_head *hash_table, const char *key, void *value)
{
	int offset;
	struct hash_node *node;

	node = new_hash_node(key, value);
	if (!node)
		return -1;

	offset = hash(key) % hash_size;
	debug("offset = %d", offset);

	hlist_add_head(&node->node, hash_table + offset);

	return 0;
}

int hash_find(const struct hlist_head *hash_table, const char *key, struct hash_node **node, size_t size)
{
	int offset;
	size_t i = 0;
	struct hash_node *pos;

	if (!hash_table)
		return -1;

	offset = hash(key) % hash_size;

	hlist_for_each_entry(pos, hash_table + offset, node) {
		if (strcmp(pos->key, key) == 0) {
			if (i < size)
				node[i++] = pos;
			else
				break;
		}
	}

	return i;
}

void hash_print(const struct hlist_head *hash_table)
{
	int i;
	struct hash_node *pos;

	for (i = 0; i < hash_size; i++) {
		hlist_for_each_entry(pos, hash_table + i, node) {
			printf("key = %s\n", pos->key);
		}
	}
}

void hash_free_node(struct hash_node *node)
{
	if (!node)
		return;
	if (!hlist_unhashed(&node->node)) {
		debug("__hlist_del");
		__hlist_del(&node->node);
	}
	free(node->key);
	free(node);
}

void hash_free(struct hlist_head *hash_table)
{
	size_t i;
	struct hash_node *pos;

	for (i = 0; i < hash_size; i++) {
		hlist_for_each_entry(pos, hash_table + i, node) {
			hash_free_node(pos);
		}
	}
	free(hash_table);
	hash_size = 0;
	hash_table = NULL;
}
