#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <netinet/in.h>
#include <pthread.h>

int connectionHandler(in_port_t port);

extern int active_connections;
extern pthread_mutex_t connection_count_mutex;
void sendFullUserListToClient(int client_socket);

#endif
