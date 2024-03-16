#ifndef LL_DICT_H
#define LL_DICT_H

#include <stdlib.h>
#include <string.h>

struct DictNode
{
    char key[64];
    struct DictNode *next;
    void *value;
};

// we want to have a dict type that points to the first node
// in our dictionary.
struct Dict
{
    struct DictNode *head;
    int value_size;
};

struct Dict *create_dict(int value_size);
void *dict_get_traversal(struct DictNode *, char *);
void *get_dict_item(struct Dict *, char *);
struct DictNode *dict_set_traversal(struct DictNode *, char *, void *, int);
struct DictNode *set_dict_item(struct Dict *, char *, void *);
void dict_del_traversal(struct DictNode *, char *, struct DictNode *);
void del_dict_item(struct Dict *, char *);

struct Dict *create_dict(int value_size)
{
    struct Dict *new_dict_ptr;

    new_dict_ptr = (struct Dict *)malloc(sizeof(struct Dict));

    // assume memory allocation was successful
    // and initialize head to NULL.
    new_dict_ptr->head = NULL;
    new_dict_ptr->value_size = value_size;

    return new_dict_ptr;
}

void *dict_get_traversal(struct DictNode *dict_node, char *key)
{
    // compare the keys
    if (strcmp(dict_node->key, key) == 0)
    {
        // return the pointer for the value stored in this dictionary node
        return dict_node->value;
    }

    // if this node is not the node we are looking for

    // check if we are at the last node of our dictionary
    if (dict_node->next == NULL)
    {
        return NULL;
    }

    // if this is not the last node, continue checking through
    // subsequent nodes for the thing we are looking for.
    return dict_get_traversal(dict_node->next, key);
}

/**
 * This function accepts a Dict struct and a key. It returns a pointer to the value
 * stored at the key, if one exists, and NULL otherwise.
 */
void *get_dict_item(struct Dict *dict, char *key)
{
    // handle empty dictionary case

    if (dict->head == NULL)
    {
        return NULL;
    }

    // if at least one item in the dictionary exists, traverse
    // through the items in the list till you find in item.
    return dict_get_traversal(dict->head, key);
}

struct DictNode *dict_set_traversal(struct DictNode *dict_node, char *key, void *value, int value_size)
{
    // check if the key of our current node matches the key
    // of the node we want to create/update
    if (strcmp(dict_node->key, key) == 0)
    {
        // return this node
        return dict_node;
    }

    // if this is not it, check if we have a next node
    if (dict_node->next == NULL)
    {
        // allocate memory for a new node, set the next pointer of our current node
        // to this new node and set the next pointer of this new node to NULL
        // and return the address of this new node
        struct DictNode *new_node;
        new_node = (struct DictNode *)malloc(sizeof(struct DictNode));
        new_node->value = malloc(value_size);
        new_node->next = NULL;
        dict_node->next = new_node;
        return new_node;
    }

    // if we have a next node, continue our check in this
    // node node
    return dict_set_traversal(dict_node->next, key, value, value_size);
}

struct DictNode *set_dict_item(struct Dict *dict, char *key, void *value)
{
    // if the dictionary is empty, create a new item to store
    // this key-value entry

    if (dict->head == NULL)
    {
        struct DictNode *new_dict_entry;
        new_dict_entry = (struct DictNode *)malloc(sizeof(struct DictNode));
        strcpy(new_dict_entry->key, key);
        new_dict_entry->value = malloc(dict->value_size);
        memcpy(new_dict_entry->value, value, dict->value_size);
        new_dict_entry->next = NULL;
        dict->head = new_dict_entry;
        return new_dict_entry;
    }

    // if the dictionary is not empty, traverse till you get till you get to
    // the node that has the same key as the thing you want to set
    struct DictNode *node_to_set = dict_set_traversal(dict->head, key, value, dict->value_size);
    strcpy(node_to_set->key, key);
    memcpy(node_to_set->value, value, dict->value_size);
    return node_to_set;
}

void dict_del_traversal(struct DictNode *dict_node, char *key, struct DictNode *caller_next_pointer)
{
    if (strcmp(dict_node->key, key) == 0)
    {
        // this is the node whose value we want to delete
        caller_next_pointer = dict_node->next;

        // deallocate the memory for this current node and it's value
        free(dict_node->value);
        free(dict_node);

        return;
    }

    // if this node does not have a next node, return;
    if (dict_node->next == NULL)
    {
        return;
    }

    // if we have a next item, we get our traversal algorithm to work on
    // this next item
    return dict_del_traversal(dict_node->next, key, dict_node->next);
}

void del_dict_item(struct Dict *dict, char *key)
{
    // if the dictionary is empty, return null
    if (dict->head == NULL)
    {
        return;
    }

    // if the dictionary is not empty, iterate over
    // it till you find an item that has the same key
    return dict_del_traversal(dict->head, key, dict->head);
}

#endif
