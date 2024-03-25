#include <sys/socket.h>
#include <string.h>
#include "resp.h"

/**
 * Sends the simple string encoding of string str to the file descriptor fd.
 */
int send_simple_string(int client_fd, char *str)
{
    struct RESPSimpleStringNode *res_data_node;
    char *res_body;

    res_data_node = create_resp_simple_string_node(str);
    res_body = encode_resp_simple_string(res_data_node);

    send(client_fd, res_body, strlen(res_body), 0);

    free(res_data_node);
    free(res_body);

    return 0;
}

/**
 * Sends the bulk string encoding of string str to the file descriptor fd.
 */
int send_bulk_string(int client_fd, char *str)
{
    struct RESPBulkStringNode *res_data_node;
    char *res_body;

    res_data_node = (str == NULL) ? NULL : create_resp_bulk_string_node(str);
    res_body = encode_resp_bulk_string(res_data_node);

    send(client_fd, res_body, strlen(res_body), 0);

    free(res_data_node);
    free(res_body);

    return 0;
}