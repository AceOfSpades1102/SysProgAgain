#ifndef BROADCASTAGENT_H
#define BROADCASTAGENT_H

#include "network.h" 

#define BROADCAST_QUEUE_NAME "/broadcast_queue"
#define BROADCASTING_DISABLED 0
#define BROADCASTING_ENABLED 1

typedef struct {
    char sender[MAX_NAME + 7];
    char text[MAX_TEXT];
    uint64_t timestamp;
} BroadcastMessage;

int broadcastAgentInit(void);
void broadcastAgentCleanup(void);
int broadcastMessage(const char *sender, const char *text, uint64_t timestamp);
void pauseBroadcasting();
void resumeBroadcasting();

#endif
