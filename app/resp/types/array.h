#ifndef RESP_ARRAY_H
#define RESP_ARRAY_H

#include "generic_node.h"

/**
 * Stores up to 1023 bytes. Our implementation of encode_resp_array requires us to have
 * at most 999 items in our array.
 */
struct RESPArrayNode
{
    struct RESPNodeMetadata metadata;
    int length;
    struct RESPNode *item_ptrs[0];
};

struct RESPArrayNode *parse_resp_array(char **parser_iter_ptr);
char *encode_resp_array(struct RESPArrayNode *node);
struct RESPArrayNode *create_resp_array_node(int length);
int free_resp_array_node(struct RESPArrayNode *node);

#endif