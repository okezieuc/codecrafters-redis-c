#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

void *handle_req(void *);

void *handle_req(void *arg)
{
	pthread_detach(pthread_self());

	// store a copy of the client_fd in this context
	int client_fd = *(int *)arg;
	char req_buffer[1024], res_buffer[1024];

	// handle multiple requests from single client
	while (1)
	{
		// receive messge from client
		recv((int)client_fd, req_buffer, 1023, 0);

		// respond to the client
		strcpy(res_buffer, "+PONG\r\n");
		send(client_fd, res_buffer, 7, 0);
	}

	close(client_fd);

	pthread_exit(NULL);
}

int main()
{
	// Disable output buffering
	setbuf(stdout, NULL);

	int server_fd, client_fd, client_addr_len;
	struct sockaddr_in client_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting REUSE_PORT
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0)
	{
		printf("SO_REUSEPORT failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(6379),
		.sin_addr = {htonl(INADDR_ANY)},
	};

	if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
	{
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0)
	{
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	int current_thread = 0;
	pthread_t t_ids[1024];

	// create a new thread for every client
	while (client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len))
	{
		printf("Client connected\n");
		pthread_create(&t_ids[current_thread++], NULL, handle_req, (void *)&client_fd);
	}

	close(server_fd);

	return 0;
}
