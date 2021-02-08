#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "tree.h"

/* private */

struct tree_node_t* create_node(const char *key, const void *value, const size_t value_size)
{
    struct tree_node_t *node;

    node = malloc(sizeof(struct tree_node_t));

    if (node)
    {
        node->value = malloc(value_size);
        if (node->value) 
        {
            memcpy(node->value, value, value_size);
            node->value_size = value_size;
            strcpy(node->key, key);
            node->left = NULL;
            node->right = NULL;
        }
        else
        {
            free(node);
            node = NULL;
        }
    }

    return node;
}

void free_node(struct tree_node_t *node)
{
    free(node->value);
    free(node);
}

struct tree_node_t* min_value_node(struct tree_node_t *node)
{
    struct tree_node_t *current = node;
    while (current && current->left != NULL)
        current = current->left;
    return current;
}

int tree_node_insert(struct tree_node_t **root, const char *key, const void *value, const size_t value_size)
{
    int cmp;

    if (*root == NULL)
    {
        struct tree_node_t *node = create_node(key, value, value_size);
        if (!node)
            return -1;
        *root = node;
        return 0;
    }
    
    cmp = strcmp(key, (*root)->key);

    if (cmp == 0)
    {
        if ((*root)->value_size != value_size)
            return -2;
        // update value
        memcpy((*root)->value, value, value_size);
        return 0;
    }

    if (cmp < 0)
        return tree_node_insert(&((*root)->left), key, value, value_size);
    return tree_node_insert(&((*root)->right), key, value, value_size);
}

int tree_node_insert_all(struct tree_t *dst, struct tree_node_t *root)
{
    int ret = 0;

    // insert root
    ret = tree_node_insert(&(dst->root), root->key, root->value, root->value_size);

    // insert left
    if (!ret && root->left != NULL)
        ret = tree_node_insert_all(dst, root->left);

    // insert right
    if (!ret && root->right != NULL)
        ret = tree_node_insert_all(dst, root->right);

    return ret;
}

int tree_node_delete(struct tree_node_t **root, const char *key)
{
    int cmp;

    if (*root == NULL) 
        return -1;

    cmp = strcmp(key, (*root)->key);
    if (cmp == 0)
    {
        if ((*root)->left == NULL)
        {
            struct tree_node_t *temp = (*root)->right;
            free_node(*root);
            *root = temp;
        }
        else if ((*root)->right == NULL)
        {
            struct tree_node_t *temp = (*root)->left;
            free_node(*root);
            *root = temp;
        }
        else
        {
            struct tree_node_t *temp = min_value_node((*root)->right);
            strcpy((*root)->key, temp->key);
            if ((*root)->value_size != temp->value_size)
                return -2;
            // update value
            memcpy((*root)->value, temp->value, temp->value_size);
            tree_node_delete(&((*root)->right), temp->key);
        }
    }
    else if (cmp < 0)
        tree_node_delete(&((*root)->left), key);
    else
        tree_node_delete(&((*root)->right), key);

   return 0;
}

void tree_node_apply_pre(struct tree_node_t *root, void (*f)(struct tree_node_t*, void*), void *arg)
{
    if (root == NULL)
        return;
    f(root, arg);
    tree_node_apply_pre(root->left, f, arg);
    tree_node_apply_pre(root->right, f, arg);
}

/* public */

struct tree_t* tree_init()
{
    struct tree_t *tree = (struct tree_t*) malloc(sizeof(struct tree_t));
    tree->max_size = TREE_DEFAULT_MAX_SIZE;
    tree->root = NULL;
    tree->size = 0;
    return tree;
}

void tree_clear(struct tree_t *tree)
{

}

int tree_insert(struct tree_t *tree, const char *key, const void *value, const size_t value_size)
{
    int ret;
    if (tree->size == tree->max_size)
        return -1;
    ret = tree_node_insert(&(tree->root), key, value, value_size);
    if (ret == 0)
        tree->size += 1;
    return ret;
}

int tree_insert_all(struct tree_t *dst, struct tree_t *src)
{
    int ret = 0;
    struct tree_node_t *src_root = src->root;

    // insert root
    ret = tree_insert(dst, src_root->key, src_root->value, src_root->value_size);

    // insert left
    if (!ret && src_root->left != NULL)
        ret = tree_node_insert_all(dst, src_root->left);

    // insert right
    if (!ret && src_root->right != NULL)
        ret = tree_node_insert_all(dst, src_root->right);

    return ret;
}

struct tree_node_t* tree_search(struct tree_t *tree, const char *key)
{
    int cmp;
    struct tree_node_t *root = tree->root;

    while (root != NULL)
    {
        cmp = strcmp(key, root->key);
        if (cmp == 0)
        {
            return root;
        }
        else if (cmp < 0)
            root = root->left;
        else
            root = root->right;
    }

    return NULL;
}

int tree_delete(struct tree_t *tree, const char *key)
{
    int ret;
    if (!tree->size)
        return -1;
    ret = tree_node_delete(&(tree->root), key);
    if (ret == 0)
        tree->size -= 1;
    return ret;
}

void tree_apply_pre(struct tree_t *tree, void (*f)(struct tree_node_t*, void*), void *arg)
{
    tree_node_apply_pre(tree->root, f, arg);
}
