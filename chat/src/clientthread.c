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
#include <time.h>
#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"
#include "def.h"
#include "broadcastagent.h"
#include "connectionhandler.h"

#include <time.h>
#include <netinet/in.h>

#define BUFFER_SIZE 2048 // Maximale Nachrichtengröße

static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;

//Message predefs
const char *serverName = "ChatServer";
const char *pauseMsg = "Broadcast paused (　 Д )　* *!";
const char *resumeMsg = "Broadcast resumed (ﾉ>ω<)ﾉ :｡･:*:･ﾟ’★,｡･:*:･ﾟ’☆";
const char *invalidCmdMsg = "Invalid command (ToT)";
const char *alreadyPausedMsg = "Broadcast already paused ( ´∀｀ )b";
const char *alreadyRunningMsg = "Broadcast already running ( ´∀｀ )b";
const char *noPermissionMsg = "You do not have permission to perform this action ( *-*)";
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
        if (sendLRE(client_socket, LRE_UNKNOWN_ERROR, "Server23(/>w<)/")) 
		{
            errorPrint("Failed to send LoginResponse to client %d with code %d", client_socket, LRE_UNKNOWN_ERROR);
        }
        close(client_socket);
        return EXIT_FAILURE;
    }

	// Validate magic number
	uint32_t received_magic = buffer->body.login_request.magic;
	debugPrint("Received magic: 0x%08x, Expected: 0x%08x", received_magic, MAGIC_LRQ);
    if (received_magic != MAGIC_LRQ) 
	{
        debugPrint("Invalid magic number in LoginRequest");
        if (sendLRE(client_socket, LRE_UNKNOWN_ERROR, "Server23(/>w<)/")) 
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
        if (sendLRE(client_socket, LRE_WRONG_VERSION, "Server23(/>w<)/")) 
		{
            errorPrint("Failed to send LoginResponse to client %d with code %d", client_socket, LRE_WRONG_VERSION);
        }
        close(client_socket);
        return EXIT_FAILURE;
    }

	// Extract and validate name
	// Calculate actual name length: total length minus fixed fields (magic + version)
	size_t fixed_fields_size = sizeof(uint32_t) + sizeof(uint8_t); // magic + version
	if (buffer->header.length < fixed_fields_size) {
		debugPrint("Login request too short to contain required fields");
		if (sendLRE(client_socket, LRE_UNKNOWN_ERROR, "Server23(/>w<)/")) {
			errorPrint("Failed to send LoginResponse to client %d with code %d", client_socket, LRE_UNKNOWN_ERROR);
		}
		close(client_socket);
		return EXIT_FAILURE;
	}
	
	size_t name_length = buffer->header.length - fixed_fields_size;
	debugPrint("Calculated name length: %zu (total: %u, fixed fields: %zu)", name_length, buffer->header.length, fixed_fields_size);
	
    if (name_length > NAME_MAX) 
	{
        debugPrint("Name length %zu exceeds maximum allowed size %d", name_length, NAME_MAX);
        if (sendLRE(client_socket, LRE_NAME_INVALD, "Server23(/>w<)/")) 
		{
            errorPrint("Failed to send LoginResponse to client %d with code %d", client_socket, LRE_NAME_INVALD);
        }
        close(client_socket);
        return EXIT_FAILURE;
    }

	char name[NAME_MAX + 1];
    memcpy(name, buffer->body.login_request.name, name_length);
    name[name_length] = '\0'; // Null terminate

    // Check name (ASCII)
    for (size_t i = 0; i < strlen(name); i++) {
        if (name[i] < 33 || name[i] > 126 || name[i] == '\'' || name[i] == '"' || name[i] == '`') {
            debugPrint("Invalid character in name: %s\n", name);
            if (sendLRE(client_socket, LRE_NAME_INVALD, "Server23(/>w<)/")) {
                errorPrint("Failed to send LoginResponse to client %d with code %d", client_socket, LRE_NAME_INVALD);
            }
            close(client_socket);
            return EXIT_FAILURE;
        }
    }

    // Check if username is already taken
    pthread_mutex_lock(&userLock);
    User* existing_user = searchUser(name);
    pthread_mutex_unlock(&userLock);
    
    if (existing_user != NULL) {
        debugPrint("Username '%s' is already taken by user on socket %d", name, existing_user->sock);
        if (sendLRE(client_socket, LRE_NAME_TAKEN, "Server23(/>w<)/")) {
            errorPrint("Failed to send LoginResponse to client %d with code %d", client_socket, LRE_NAME_TAKEN);
        }
        close(client_socket);
        return EXIT_FAILURE;
    }

    // All validation passed - send success response
    debugPrint("Login validation successful for user: %s", name);
    if (sendLRE(client_socket, LRE_SUCCESS, "Server23(/>w<)/")) {
        errorPrint("Failed to send success LoginResponse to client %d", client_socket);
        close(client_socket);
        return EXIT_FAILURE;
    }

    debugPrint("Success LoginResponse sent to client %d", client_socket);
	return EXIT_SUCCESS;

}

