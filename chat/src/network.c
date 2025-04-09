#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h> // For htons, htonl, ntohs, ntohl
#include <stdlib.h>
#include <time.h>
#include "network.h"
#include "util.h"
#include "def.h"

int recieveType(int fd, Message *buffer)
{
    ssize_t received = recv(fd, &buffer->header.type, sizeof(buffer->header.type), MSG_WAITALL);
    debugPrint("networkReceive: Raw received type: %zd", received);
    if (received != sizeof(buffer->header.type)) {
        debugPrint("networkReceive: Failed to receive message type. Received: %zd", received);
        return EXIT_FAILURE;
    }

    // Validate the message type
    if (buffer->header.type > TYPE_MAX) {
        debugPrint("networkReceive: Invalid message type received: %d", buffer->header.type);
        return INVALID_TYPE;
    }

    switch (buffer->header.type)
    {
    case 0:
        return LRQ;
        break;
    case 2:
        return C2S;
        break;
    }

    debugPrint("If this message is displayed something went wrong with the type. In other words: If you can see this, you done goofed(*｀Д´)ノ！！！");
    return EXIT_FAILURE;

}

int recieveLen(int fd, Message *buffer)
{
    // Receive length (2 bytes, big endian)
    uint16_t net_length;
    ssize_t received = recv(fd, &net_length, sizeof(net_length), MSG_WAITALL);
    if (received != sizeof(net_length)) {
        debugPrint("networkReceive: Failed to receive message length. Received: %zd", received);
        return EXIT_FAILURE;
    }

    //convert length byte order
    buffer->header.length = ntohs(net_length);
    debugPrint("networkReceive: Received length: %u", buffer->header.length);

    //validate length
    if (buffer->header.length + 3 < 4 || buffer->header.length + 3 > MSG_MAX) {
        debugPrint("networkReceive: Invalid message length: %u (%u with type and length)", buffer->header.length, buffer->header.length + 3);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int recieveHeader(int fd, Message *buffer)
{
    int type = recieveType(fd, buffer);
    int length = recieveLen(fd, buffer);

    return type;
}

int recieveMessage(int fd, Message *buffer)
{
    ssize_t body_length = buffer->header.length;
    ssize_t received = recv(fd, &buffer->body, body_length, MSG_WAITALL);
    if (received != body_length) {
        debugPrint("networkReceive: Failed to receive full message. Received: %zd", received);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}

int networkReceive(int fd, Message *buffer)
{
    
    debugPrint("networkRecieve reached");
    //TODO: Recieve type
    if (recieveType(fd, buffer) == INVALID_TYPE||EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }

    //TODO: Receive length and like do shit yk
    if (recieveLen(fd, buffer) == INVALID_TYPE||EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
	

	//TODO: Receive text
    if (recieveMessage(fd, buffer) == INVALID_TYPE||EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
	
	return EXIT_SUCCESS;
}

int networkSend(int fd, const Message *buffer)
{
	//TODO: Send complete message

	errno = ENOSYS;
	return -1;
}
