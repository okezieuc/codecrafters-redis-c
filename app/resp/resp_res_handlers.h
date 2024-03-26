#ifndef REDIS_RESP_RES_HANDLERS
#define REDIS_RESP_RES_HANDLERS

int send_simple_string(int client_fd, char *str);
int send_bulk_string(int client_fd, char *str);

#endif
