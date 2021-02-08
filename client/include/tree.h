#pragma once

struct tree_node_t
{
    char key[255];
    void *value;
    size_t value_size;
    struct tree_node_t *left;
    struct tree_node_t *right;
};

#define TREE_DEFAULT_MAX_SIZE 10000

struct tree_t 
{
    struct tree_node_t *root;
    size_t size;
    size_t max_size;
};

struct tree_t* tree_init();
void tree_clear(struct tree_t *tree);

int tree_insert(struct tree_t *tree, const char *key, const void *value, const size_t value_size);
int tree_insert_all(struct tree_t *dst, struct tree_t *src);
int tree_delete(struct tree_t *tree, const char *key);
struct tree_node_t* tree_search(struct tree_t *tree, const char *key);
void tree_apply_pre(struct tree_t *tree, void (*f)(struct tree_node_t*, void*), void *arg);

