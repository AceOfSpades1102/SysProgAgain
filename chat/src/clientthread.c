#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"

#define BUFFER_SIZE 2048 // Maximale Nachrichtengröße

void *clientthread(void *arg)
{
	User *self = (User *)arg;
	free(arg);

	int client_fd = self->sock;        
    pthread_t client_thread = self->thread;   
    
	Message buffer;
    memset(&buffer, 0, sizeof(Message));

	debugPrint("Client thread started.");

	//TODO: Receive messages and send them to all users, skip self
	//TODO: literally everything

	close(client_thread);
	debugPrint("Client thread stopping.");
	return NULL;
}
