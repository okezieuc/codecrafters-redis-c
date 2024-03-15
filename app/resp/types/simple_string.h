#ifndef RESP_SIMPLE_STRING_H
#define RESP_SIMPLE_STRING_H

#include "../parser.h"
#include <stdlib.h>
#include <string.h>

/**
 * Stores strings of up to 1023 characters.
 */
struct RESPSimpleStringNode
{
    struct RESPNodeMetadata metadata;
    char data[1024];
};

/**
 * This function parses an RESP-formatted simple string into an RESPSimpleStringNode.
 * It updates the pointer passed to it to point to where the next item to be passed,
 * if one would exist and it returns a pointer to the created RESPSimpleStringNode.
 */
struct RESPSimpleStringNode *parse_resp_simple_string(char *parser_iter_ptr)
{
    struct RESPSimpleStringNode *node;
    char *ptr;

    // allocate memory
    node = (struct RESPSimpleStringNode *)malloc(sizeof(struct RESPSimpleStringNode));
    node->metadata.type = '-';

    // find the null terminator
    ptr = strstr(parser_iter_ptr, "\r\n");

    // we proceed, assuming ptr is not NULL, to copy the string to the allocated memory.
    // this is vulnerable to buffer overflow exploits.
    // a fix for this is to copy the smaller of 1023 and len(string) bytes
    // + 1 and -1 accomodate the plus in the RESP string
    strncpy(node->data, parser_iter_ptr + 1, (ptr - parser_iter_ptr - 1));
    node->data[(ptr - parser_iter_ptr - 1)] = '\0';

    parser_iter_ptr = ptr + 2;
    return node;
}

char *encode_resp_simple_string(struct RESPSimpleStringNode *node) {
    char *encoding_ptr;
    int len;

    len = strlen(node->data);
    encoding_ptr = (char *) malloc(len + 3);

    strcpy(encoding_ptr, "+");
    strcat(encoding_ptr, node->data);
    strcat(encoding_ptr, "\r\n");

    return encoding_ptr;
}

#endif