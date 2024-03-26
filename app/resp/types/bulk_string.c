#include "bulk_string.h"
#include "../../global_imports.h"
#include "../../utils.h"
#include "../resp.h"
#include "generic_node.h"

struct RESPBulkStringNode *parse_resp_bulk_string(char **parser_iter_ptr)
{
    struct RESPBulkStringNode *node;
    char *ptr;

    // allocate memory
    node = (struct RESPBulkStringNode *)malloc(sizeof(struct RESPBulkStringNode));
    node->metadata.type = '$';

    ptr = *parser_iter_ptr + 1;
    int length = parse_number(&ptr, 1);
    node->length = length;
    ptr += 3;

    strncpy(node->data, ptr, 1023);

    node->data[length] = '\0';

    *parser_iter_ptr = ptr + length + 2;
    return node;
}

char *encode_resp_bulk_string(struct RESPBulkStringNode *node)
{
    char *encoding_ptr;
    int len;

    // encode NULL bulk strings
    if (node == NULL)
    {
        encoding_ptr = malloc(6);
        strcpy(encoding_ptr, "$-1\r\n");
        return encoding_ptr;
    }

    len = strlen(node->data);
    // 4 here represents the log of the maximum length of stored bytes + 1. since
    // we store at most 1023 bytes, this is at most 4. change this if the size of
    // RESPBulkStringNode.data changes.
    encoding_ptr = (char *)malloc(len + 5 + 4);

    strcpy(encoding_ptr, "$");
    sprintf(encoding_ptr + 1, "%d", len);
    strcat(encoding_ptr, "\r\n");
    strcat(encoding_ptr, node->data);
    strcat(encoding_ptr, "\r\n");

    return encoding_ptr;
}

struct RESPBulkStringNode *create_resp_bulk_string_node(char *str)
{
    struct RESPBulkStringNode *node = malloc(sizeof(struct RESPBulkStringNode));

    node->metadata.type = '$';
    node->length = strlen(str);
    strcpy(node->data, str);

    return node;
}
