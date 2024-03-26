#include "resp.h"
#include <stdlib.h>
#include "types/generic_node.h"
#include "types/array.h"
#include "types/bulk_string.h"
#include "types/simple_string.h"
#include "resp_res_handlers.h"


// this RESP parser accepts an RESP-formatted request string
// and parses the contents of that string into a more useful format.

char *encode_resp_node(struct RESPNode *node)
{
    switch (node->metadata.type)
    {
    case '+':
        return encode_resp_simple_string((struct RESPSimpleStringNode *)node);
        break;
    case '$':
        return encode_resp_bulk_string((struct RESPBulkStringNode *)node);
        break;
    case '*':
        return encode_resp_array((struct RESPArrayNode *)node);
        break;
    }
}

/**
 * This function accepts a pointer to a pointer to the start of the RESP string.
 */
struct RESPNode *parse_resp_node(char **resp_string)
{
    switch (*resp_string[0])
    {
    case '+':
        return (struct RESPNode *)parse_resp_simple_string(resp_string);
        break;
    case '$':
        return (struct RESPNode *)parse_resp_bulk_string(resp_string);
        break;
    case '*':
        return (struct RESPNode *)parse_resp_array(resp_string);
        break;
    }
}

int free_resp_node(struct RESPNode *node)
{
    switch (node->metadata.type)
    {
    case '*':
        return free_resp_array_node((struct RESPArrayNode *)node);
        break;
    default:
        free(node);
    }

    return 1;
}