int sendLRE(int client_socket, uint8_t code, const char* serverName)
{
	Message response;
	memset(&response, 0, sizeof(Message));
	
	// Set header
	response.header.type = LRE;  // Login Response type
	response.header.length = sizeof(uint32_t) + sizeof(uint8_t) + strlen(serverName);
	
	// Set body
	response.body.login_response.magic = htonl(MAGIC_LRE);
	response.body.login_response.code = code;
	strncpy(response.body.login_response.serverName, serverName, NAME_MAX - 1);
	response.body.login_response.serverName[NAME_MAX - 1] = '\0';
	
	// Send the response
	if (networkSend(client_socket, &response) == -1) {
		errnoPrint("Failed to send login response");
		return 1; // Return 1 on failure
	}
	
	return 0; // Return 0 on success
}

int handleAdminCommand(int client_socket, const char* username, const char* command)
{
	debugPrint("Processing admin command from %s: %s", username, command);
	
	// Check if user is admin
	if (strcmp(username, "Admin") != 0) {
		debugPrint("User %s attempted admin command but is not admin", username);
		uint64_t timestamp = (uint64_t)time(NULL);
		sendServer2Client(client_socket, "Server", timestamp, noPermissionMsg);
		return 0; // Not an error, just not authorized
	}
	
	// Parse command
	if (strncmp(command, "/kick ", 6) == 0) {
		// Extract target username
		const char* target_name = command + 6; // Skip "/kick "
		
		debugPrint("Admin %s attempting to kick user: %s", username, target_name);
		
		// Find target user
		User* target_user = searchUser(target_name);
		if (target_user == NULL) {
			debugPrint("Kick target user %s not found", target_name);
			uint64_t timestamp = (uint64_t)time(NULL);
			sendServer2Client(client_socket, "Server", timestamp, userNotFoundMsg);
			return 0;
		}
		
		// Block Admin kick
		if (strcmp(target_name, "Admin") == 0) {
			debugPrint("Admin attempted to kick another admin");
			uint64_t timestamp = (uint64_t)time(NULL);
			sendServer2Client(client_socket, "Server", timestamp, noKickingAdminMsg);
			return 0;
		}
		
		// Send kick notification to target user
		uint64_t timestamp = (uint64_t)time(NULL);
		sendServer2Client(target_user->sock, "Server", timestamp, "You have been kicked from the server Σ(x_x;)!");
		
		// Sending user removed message
		sendUserRemoved(target_name, 1);

		// Close target user's connection ->>cleanup
		debugPrint("Kicking user %s (socket %d)", target_name, target_user->sock);
		close(target_user->sock);
		
		sendServer2Client(client_socket, "Server", timestamp, "User successfully kicked ( ´∀｀ )b");
		
		return 0;
		
	} else if (strcmp(command, "/pause") == 0) {
		debugPrint("Admin %s pausing broadcast", username);
		if (isBroadcastPaused()) {
			uint64_t timestamp = (uint64_t)time(NULL);
			sendServer2Client(client_socket, "Server", timestamp, alreadyPausedMsg);
			return 0;
		}
		pauseBroadcasting();
		uint64_t timestamp = (uint64_t)time(NULL);
		sendServer2Client(client_socket, "Server", timestamp, pauseMsg);
		return 0;
		
	} else if (strcmp(command, "/resume") == 0) {
		debugPrint("Admin %s resuming broadcast", username);
		if (!isBroadcastPaused()) {
			uint64_t timestamp = (uint64_t)time(NULL);
			sendServer2Client(client_socket, "Server", timestamp, alreadyRunningMsg);
			return 0;
		}
		resumeBroadcasting();
		uint64_t timestamp = (uint64_t)time(NULL);
		sendServer2Client(client_socket, "Server", timestamp, resumeMsg);
		return 0;
		
	} else {
		debugPrint("Unknown admin command: %s", command);
		uint64_t timestamp = (uint64_t)time(NULL);
		sendServer2Client(client_socket, "Server", timestamp, invalidCmdMsg);
		return 0;
	}
}

