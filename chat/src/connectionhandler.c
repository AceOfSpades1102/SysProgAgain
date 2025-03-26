#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "user.h"
#include "clientthread.h"
#include "util.h"
#include "def.h"
#include "connectionhandler.h"

static int createPassiveSocket(in_port_t port)
{
	int fd = -1;
	//TODO: socket()
	fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) 
	{
        debugPrint("could not get socket");
        return -1;
    }

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;		//only return IPv4 addresses
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//TODO: bind() to port
	if (bind(fd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
	{
		errnoPrint("failed to bind socket");
		close(fd);
		return -1;
	}

	//TODO: listen()
	if (listen(fd, 7) == -1) 
	{
        errnoPrint("failed to listen");
        return -1;
    }

	errno = ENOSYS;
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

	for(;;)
	{
		//TODO: accept() incoming connection
		//TODO: add connection to user list and start client thread
	}

	return 0;	//never reached
}
