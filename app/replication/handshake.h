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

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = 0;
    bind(client_fd, (struct sockaddr *)&client_addr, sizeof(client_addr));

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


#endif