int sendUserAdded(char *username)
{
	debugPrint("sending message adding User");
	Message userAdded;
	memset (&userAdded, 0, sizeof(Message));

	//set header
	userAdded.header.type = UAD;  // Login Response type
	userAdded.header.length = sizeof(uint64_t) + strlen(username);

	// Get current timestamp
	uint64_t timestamp = (uint64_t)time(NULL);
	debugPrint("timestamp: %ld", timestamp);

	//set body
	userAdded.body.user_added.timestamp = htonll(timestamp);
	strncpy(userAdded.body.user_added.name, username, NAME_MAX - 1);
	userAdded.body.user_added.name[NAME_MAX - 1] = '\0';

	//broadcast this bitch
	//broadcastServer2client
	User *current = userFront;
    while(current)
    {
        //printUser(current);
		networkSend(current->sock, &userAdded);
        current = current->next;
    }

	return 0; //replace with actual return
}

int sendUserRemoved(const char *username, uint8_t code)
{
	debugPrint("sending message removing User");
	Message userRemoved;
	memset (&userRemoved, 0, sizeof(Message));

	//set header
	userRemoved.header.type = URM;  // Login Response type
	userRemoved.header.length = sizeof(uint64_t) + strlen(username) + sizeof(uint8_t);

	// Get current timestamp
	uint64_t timestamp = (uint64_t)time(NULL);

	//set body
	userRemoved.body.user_removed.timestamp = htonll(timestamp);
	strncpy(userRemoved.body.user_removed.name, username, NAME_MAX - 1);
	userRemoved.body.user_removed.code = code;
	userRemoved.body.user_removed.name[NAME_MAX - 1] = '\0';

	//broadcast this bitch
	//broadcastServer2client
	User *current = userFront;
    while(current)
    {
        //printUser(current);
		networkSend(current->sock, &userRemoved);
        current = current->next;
    }

	return 0; //replace with actual return
}

//building the Useradded Message for Login
void buildUserAddMessage(Message *message, const char *username)
{

	debugPrint("bueilding user added message");
	size_t name_length = strlen(username);
    if (name_length < 1 || name_length > MAX_NAME) {
        errnoPrint("prepareUserAddedMessage: Invalid client_name length: %zu", name_length);
        return;
    }

	memset (message, 0, sizeof(Message));

	//set header
	message->header.type = UAD;  // Login Response type
	message->header.length = sizeof(uint64_t) + strlen(username);

	//set body
	message->body.user_added.timestamp = htonll(0);
	memcpy(message->body.user_added.name, username, strlen(username));

}

void notify_new_user_callback(User *existing_user, void *context)
{
	debugPrint("callback");
    if (!existing_user || !context) {
        return;
    }

    // Cast context back to the new user's socket
    int *new_user_sock = (int *)context;

    if (existing_user->sock == *new_user_sock) {
        return; // existing user is new user
    }

    // Prepare
    Message message;
    buildUserAddMessage(&message, existing_user->name);

    // Send the message to the new user's socket
    if (networkSend(*new_user_sock, &message) == -1) {
        errorPrint("Failed to notify new user about existing user %s (fd=%d).", existing_user->name, *new_user_sock);
    } else {
        debugPrint("Notified new user about existing user %s (fd=%d).", existing_user->name, *new_user_sock);
    }
}

void sendUserList(int socket)
{
	debugPrint("UserList");

	forEachUser(notify_new_user_callback, &socket);

    /*User *current = userFront;
    while(current)
    {
		if(strcmp(current->name, username) == 0)
		{
			current = current->next;
		}

		debugPrint("UserList2");

		Message userAdded;
		memset (&userAdded, 0, sizeof(Message));

		debugPrint("UserList3");
		//set header
		userAdded.header.type = UAD;  // Login Response type
		userAdded.header.length = sizeof(uint64_t) + strlen(username);

		// Get current timestamp
		uint64_t timestamp = 0;

		//set body
		userAdded.body.user_added.timestamp = htonll(timestamp);
		strncpy(userAdded.body.user_added.name, current->name, NAME_MAX - 1);
		userAdded.body.user_added.name[NAME_MAX - 1] = '\0';

		debugPrint("UserList4");

		networkSend(socket, &userAdded);
        current = current->next;
    }*/


}




