#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <stdint.h>

void *clientthread(void *arg);
int sendLRE(int client_socket, uint8_t code, const char* serverName);

#endif
