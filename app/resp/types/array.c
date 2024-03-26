#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../utils.h"
#include "generic_node.h"

/**
 * This function parses an RESP-formatted array string into an RESPArrayNode.
 * It updates the pointer passed to it to point to where the next item to be passed,
 * if one would exist and it returns a pointer to the created RESPArrayNode.
 */
struct RESPArrayNode *parse_resp_array(char **parser_iter_ptr)
{
    struct RESPArrayNode *node;
    struct RESPNode *array_item;
    char *ptr;
    int length;

    // check for the array length and allocate memory
    ptr = *parser_iter_ptr + 1;
    length = parse_number(&ptr, 1);
    node = (struct RESPArrayNode *)malloc(sizeof(struct RESPArrayNode) + (sizeof(struct RESPNode *) * length));

    // initialize RESPArrayNode struct
    node->metadata.type = '*';
    node->length = length;

    // parse the items in the RESP array
    ptr += 3;
    for (int i = 0; i < length; i++)
    {
        array_item = parse_resp_node(&ptr);
        node->item_ptrs[i] = array_item;
    }

    *parser_iter_ptr = ptr;

    return node;
}

char *encode_resp_array(struct RESPArrayNode *node)
{
    char *encoding_ptr, *item_encoding_ptrs[node->length];
    int len = 0;

    // the first three is for the null terminator, *, and \r\n
    // the second three is based on the assumption that our array contains at most
    // 999 items.
    len += 4 + 3;

    // encode all items in array
    for (int i = 0; i < node->length; i++)
    {
        item_encoding_ptrs[i] = encode_resp_node(node->item_ptrs[i]);
        len += strlen(item_encoding_ptrs[i]);
    }

    encoding_ptr = (char *)malloc(len);

    strcpy(encoding_ptr, "*");
    sprintf(encoding_ptr + 1, "%d", node->length);
    strcat(encoding_ptr, "\r\n");

    for (int i = 0; i < node->length; i++)
    {
        strcat(encoding_ptr, item_encoding_ptrs[i]);
        free(item_encoding_ptrs[i]);
    }

    return encoding_ptr;
}

struct RESPArrayNode *create_resp_array_node(int length)
{
    struct RESPArrayNode *node = (struct RESPArrayNode *)malloc(sizeof(struct RESPArrayNode) + sizeof(struct RESPNode *) * length);

    node->metadata.type = '*';
    node->length = length;

    return node;
}

int free_resp_array_node(struct RESPArrayNode *node)
{
    // free the memory associated with all items in the array
    for (int i = 0; i < node->length; i++)
    {
        free_resp_node(node->item_ptrs[i]);
    }

    free(node);

    return 1;
}
