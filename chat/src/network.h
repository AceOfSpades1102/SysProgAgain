#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include <stdint.h>

/* TODO: When implementing the fully-featured network protocol (including
 * login), replace this with message structures derived from the network
 * protocol (RFC) as found in the moodle course. */
enum { 
	MSG_MAX = 1024,
	NAME_MAX = 32,
	TEXT_MAX = 512,
	TYPE_MAX = 5,
	LRG_LEN_MIN = 6,
	LRG_LEN_MAX = 36,
	
	MAGIC_LRQ = 0x0badf00d,
	MAGIC_LRE = 0xc001c001,
	SERVER_VERSION = 0,
	
	LRQ = 0,
	LRE = 1,
	C2S = 2,
	S2C = 3,
	UAD = 4,
	URM = 5,


	//Login Responses
	LRE_SUCCESS = 0,
	LRE_NAME_TAKEN = 1,
	LRE_NAME_INVALD = 2,
	LRE_WRONG_VERSION = 3,
	LRE_UNKNOWN_ERROR = 255,

	VALID_TYPE = 1,

	// User removed
	CONN_CLOSED_CLIENT = 0,
	KICKED_FROM_SERVER = 1,
	COMMUNICATION_ERROR = 2,

	};

typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
} messageHeader;

typedef struct __attribute__((packed)){
	uint32_t magic;
	uint8_t version;
	char name[NAME_MAX];
}loginRequestBody; // 0

typedef struct __attribute__((packed)){
	uint32_t magic;
	uint8_t code;
	char serverName[NAME_MAX];
}loginResponseBody; // 1

typedef struct __attribute__((packed)){
	char text[MSG_MAX];
} clientToServerBody; // 2

typedef struct __attribute__((packed)){
	uint64_t timestamp;
	char originalSender[NAME_MAX];
	char text[MSG_MAX];
} serverToClientBody; // 3

typedef struct __attribute__((packed)){
	uint64_t timestamp;
	char name[NAME_MAX];
} userAddedBody; // 4

typedef struct __attribute__((packed)){
	uint64_t timestamp;
	uint8_t code;
	char name[NAME_MAX];
} userRemovedBody; // 5	

typedef union {
    loginRequestBody login_request;
    loginResponseBody login_response;
	serverToClientBody server_to_client;
    clientToServerBody client_to_server;
    userAddedBody user_added;
    userRemovedBody user_removed;
} messageBody;

typedef struct __attribute__((packed)){
	messageHeader header;
	messageBody body;
} Message;


int networkReceive(int fd, Message *buffer);
int networkSend(int fd, const Message *buffer);

#endif
