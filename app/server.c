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
#include "resp/resp.h"
#include "dict.h"

struct ConnArgs
{
	int client_fd;
	struct Dict *store;
};

void *handle_req(void *);

void *handle_req(void *arg)
{
	pthread_detach(pthread_self());

	// store a copy of the client_fd in this context
	int len, client_fd = (int)(((struct ConnArgs *)arg)->client_fd);
	struct Dict *store = (struct Dict *)(((struct ConnArgs *)arg)->store);
	char req_buffer[1024], res_buffer[1024], *ptr;

	// receive messge from client
	// handle multiple requests from single client
	while (recv(client_fd, req_buffer, 1023, 0))
	{
		// parse the request
		ptr = (char *)&req_buffer;

		struct RESPArrayNode *resp_request = parse_resp_array(&ptr);

		// assuming all requests are sent as RESP arrays
		// and assuming the command is sent as a bulk string
		struct RESPBulkStringNode *command = (struct RESPBulkStringNode *)resp_request->item_ptrs[0];

		// convert the command to lowercase
		for (int i = 0; i < command->length; i++)
		{
			*(command->data + i) = tolower(*(command->data + i));
		}

		// handle ping commands
		if (strcmp(command->data, "ping") == 0)
		{
			printf("STATUS: Received PING\n");

			struct RESPSimpleStringNode *res_data = create_resp_simple_string_node("PONG");
			char *res_body = encode_resp_node((struct RESPNode *)res_data);
			send(client_fd, res_body, strlen(res_body), 0);

			free_resp_node((struct RESPNode *)res_data);
			free(res_body);
		}

		else if (strcmp(command->data, "echo") == 0)
		{
			printf("STATUS: Received PING\n");

			struct RESPBulkStringNode *res_data = create_resp_bulk_string_node(((struct RESPBulkStringNode *)resp_request->item_ptrs[1])->data);
			char *res_body = encode_resp_node((struct RESPNode *)res_data);
			send(client_fd, res_body, strlen(res_body), 0);

			free_resp_node((struct RESPNode *)res_data);
			free(res_body);
		}

		else if (strcmp(command->data, "set") == 0)
		{
			printf("STATUS: Received SET\n");
			set_dict_item(store, ((struct RESPBulkStringNode *)resp_request->item_ptrs[1])->data, ((struct RESPBulkStringNode *)resp_request->item_ptrs[2])->data);

			struct RESPSimpleStringNode *res_data = create_resp_simple_string_node("OK");
			char *res_body = encode_resp_simple_string(res_data);
			send(client_fd, res_body, strlen(res_body), 0);

			free(res_data);
			free(res_body);
		}

		free_resp_array_node(resp_request);
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

	// create data store
	struct Dict *store = create_dict();

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	int current_thread = 0;
	pthread_t t_ids[1024];

	// create a new thread for every client
	while ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) != -1)
	{
		printf("Client connected\n");
		struct ConnArgs args = {client_fd, store};
		pthread_create(&t_ids[current_thread++], NULL, handle_req, (void *)&args);
	}

	printf("server stopped");

	close(server_fd);

	return 0;
}
