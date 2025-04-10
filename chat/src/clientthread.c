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
#include "def.h"

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

int handleLRQ(Message *buffer, int client_socket)
{
	debugPrint("handling LRQ");

	// Validate length
    if (buffer->header.length < 6 || buffer->header.length > 36) 
	{
        errnoPrint("Invalid message length received (  -_・)? %u", buffer->header.length);
        if (sendLoginResponse(client_socket, LRE_UNKNOWN_ERROR, "09Server")) 
		{
            errorPrint("Failed to send LoginResponse to client %d with code %d", client_socket, LRE_UNKNOWN_ERROR);
        }
        close(client_socket);
        return EXIT_FAILURE;
    }

	// Validate magic number
    if (ntohl(buffer->body.login_request.magic) != MAGIC_LRQ) 
	{
        debugPrint("Invalid magic number in LoginRequest");
        if (sendLoginResponse(client_socket, LRE_UNKNOWN_ERROR, "09Server")) 
		{
            errorPrint("Failed to send LoginResponse to client %d with code %d", client_socket, LRE_UNKNOWN_ERROR);
        }
        close(client_socket);
        return EXIT_FAILURE;
    }

	// Validate version
    if (buffer->body.login_request.version != LRQ_VERSION) 
	{
        debugPrint("Invalid version in LoginRequest");
        if (sendLoginResponse(client_socket, LRE_WRONG_VERSION, "09Server")) 
		{
            errorPrint("Failed to send LoginResponse to client %d with code %d", client_socket, LRE_WRONG_VERSION);
        }
        close(client_socket);
        return EXIT_FAILURE;
    }

	// Extract and validate name
    size_t name_length = buffer->header.length; // Subtract fixed fields
    if (name_length > NAME_MAX) 
	{
        debugPrint("Name length exceeds maximum allowed size");
        if (sendLoginResponse(client_socket, LRE_NAME_INVALD, "09Server")) 
		{
            errorPrint("Failed to send LoginResponse to client %d with code %d", client_socket, LRE_NAME_INVALD);
        }
        close(client_socket);
        return EXIT_FAILURE;
    }

	char name[NAME_MAX + 1];
    memcpy(name, buffer->body.login_request.name, name_length);
    name[name_length] = '\0'; // Null terminate

    // Chec5k name (ASCII)
    for (size_t i = 0; i < strlen(name); i++) {
        if (name[i] < 33 || name[i] > 126 || name[i] == '\'' || name[i] == '"' || name[i] == '`') {
            debugPrint("Invalid character in name: %s\n", name);
            if (sendLoginResponse(client_socket, LRE_NAME_INVALD, "09Server")) {
                errorPrint("Failed to send LoginResponse to client %d with code %d", client_socket, LRE_NAME_INVALD);
            }
            close(client_socket);
            return EXIT_FAILURE;
        }
    }

	return EXIT_SUCCESS;

}

int sendLRE()
{
	//send stuff
	//if sending stuff didn't work return fail
	//else return success
	return 1;

}


void *clientthread(void *arg)
{
	int client_socket = *(int *)arg;
	free(arg);

	debugPrint("Client thread started (ﾉ>ω<)ﾉ");

	//TODO: everything here lolz

	Message buffer;
    memset(&buffer, 0, sizeof(Message));

	//recieve shit
	if (networkReceive(client_socket, &buffer) == 0)
	{
		if (buffer.header.type == LRQ)
		{
			//handle login request
			if (handleLRQ(&buffer, client_socket) == 0)
			{
				debugPrint("Login successful");

			}
			//create user (in lrq?)
		}
	}
    
	debugPrint("Client thread started.");

	//TODO: Receive messages and send them to all users, skip self
	//TODO: literally everything

	close(clientthread);//maybe different var
	debugPrint("Client thread stopping.");
	return NULL;
}
