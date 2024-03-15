#ifndef RESP_PARSER_H
#define RESP_PARSER_H

#include "types/simple_string.h"

// this RESP parser accepts an RESP-formatted request string
// and parses the contents of that string into a more useful format.

/**
 * RESPNodeMetaData contains one single attribute, type, that contains the character
 * that corresponds to the RESP data type that the RESP node contains.
 */
struct RESPNodeMetadata
{
    char type;
};

/**
 * RESP Node is an abstraction of the struct types we would have for the different RESP
 * data types. As the first few sizeof(RESPNodeMetadata) bytes of the struct would all
 * contain data of the kind of RESPNodeMetadata, we can, when unsure of what an RESP node
 * struct contains, cast all such structs to this type and based on what is stored in type,
 * recast the struct to its actual type.
 */
struct RESPNode
{
    struct RESPNodeMetadata metadata;
};

char *encode_resp_node(struct RESPNode *node)
{
    switch (node->metadata.type)
    {
    case '+':
        return encode_resp_simple_string((RESPSimpleStringNode *)node);
        break;
    }
}

struct RESPNode *parse_resp_node(char *resp_string)
{
    switch (resp_string[0])
    {
    case '+':
        return (RESPNode *)parse_resp_simple_string(resp_string);
        break;
    }
}

#endif
