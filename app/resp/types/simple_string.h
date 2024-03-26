#ifndef RESP_SIMPLE_STRING_H
#define RESP_SIMPLE_STRING_H

#include "generic_node.h"

/**
 * Stores strings of up to 1023 characters.
 */
struct RESPSimpleStringNode
{
    struct RESPNodeMetadata metadata;
    char data[1024];
};

struct RESPSimpleStringNode *parse_resp_simple_string(char **parser_iter_ptr);
char *encode_resp_simple_string(struct RESPSimpleStringNode *node);
struct RESPSimpleStringNode *create_resp_simple_string_node(char *data);

#endif