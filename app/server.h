#ifndef REDIS_SERVER
#define REDIS_SERVER

struct ServerMetadata
{
    int is_replica;
    char replication_id[64];
    int replication_offset;
    char master_host[32];
    int master_port;
    int port;
};

#endif