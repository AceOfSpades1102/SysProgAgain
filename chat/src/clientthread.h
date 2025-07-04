#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <stdint.h>

void *clientthread(void *arg);
int sendLRE(int client_socket, uint8_t code, const char* serverName);
void sendUserRemoved(const char *username, uint8_t code);
void sendUserAddedMessage(int client_socket, const char *username, uint64_t timestamp);
void sendFullUserListToClient(int client_socket);
void broadcastUserAddedToOthers(const char *username);

#endif
