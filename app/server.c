#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include "utils.h"
#include "resp/resp.h"
#include "dict.h"
#include "replication/handshake.h"
#include "server.h"

struct ConnArgs
{
	struct ServerMetadata server_meta_data;
	int client_fd;
	struct Dict *store;
};

struct RedisEntry
{
	char value[512];
	long int expiry_time;
};

void *handle_req(void *);

void *handle_req(void *arg)
{
	pthread_detach(pthread_self());
	struct ConnArgs *req_conn_args = arg;

	// store a copy of the client_fd in this context
	int len, client_fd = req_conn_args->client_fd;
	struct Dict *store = req_conn_args->store;
	struct ServerMetadata server_meta_data = req_conn_args->server_meta_data;

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

			struct RedisEntry new_entry;
			strcpy(new_entry.value, ((struct RESPBulkStringNode *)resp_request->item_ptrs[2])->data);
			new_entry.expiry_time = 0;

			// if the received command contains up to four items, we should expect to have an expiry
			if (resp_request->length >= 4)
			{
				// check if the third command argument is px
				struct RESPBulkStringNode *arg3 = (struct RESPBulkStringNode *)resp_request->item_ptrs[3];

				for (int i = 0; i < arg3->length; i++)
				{
					*(arg3->data + i) = tolower(*(arg3->data + i));
				}

				if (strcmp(arg3->data, "px") == 0)
				{
					struct RESPBulkStringNode *arg4 = (struct RESPBulkStringNode *)resp_request->item_ptrs[4];
					int ttl = atoi(arg4->data);
					new_entry.expiry_time = get_current_time() + ttl;
				}
			}

			set_dict_item(store, ((struct RESPBulkStringNode *)resp_request->item_ptrs[1])->data, &new_entry);

			struct RESPSimpleStringNode *res_data = create_resp_simple_string_node("OK");
			char *res_body = encode_resp_simple_string(res_data);
			send(client_fd, res_body, strlen(res_body), 0);

			free(res_data);
			free(res_body);
		}

		else if (strcmp(command->data, "get") == 0)
		{
			printf("STATUS: Received GET\n");

			struct RedisEntry *saved_entry = get_dict_item(store, ((struct RESPBulkStringNode *)resp_request->item_ptrs[1])->data);
			char *res_body;
			long int now = get_current_time();

			// check if the entry is expired
			if ((saved_entry->expiry_time != 0) && ((now - saved_entry->expiry_time) > 0))
			{
				// the key is expired
				del_dict_item(store, ((struct RESPBulkStringNode *)resp_request->item_ptrs[1])->data);
				res_body = encode_resp_bulk_string(NULL);
			}
			else
			{
				struct RESPBulkStringNode *res_data = create_resp_bulk_string_node(saved_entry->value);
				res_body = encode_resp_bulk_string(res_data);
				free(res_data);
			}

			send(client_fd, res_body, strlen(res_body), 0);
			free(res_body);
		}

		else if (strcmp(command->data, "info") == 0)
		{
			struct RESPBulkStringNode *arg1 = (struct RESPBulkStringNode *)resp_request->item_ptrs[1];

			if (strcmp(arg1->data, "replication") == 0)
			{
				struct RESPBulkStringNode *res_node;

				if (server_meta_data.is_replica == 0)
				{
					char server_info[256];
					strcpy(server_info, "role:master\n");
					strcat(server_info, "master_replid:");
					strcat(server_info, server_meta_data.replication_id);
					strcat(server_info, "\nmaster_repl_offset:");
					sprintf(server_info + strlen(server_info), "%d", server_meta_data.replication_offset);

					res_node = create_resp_bulk_string_node(server_info);
				}
				else
				{
					res_node = create_resp_bulk_string_node("role:slave");
				}

				char *res_body = encode_resp_bulk_string(res_node);

				send(client_fd, res_body, strlen(res_body), 0);

				free(res_node);
				free(res_body);
			}
		}

		else if (strcmp(command->data, "replconf") == 0)
		{
			struct RESPSimpleStringNode *res_node;
			struct RESPBulkStringNode *arg1 = (struct RESPBulkStringNode *)resp_request->item_ptrs[1];

			if (strcmp(arg1->data, "listening-port") == 0)
			{
				struct RESPBulkStringNode *arg2 = (struct RESPBulkStringNode *)resp_request->item_ptrs[2];

				printf("Replica listens on port %s\n", arg2->data);
				res_node = create_resp_simple_string_node("OK");
			}
			else if (strcmp(arg1->data, "capa") == 0)
			{
				struct RESPBulkStringNode *arg2 = (struct RESPBulkStringNode *)resp_request->item_ptrs[2];

				printf("Replica has capability %s\n", arg2->data);
				res_node = create_resp_simple_string_node("OK");
			}

			char *res_body = encode_resp_simple_string(res_node);
			send(client_fd, res_body, strlen(res_body), 0);

			free(res_node);
			free(res_body);
		}

		free_resp_array_node(resp_request);
	}

	close(client_fd);

	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	// Disable output buffering
	setbuf(stdout, NULL);

	int server_fd, client_fd, client_addr_len;
	struct sockaddr_in client_addr;

	struct ServerMetadata server_meta_data;
	server_meta_data.is_replica = 0;
	server_meta_data.port = 6379;

	// handle command line arguments
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "--port") == 0)
		{
			server_meta_data.port = atoi(argv[i + 1]);
			i++;
		}
		else if (strcmp(argv[i], "--replicaof") == 0)
		{
			server_meta_data.is_replica = 1;

			strcpy(server_meta_data.master_host, argv[i + 1]);
			server_meta_data.master_port = atoi(argv[i + 2]);

			i += 2;
		}
	}

	// set replication ID and offset if server is a master node
	if (server_meta_data.is_replica == 0)
	{
		strcpy(server_meta_data.replication_id, "8371b4fb1155b71f4a04d3e1bc3e18c4a990aeeb");
		server_meta_data.replication_offset = 0;
	}

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting REUSE_PORT
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(server_meta_data.port),
		.sin_addr = {htonl(INADDR_ANY)},
	};

	if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
	{
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	// replica handshake to master. this only runs in replicas
	if (server_meta_data.is_replica)
	{
		send_handshake(server_meta_data);
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0)
	{
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	// create data store
	struct Dict *store = create_dict(sizeof(struct RedisEntry));

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	int current_thread = 0;
	pthread_t t_ids[1024];

	// create a new thread for every client
	while ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) != -1)
	{
		printf("Client connected\n");
		struct ConnArgs args = {server_meta_data, client_fd, store};
		pthread_create(&t_ids[current_thread++], NULL, handle_req, (void *)&args);
	}

	printf("server stopped");

	close(server_fd);

	return 0;
}
