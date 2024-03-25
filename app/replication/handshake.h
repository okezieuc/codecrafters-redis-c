#ifndef REDIS_HANDSHAKE
#define REDIS_HANDSHAKE

#include "../server.h"

int send_handshake(struct ServerMetadata server_meta_data)
{
    struct sockaddr_in client_addr, master_node_addr;
    int client_fd;
    char buffer[1024], message[1024], *msg_body, *ptr;
    struct RESPSimpleStringNode *res_ss_data;
    struct RESPArrayNode *msg_array;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1)
    {
        printf("Socket creation failed: %s...\n", strerror(errno));
        return -1;
    }

    master_node_addr.sin_family = AF_INET;
    master_node_addr.sin_port = htons(server_meta_data.master_port);
    struct hostent *mh;
    mh = gethostbyname(server_meta_data.master_host);
    memcpy(&master_node_addr.sin_addr, mh->h_addr_list[0], mh->h_length);
    connect(client_fd, (struct sockaddr *)&master_node_addr, sizeof(master_node_addr));

    // send PING to master
    strcpy(message, "*1\r\n$4\r\nping\r\n");
    send(client_fd, message, strlen(message), 0);

    // receive a response from the master server
    recv(client_fd, buffer, 1024, 0);
    ptr = buffer;
    res_ss_data = parse_resp_simple_string(&ptr);

    if (strcmp(res_ss_data->data, "PONG") != 0)
    {
        printf("Received non-PONG response in handshake. Received %s. Raw buffer value is %s\n", res_ss_data, buffer);
        return -1;
    }

    // send REPLCONF with listening-port to master
    msg_array = create_resp_array_node(3);
    msg_array->item_ptrs[0] = (struct RESPNode *)create_resp_bulk_string_node("REPLCONF");
    msg_array->item_ptrs[1] = (struct RESPNode *)create_resp_bulk_string_node("listening-port");
    sprintf(buffer, "%d", server_meta_data.port);
    msg_array->item_ptrs[2] = (struct RESPNode *)create_resp_bulk_string_node(buffer);

    ptr = encode_resp_array(msg_array);
    send(client_fd, ptr, strlen(ptr), 0);
    free_resp_array_node(msg_array);
    free(ptr);

    recv(client_fd, buffer, 1024, 0);
    ptr = buffer;
    res_ss_data = parse_resp_simple_string(&ptr);

    if (strcmp(res_ss_data->data, "OK") != 0)
    {
        printf("Received non-OK response to REPLCONF. Received: %s\n", res_ss_data->data);
        return -1;
    }
    free(res_ss_data);

    // send REPLCONF with capa to master
    msg_array = create_resp_array_node(3);
    msg_array->item_ptrs[0] = (struct RESPNode *)create_resp_bulk_string_node("REPLCONF");
    msg_array->item_ptrs[1] = (struct RESPNode *)create_resp_bulk_string_node("capa");
    msg_array->item_ptrs[2] = (struct RESPNode *)create_resp_bulk_string_node("psync2");

    ptr = encode_resp_array(msg_array);
    send(client_fd, ptr, strlen(ptr), 0);
    free_resp_array_node(msg_array);
    free(ptr);

    recv(client_fd, buffer, 1024, 0);
    ptr = buffer;
    res_ss_data = parse_resp_simple_string(&ptr);

    if (strcmp(res_ss_data->data, "OK") != 0)
    {
        printf("Received non-OK response to REPLCONF. Received: %s\n", res_ss_data->data);
        return -1;
    }
    free(res_ss_data);

    // send PSYNC  with replication ID and offset to master
    msg_array = create_resp_array_node(3);
    msg_array->item_ptrs[0] = (struct RESPNode *)create_resp_bulk_string_node("PSYNC");
    msg_array->item_ptrs[1] = (struct RESPNode *)create_resp_bulk_string_node(server_meta_data.master_replication_id);
    sprintf(buffer, "%d", server_meta_data.master_offset);
    msg_array->item_ptrs[2] = (struct RESPNode *)create_resp_bulk_string_node(buffer);

    ptr = encode_resp_array(msg_array);
    send(client_fd, ptr, strlen(ptr), 0);
    free_resp_array_node(msg_array);
    free(ptr);
}

#endif