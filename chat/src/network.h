#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include <stdint.h>
#include "def.h"
#include "user.h"

#define MAX_NAME 31 // name and sname
#define MAX_TEXT 512
#define MAX_MESSAGE 555
#define TIMESTAMP_LEN 8 // 64 bit timestamp


/* TODO: When implementing the fully-featured network protocol (including
 * login), replace this with message structures derived from the network
 * protocol (RFC) as found in the moodle course. */

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
int broadcastServer2Client(const char *orig_sender,const char *text, uint64_t timestamp);
int sendServer2Client(int receiver_client, const char *original_sender, uint64_t timestamp, const char *text);
void prepareServer2ClientMessage(Message *msg, const char *original_sender, uint64_t timestamp, const char *text);
void broadcast_server2client_callback(User *user, void *context);

#endif
