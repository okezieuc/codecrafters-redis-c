#ifndef RESP_GENERIC_H
#define RESP_GENERIC_H

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

struct RESPNode *parse_resp_node(char **);
char *encode_resp_node(struct RESPNode *);

#endif