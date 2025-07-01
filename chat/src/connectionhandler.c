#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include "user.h"
#include "clientthread.h"
#include "util.h"
#include "def.h"
#include "connectionhandler.h"

#define MAX_CLIENTS 100
#define THREAD_POOL_SIZE 20

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
} client_work_t;

int active_connections = 0;
pthread_mutex_t connection_count_mutex = PTHREAD_MUTEX_INITIALIZER;

static int createPassiveSocket(in_port_t port)
{
	int fd = -1;
	//TODONE: socket()
	fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) 
	{
        debugPrint("could not get socket");
        return -1;
    }

	// Set SO_REUSEADDR to allow immediate reuse of address
	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
	{
		errnoPrint("failed to set SO_REUSEADDR");
		close(fd);
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;		//only return IPv4 addresses
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	//TODONE: bind() to port
	if (bind(fd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
	{
		errnoPrint("failed to bind socket");
		close(fd);
		return -1;
	}

	//TODONE: listen()
	if (listen(fd, SOMAXCONN) == -1) 
	{
        errnoPrint("failed to listen");
        return -1;
    }

	// Set TCP_NODELAY to disable Nagle's algorithm
	int nodelay = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

	// Set SO_KEEPALIVE to enable keep-alive messages
	int keepalive = 1;
	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));

	//errno = ENOSYS;
	return fd;
}

int connectionHandler(in_port_t port)
{
	const int fd = createPassiveSocket(port);
	if(fd == -1)
	{
		errnoPrint("Unable to create server socket");
		return -1;
	}

	infoPrint("Server running on port %d...\n", port);

	for(;;)
	{
		struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        pthread_mutex_lock(&connection_count_mutex);
        if (active_connections >= MAX_CLIENTS) {
            pthread_mutex_unlock(&connection_count_mutex);
            debugPrint("Maximum client limit reached, rejecting new connection");
            
            int temp_sock = accept(fd, (struct sockaddr *)&client_addr, &client_len);
            if (temp_sock != -1) {
                close(temp_sock); // Immediately close if at limit
            }
            usleep(10000); // Brief delay before checking again
            continue;
        }
        active_connections++;
        pthread_mutex_unlock(&connection_count_mutex);

		int *client_sock = malloc(sizeof(int));
        if (!client_sock) {
            pthread_mutex_lock(&connection_count_mutex);
            active_connections--;
            pthread_mutex_unlock(&connection_count_mutex);
            continue;
        }

		debugPrint("Listening socket descriptor: %d", fd);

		*client_sock = accept(fd, (struct sockaddr *)&client_addr, &client_len);        if (*client_sock == -1)
        {
            pthread_mutex_lock(&connection_count_mutex);
            active_connections--;
            pthread_mutex_unlock(&connection_count_mutex);
            free(client_sock);
            continue;
        }

		infoPrint("New connection from fd:%d \n", *client_sock);

		//TODONE: add connection to user list and start client thread
		pthread_t tid;

		//TODONE: create User in clientthread
		if (pthread_create(&tid, NULL, clientthread, client_sock) != 0) {
            errnoPrint("Failed to create client thread");
            close(*client_sock);
            free(client_sock);
            continue;
        }

		pthread_detach(tid);
	}

	close(fd);
	return 0;	//never reached
}
