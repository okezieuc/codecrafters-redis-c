#ifndef RESP_PARSER_H
#define RESP_PARSER_H

#include "types/generic_node.h"
#include "types/array.h"
#include "types/bulk_string.h"
#include "types/simple_string.h"
#include "resp_res_handlers.h"

char *encode_resp_node(struct RESPNode *node);
struct RESPNode *parse_resp_node(char **resp_string);
int free_resp_node(struct RESPNode *node);


#endif