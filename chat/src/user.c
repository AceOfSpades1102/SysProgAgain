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
    return EXIT_SUCCESS;
}


void removeUser(pthread_t thread_id)
{
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

// Remove user by socket fd (for broken pipe handling)
void removeUserBySock(int sock)
{
    pthread_mutex_lock(&userLock);

    User *current = userFront;
    while (current) {
        if (current->sock == sock) {
            debugPrint("Removing user '%s' from user list", current->name);
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                userFront = current->next;
            }
            if (current->next) {
                current->next->prev = current->prev;
            } else {
                userBack = current->prev;
            }
            close(current->sock);
            free(current);
            pthread_mutex_unlock(&userLock);
            return; // Immediately return after removal to avoid use-after-free
        }
        current = current->next;
    }
    pthread_mutex_unlock(&userLock);
}

// Safe iteration: allows callback to remove the current user
void forEachUser(void (*callback)(User *, void *), void *context)
{
    pthread_mutex_lock(&userLock);

    User *current = userFront;
    while(current)
    {
        User *next = current->next; // Save next before callback
        callback(current, context);
        current = next;
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
        //close socket
        close(toFree->sock);
        free(toFree);
    }

    userFront = NULL;

    pthread_mutex_unlock(&userLock);
}

User* searchUser(const char *name)
{
    // Assumes caller holds userLock!
    User *current = userFront;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}