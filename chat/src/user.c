#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "user.h"
#include "clientthread.h"
#include "util.h"
#include "def.h"

static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
User *userFront = NULL;
static User *userBack = NULL;

//TODO: Implement the functions declared in user.h

User* createUser(User **head, const char *name, int sock, pthread_t thread)
{
    pthread_mutex_lock(&userLock);
    User *newUser = (User *)malloc(sizeof(User));
    if (!newUser) {
        debugPrint("%s", allocError);
        pthread_mutex_unlock(&userLock);
        return NULL;
    }

    strncpy(newUser->name, name, MAX_NAME_LEN);

    newUser->name[MAX_NAME_LEN] = '\0';
    newUser->sock = sock;
    newUser->thread = thread;
    newUser->prev = NULL;
    newUser->next = *head;


    if(userFront != NULL)
    {
        (*userFront)->prev = newUser;
    }
        
    *userFront = newUser;

    //printf("userFront is now: %p\n", (void *)userFront);
    
    pthread_mutex_unlock(&userLock);
    //printUser(newUser);
    return newUser;
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
            }

            // Free the memory of the removed user
            free(current);

            // Unlock the list mutex and return
            pthread_mutex_unlock(&userLock);
            return;
        }
        current = current->next;
    }

    //printf("userFront is now: %p\n", (void *)userFront);

    pthread_mutex_unlock(&userLock);
    return;
}


void forEachUser(void (*callback)(User *))
{
    pthread_mutex_lock(&userLock);

    User *current = userFront;
    while(current)
    {
        //printUser(current);
        callback(current);
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
