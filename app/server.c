#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include "utils.h"

void *handle_req(void *);

void *handle_req(void *arg)
{
	pthread_detach(pthread_self());

	// store a copy of the client_fd in this context
	int len, client_fd = *(int *)arg;
	char req_buffer[1024], res_buffer[1024], *ptr;

	// receive messge from client
	// handle multiple requests from single client
	while (recv(client_fd, req_buffer, 1023, 0))
	{
		// parse the request

		// check the first character of the request to understand
		// the structure of the request

		// handle array requests
		if (*req_buffer == '*')
		{

			ptr = req_buffer + 1; // we use a copy of our pointer, as it gets modified
			int count = parse_number(&ptr, 1);
			char tmp_str[20]; // temporary string storage. 20 characters long.

			// ptr now points to the last character in the array item count
			// we assume we have at least one item in our array
			// point ptr at the first character after our terminator
			ptr += 3;

			// handle bulk-formatted commands
			if (*ptr == '$')
			{
				ptr = ptr + 1;
				int len = parse_number(&ptr, 1);
				ptr += 3;

				// convert the command to lowercase
				for (int i = 0; i < len; i++)
				{
					*(ptr + i) = tolower(*(ptr + i));
				}

				// handle ping commands
				if (strncmp(ptr, "ping", len) == 0)
				{
					printf("STATUS: Received PING\n");

					strcpy(res_buffer, "+PONG\r\n");
					send(client_fd, res_buffer, 7, 0);

					// we don't expect to have any other thing in an array after the
					// ping command. so we do nothing here

				} // handle echo commands
				else if (strncmp(ptr, "echo", len) == 0)
				{
					printf("STATUS: Received ECHO\n");

					// move our pointer to the start of the data to echo
					// which most likely would be a dollar sign
					ptr += len + 2;

					// handle bulk strings
					if (*ptr == '$')
					{
						ptr = (ptr + 1);
						int message_len = parse_number(&ptr, 1);
						ptr += 3;
						// ptr now points to the first character of our message
						strcpy(res_buffer, "$");

						// get the length of the returned character and append to response
						sprintf(tmp_str, "%d", message_len);
						strcat(res_buffer, tmp_str);

						strcat(res_buffer, "\r\n");
						strncat(res_buffer, ptr, message_len);
						strcat(res_buffer, "\r\n");
						send(client_fd, res_buffer, strlen(res_buffer), 0);
						printf("STATUS: Sent a response\n");
					}
				}
			}
		}
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
	while ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) != -1)
	{
		printf("Client connected\n");
		pthread_create(&t_ids[current_thread++], NULL, handle_req, (void *)&client_fd);
	}

	printf("server stopped");

	close(server_fd);

	return 0;
}
