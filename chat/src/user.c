#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "user.h"
#include "clientthread.h"
#include "util.h"
#include "def.h"

static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
User *userFront = NULL;
static User *userBack = NULL;

//errors
const char *allocError =("Something went wrong with allocating Memory (・_・)?");

//TODO: Implement the functions declared in user.h

int createUser(int sock, pthread_t thread,const char *name)
{
    pthread_mutex_lock(&userLock);
    User *newUser = (User *)malloc(sizeof(User));
    if (!newUser) {
        debugPrint("%s", allocError);
        pthread_mutex_unlock(&userLock);
        return EXIT_FAILURE;
    }

    strncpy(newUser->name, name, NAME_MAX);

    newUser->name[NAME_MAX] = '\0';
    newUser->sock = sock;
    newUser->thread = thread;
    newUser->prev = NULL;
    newUser->next = NULL;


    if (!userFront) {
        userFront = newUser;
        userBack = newUser;
    } else {
        userBack->next = newUser;
        newUser->prev = userBack;
        userBack = newUser;
    }

    debugPrint("userFront is now: %p\n", (void *)userFront);
    
    pthread_mutex_unlock(&userLock);
    //printUser(newUser);
    return EXIT_SUCCESS;
}


void removeUser(pthread_t thread_id)
{
    //assert(User != NULL);

    pthread_mutex_lock(&userLock);

    User *current = userFront;
    while (current) {
        if (pthread_equal(current->thread, thread_id)) {
            // Found the user to remove

            // Update the pointers of the surrounding nodes
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                userFront = current->next; // Removing the head node
            }

            if (current->next) {
                current->next->prev = current->prev;
            } else {
                userBack = current->prev;
            }


            //close sock
            close(current->sock);
            // Free the memory of the removed user
            free(current);

            // Unlock the list mutex and return
            pthread_mutex_unlock(&userLock);
            return; // TODO : Success
        }
        current = current->next;
    }

    //printf("userFront is now: %p\n", (void *)userFront);

    pthread_mutex_unlock(&userLock);
    return; //TODO Failure
}


void forEachUser(void (*callback)(User *, void *), void *context)
{
    pthread_mutex_lock(&userLock);

    User *current = userFront;
    while(current)
    {
        //printUser(current);
        callback(current, context);
        current = current->next;
    }

    pthread_mutex_unlock(&userLock);
}

// zum testen und debuggen
void printUser(User *user) 
{
    printf("User socket: %d, Thread ID: %lu name: %s\n", user->sock, user->thread, user->name);
}

void userRemoveAll(void)
{
    pthread_mutex_lock(&userLock);

    User *current = userFront;
    while (current) 
    {
        User *toFree = current;
        current = current->next;
        free(toFree);
    }

    userFront = NULL;

    pthread_mutex_unlock(&userLock);
}

User* searchUser(const char *name)
{
    pthread_mutex_lock(&userLock);
    User *current = userFront;
    //int pos = 0;

   while (current) {
        if (strcmp(current->name, name) == 0) {
            pthread_mutex_unlock(&userLock);
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&userLock);
    return NULL;
}
