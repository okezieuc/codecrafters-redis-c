#ifndef RESP_BULK_STRING_H
#define RESP_BULK_STRING_H

#include "../resp.h"

/**
 * Stores up to 1023 bytes.
 */
struct RESPBulkStringNode
{
    struct RESPNodeMetadata metadata;
    char data[1024];
    int length;
};

/**
 * This function parses an RESP-formatted bulk string into an RESPBulkStringNode.
 * It updates the pointer passed to it to point to where the next item to be passed,
 * if one would exist and it returns a pointer to the created RESPBulkStringNode.
 */
struct RESPBulkStringNode *parse_resp_bulk_string(char **parser_iter_ptr);

char *encode_resp_bulk_string(struct RESPBulkStringNode *node);

struct RESPBulkStringNode *create_resp_bulk_string_node(char *str);

#endif