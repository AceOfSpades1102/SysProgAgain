#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"

#include <time.h>
#include <netinet/in.h>

#define BUFFER_SIZE 2048 // Maximale Nachrichtengröße

//Message predefs
const char *serverName = "ChatServer";
const char *pauseMsg = "Broadcast paused (　ﾟﾛﾟ)!!";
const char *resumeMsg = "Broadcast resumed (ﾉ>ω<)ﾉ :｡･:*:･ﾟ’★,｡･:*:･ﾟ’☆";
const char *invalidCmdMsg = "Invalid command (´･ｪ･｀)";
const char *alreadyPausedMsg = "Broadcast already paused ( ´∀｀ )b";
const char *alreadyRunningMsg = "Broadcast already running ( ´∀｀ )b";
const char *noPermissionMsg = "You do not have permission to perform this action ( ・－・)";
const char *emptyString = "";
const char *userNotFoundMsg = "User not found (  -_・)?";
const char *noKickingAdminMsg = "You cannot kick an admin ！Σ(x_x;)!";
const char *fullQueueMsg = "Message queue is full (´･ｪ･｀)";




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
