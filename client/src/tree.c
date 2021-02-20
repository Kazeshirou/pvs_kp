#include "tree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "global.h"

/* private */

tree_node_t* create_node(const tree_t* tree, const char* key,
                         const void* value) {
    tree_node_t* node;

    node = malloc(sizeof(tree_node_t));
    if (!node) {
        sprintf(g_log_message, "Ошибка выделения памяти: create_node()");
        send_log();
        return NULL;
    }

    node->value = copy(value, tree->value_size, tree->value_copy_constr);
    if (!node->value && value) {
        free(node);
        return NULL;
    }

    strcpy(node->key, key);
    node->left  = NULL;
    node->right = NULL;

    return node;
}

void free_node(const tree_t* tree, tree_node_t* node) {
    destruct(node->value, tree->value_destr);
    free(node);
}

tree_node_t* min_value_node(tree_node_t* node) {
    tree_node_t* current = node;
    while (current && current->left != NULL)
        current = current->left;
    return current;
}

int tree_node_insert(tree_t* tree, tree_node_t** root, const char* key,
                     const void* value) {
    if (tree->size == tree->max_size)
        return TREE_OVERFLOW;

    int cmp;

    if (*root == NULL) {
        tree_node_t* node = create_node(tree, key, value);
        if (!node)
            return MEMORY_ERROR;
        *root = node;
        tree->size++;
        return SUCCESS;
    }

    cmp = strcmp(key, (*root)->key);

    if (cmp == 0) {
        // update value
        destruct((*root)->value, tree->value_destr);
        (*root)->value = copy(value, tree->value_size, tree->value_copy_constr);
        if (!((*root)->value)) {
            return MEMORY_ERROR;
        }
        return SUCCESS;
    }

    if (cmp < 0)
        return tree_node_insert(tree, &((*root)->left), key, value);
    else
        return tree_node_insert(tree, &((*root)->right), key, value);
}

int tree_node_insert_all(tree_t* dst, tree_node_t* root) {
    int ret = 0;

    // insert root
    ret = tree_node_insert(dst, &(dst->root), root->key, root->value);

    // insert left
    if (!ret && root->left != NULL)
        ret = tree_node_insert_all(dst, root->left);

    // insert right
    if (!ret && root->right != NULL)
        ret = tree_node_insert_all(dst, root->right);

    return ret;
}

int tree_node_delete(tree_t* tree, tree_node_t** root, const char* key) {
    assert(tree->size > 0);
    assert(*root != NULL);

    int cmp;

    cmp = strcmp(key, (*root)->key);
    if (cmp == 0) {
        if ((*root)->left == NULL) {
            tree_node_t* temp = (*root)->right;
            free_node(tree, *root);
            *root = temp;
            tree->size--;
            return SUCCESS;
        } else if ((*root)->right == NULL) {
            tree_node_t* temp = (*root)->left;
            free_node(tree, *root);
            *root = temp;
            tree->size--;
            return SUCCESS;
        } else {
            tree_node_t* temp = min_value_node((*root)->right);
            strcpy((*root)->key, temp->key);
            // update value
            destruct((*root)->value, tree->value_destr);
            (*root)->value =
                copy(temp->value, tree->value_size, tree->value_copy_constr);
            if (!(*root)->value) {
                return MEMORY_ERROR;
            }
            return tree_node_delete(tree, &((*root)->right), temp->key);
        }
    }

    if (cmp < 0)
        return tree_node_delete(tree, &((*root)->left), key);
    else
        return tree_node_delete(tree, &((*root)->right), key);
}

/* public */

tree_t* tree_init(size_t value_size, copy_constructor_t value_copy_constr,
                  destructor_t value_destr) {
    tree_t* tree = (tree_t*)malloc(sizeof(tree_t));
    if (!tree) {
        sprintf(g_log_message, "Ошибка выделения памяти: tree_init()");
        send_log();
        return NULL;
    }

    tree->max_size = TREE_DEFAULT_MAX_SIZE;
    tree->root     = NULL;
    tree->size     = 0;

    tree->value_size        = value_size;
    tree->value_copy_constr = value_copy_constr;
    tree->value_destr       = value_destr;

    return tree;
}

void tree_clear(tree_t* tree) {
    tree_node_t** nodes = tree_nodes_to_array(tree);
    if (!nodes) {
        return;
    }

    int i = 0;
    for (i = 0; i < tree->size; i++) {
        free_node(tree, nodes[i]);
    }
    free(nodes);
    free(tree);
}

int tree_insert(tree_t* tree, const char* key, const void* value) {
    return tree_node_insert(tree, &(tree->root), key, value);
}

int tree_insert_all(tree_t* dst, tree_t* src) {
    int          ret      = 0;
    tree_node_t* src_root = src->root;

    // insert root
    ret = tree_insert(dst, src_root->key, src_root->value);

    // insert left
    if (!ret && src_root->left != NULL)
        ret = tree_node_insert_all(dst, src_root->left);

    // insert right
    if (!ret && src_root->right != NULL)
        ret = tree_node_insert_all(dst, src_root->right);

    return ret;
}

tree_node_t* tree_search(tree_t* tree, const char* key) {
    int          cmp;
    tree_node_t* root = tree->root;

    while (root != NULL) {
        cmp = strcmp(key, root->key);
        if (cmp == 0) {
            return root;
        } else if (cmp < 0)
            root = root->left;
        else
            root = root->right;
    }

    return NULL;
}

int tree_delete(tree_t* tree, const char* key) {
    return tree_node_delete(tree, &(tree->root), key);
}

void tree_node_nodes_to_array(const tree_t* tree, tree_node_t* root,
                              tree_node_t*** nodes, int* index) {
    if (!root)
        return;

    if (*index == 0) {
        *nodes = (tree_node_t**)malloc(sizeof(tree_node_t*) * tree->size);
        if (!(*nodes)) {
            sprintf(g_log_message,
                    "Ошибка выделения памяти: tree_node_nodes_to_array()");
            send_log();
            return;
        }
    }

    (*nodes)[*index] = (tree_node_t*)root;
    (*index)++;

    if (root->left)
        tree_node_nodes_to_array(tree, root->left, nodes, index);
    if (root->right)
        tree_node_nodes_to_array(tree, root->right, nodes, index);
}

tree_node_t** tree_nodes_to_array(const tree_t* tree) {
    tree_node_t** nodes = NULL;
    int           index = 0;
    tree_node_nodes_to_array(tree, tree->root, &nodes, &index);
    assert(tree->size == index);
    return nodes;
}