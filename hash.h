#ifndef _HASH_H_
#define _HASH_H_

#include "list.h"

struct hash_node {
	char *key;
	void *value;
	struct hlist_node node;
};

struct hlist_head *hash_init(int size);
int hash_add(struct hlist_head *hash_table, const char *key, void *value);
int hash_find(const struct hlist_head *hash_table, const char *key, struct hash_node **node, size_t size);
void hash_print(const struct hlist_head *hash_table);
void hash_free_node(struct hash_node *node);
void hash_free(struct hlist_head *hash_table);

#endif /* _HASH_H_ */
