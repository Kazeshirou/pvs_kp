#pragma once

#include "copy_constructor.h"
#include "destructor.h"

typedef struct tree_node__t
{
    char key[255];
    void *value;
    struct tree_node__t *left;
    struct tree_node__t *right;
} tree_node_t;

#define TREE_DEFAULT_MAX_SIZE 10000

typedef struct tree__t 
{
    tree_node_t *root;
    size_t size;
    size_t max_size;

    size_t value_size;
    copy_constructor_t value_copy_constr;
    destructor_t value_destr;
} tree_t;

tree_t* tree_init(size_t value_size, 
                  copy_constructor_t value_copy_constr, 
                  destructor_t value_destr);
void tree_clear(tree_t *tree);

int tree_insert(tree_t *tree, const char *key, const void *value);
int tree_insert_all(tree_t *dst, tree_t *src);
int tree_delete(tree_t *tree, const char *key);
tree_node_t* tree_search(tree_t *tree, const char *key);
void tree_apply_pre(tree_t *tree, void (*f)(tree_node_t*, void*), void *arg);
tree_node_t** tree_nodes_to_array(const tree_t *tree);

#define TREE_INIT(type, value_copy_constr, value_destr) \
    tree_init(sizeof(type), value_copy_constr, value_destr)

#define TREE_INSERT(tree, key, value) \
    tree_insert(tree, key, &value)

