#include <errno.h>
#include "network.h"
#include "util.h"
#include "def.h"


int networkReceive(int fd, Message *buffer)
{
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
