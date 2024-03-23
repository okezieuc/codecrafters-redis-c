int send_handshake(char *host, int port)
{
    struct sockaddr_in client_addr, master_node_addr;
    int client_fd;

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
    master_node_addr.sin_port = htons(port);
    struct hostent *mh;
    mh = gethostbyname(host);
    memcpy(&master_node_addr.sin_addr, mh->h_addr_list[0], mh->h_length);
    connect(client_fd, (struct sockaddr *)&master_node_addr, sizeof(master_node_addr));

    char message[] = "*1\r\n$4\r\nping\r\n";
    send(client_fd, message, strlen(message), 0);
}