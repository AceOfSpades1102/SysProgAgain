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

    // Convert length byte order
    buffer->header.length = ntohs(net_length);
    debugPrint("networkReceive: Received length: %u", buffer->header.length);

    // Validate length
    if (buffer->header.length + 3 < 4 || buffer->header.length + 3 > MSG_MAX) {
        debugPrint("networkReceive: Invalid message length: %u (%u with type and length)", buffer->header.length, buffer->header.length + 3);
        return EXIT_FAILURE;
    }
}

int recieveHeader(int fd, Message *buffer)
{
    int type = recieveType(fd, buffer);
    int length = recieveLen(fd, buffer);

    return type;
}

int networkReceive(int fd, Message *buffer)
{
    
    debugPrint("networkRecieve reached");
    //TODO: Recieve type
    if (recieveType() == INVALID_TYPE)
    {

    }
    recieveHeader(fd, buffer);
	//TODO: Receive length
	//TODO: Convert length byte order
	//TODO: Validate length
	//TODO: Receive text

	//TODO: ^that but header and message separately

	// Step 1: Receive the header

	//maybe separate type and length but not desperately
	//like it works like this

    messageHeader header;
    ssize_t header_size = recv(fd, &header, sizeof(header), MSG_WAITALL);

    if (header_size != sizeof(header)) {
        perror("Failed to receive header");
        return NULL;
    }

    // Debug: Print raw header bytes
    debugPrint("Raw Header Bytes: ");
    unsigned char *header_bytes = (unsigned char *)&header;
    for (int i = 0; i < sizeof(header); i++) {
        debugPrint("%02x ", header_bytes[i]);
    }

    // Step 2: Parse the header
    header.length = ntohs(header.length); // Convert length from network byte order

    // Debug: Print parsed header fields
    printf("Parsed Header:\n");
    printf("  Type: %d\n", header.type);
    printf("  Length (payload): %d bytes\n", header.length);

    //check if header between 5 and 37



	

	errno = ENOSYS;
	return -1;
}

int networkSend(int fd, const Message *buffer)
{
	//TODO: Send complete message

	errno = ENOSYS;
	return -1;
}
