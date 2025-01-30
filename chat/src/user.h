#ifndef USER_H
#define USER_H

#include <pthread.h>

typedef struct User
{
	struct User *prev;
	struct User *next;
	pthread_t thread;	//thread ID of the client thread
	int sock;		//socket for client
	char name[MAX_NAME_LEN +1 ];
} User;


void removeUser(pthread_t thread_id);
User* createUser(int sock, pthread_t thread);
void forEachUser(void (*callback)(User *));
void userRemoveAll(void);
void printUser(User *user);

User* searchUser(const char *name);

extern User *userFront;

//TODO: Add prototypes for functions that fulfill the following tasks:
// * Add a new user to the list and start client thread
// * Iterate over the complete list (to send messages to all users)
// * Remove a user from the list
//CAUTION: You will need proper locking!

#endif