void *clientthread(void *arg)
{
    int client_socket = *(int *)arg;
    free(arg);

    debugPrint("Client thread started (ﾉ>ω<)ﾉ (socket: %d)", client_socket);

    Message buffer;
    memset(&buffer, 0, sizeof(Message));

    //receive initial login request
    if (networkReceive(client_socket, &buffer) == 0)
    {
        if (buffer.header.type == LRQ)
        {
            //handle login request
            if (handleLRQ(&buffer, client_socket) == 0)
            {
                debugPrint("Login successful");
                
                //Extract username from login request for user creation
                char username[NAME_MAX + 1];
                size_t name_length = buffer.header.length;
                if (name_length > NAME_MAX) {
                    name_length = NAME_MAX;
                }
                memcpy(username, buffer.body.login_request.name, name_length);
                username[name_length] = '\0';

                pthread_t current_thread = pthread_self();

                pthread_mutex_lock(&userLock);

                //Send full user list to new client (excluding itself)
                sendFullUserListToClient(client_socket);

                //Add new user to user list
                if (createUser(client_socket, current_thread, username) != 0) {
                    errorPrint("Failed to create user for client %d", client_socket);
                    close(client_socket);
                    return NULL;
                }

                //Broadcast new user to others (excluding itself)
                broadcastUserAddedToOthers(username);

                pthread_mutex_unlock(&userLock);

                debugPrint("User '%s' created and added to user list", username);


				// Enter the main message loop
				debugPrint("Client thread will continue listening for messages...");
				
				// Message loop - keep receiving messages from this client
				while (1) {
					memset(&buffer, 0, sizeof(Message));
					int result = networkReceive(client_socket, &buffer);
					
					if (result == 0) {
						// Handle different message types
						switch (buffer.header.type) {
							case C2S:
								// Handle client-to-server chat message
								debugPrint("Received C2S message from client %d", client_socket);
								
								// Extract message text from the C2S message
								char message_text[MSG_MAX + 1];
								size_t text_length = buffer.header.length;
								if (text_length > MSG_MAX) {
									text_length = MSG_MAX;
								}
								
								memcpy(message_text, buffer.body.client_to_server.text, text_length);
								message_text[text_length] = '\0';
								
								// Check if this is an admin command
								if (message_text[0] == '/') {
									if (handleAdminCommand(client_socket, username, message_text) != 0) {
										debugPrint("Failed to handle admin command from %s: %s", username, message_text);
									}
								} else {
									// Regular chat message - broadcast it
									uint64_t timestamp = (uint64_t)time(NULL);
									
									debugPrint("Broadcasting message from %s: %s", username, message_text);
									
									//Use timeout to avoid blocking when queue is full
									int broadcast_result = broadcastMessage(username, message_text, timestamp);
									if (broadcast_result == 2) {
										debugPrint("Queue full while paused, sending error to client %d", client_socket);
										sendServer2Client(client_socket, "Server", timestamp, fullQueueMsg);
									} else if (broadcast_result != 0) {
										debugPrint("Failed to broadcast message from client %d (queue timeout or full)", client_socket);
										// Send error message back to client
										sendServer2Client(client_socket, "Server", timestamp, "Message failed to send - server busy or paused");
									} else {
										debugPrint("Message successfully queued for broadcast");
									}
								}
								break;
							case UAD:
								// Handle user admin command
								debugPrint("Received UAD command from client %d", client_socket);
								// TODO: Handle admin commands
								break;
							default:
								debugPrint("Received unknown message type %d from client %d", buffer.header.type, client_socket);
								break;
						}
					} else {
						// Connection lost or error
						debugPrint("Client %d disconnected or error occurred", client_socket);
						break;
					}
				}
				
				// Clean up: remove user from list when connection ends
				sendUserRemoved(username, CONN_CLOSED_CLIENT);
				debugPrint("Removing user '%s' from user list", username);
				removeUser(current_thread);
				

				//TODO send remove user message



			} else {
				debugPrint("Login failed for client %d", client_socket);
			}
		} else {
			debugPrint("Expected LRQ but received type %d from client %d", buffer.header.type, client_socket);
		}
	} else {
		debugPrint("Failed to receive initial message from client %d", client_socket);
	}

	close(client_socket);//maybe different var
	debugPrint("Client thread stopping.");

    pthread_mutex_lock(&connection_count_mutex);
    active_connections--;
    pthread_mutex_unlock(&connection_count_mutex);

    debugPrint("Client thread stopping - connection count: %d", active_connections);
	return NULL;
}

void sendFullUserListToClient(int client_socket) {
    User *current = userFront;
    while (current) {
        sendUserAddedMessage(client_socket, current->name);
        current = current->next;
    }
}

void broadcastUserAddedToOthers(const char *username) {
    User *current = userFront;
    while (current) {
        sendUserAddedMessage(current->sock, username);
        current = current->next;
    }
}

void sendUserAddedMessage(int client_socket, const char *username) {
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.header.type = UAD;
    msg.header.length = sizeof(uint64_t) + strlen(username);

    // Set body
    msg.body.user_added.timestamp = htonll((uint64_t)time(NULL));
    strncpy(msg.body.user_added.name, username, NAME_MAX - 1);
    msg.body.user_added.name[NAME_MAX - 1] = '\0';

    networkSend(client_socket, &msg);
}

