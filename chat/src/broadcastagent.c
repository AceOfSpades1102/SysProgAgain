#include <pthread.h>
#include <mqueue.h>
#include "broadcastagent.h"
#include "util.h"

static mqd_t messageQueue;
static pthread_t threadId;
static int exitFlag = 0;
static int is_paused = 0;


static void *broadcastAgent(void *arg)
{	//TODONE: Implement thread function for the broadcast agent here!
	debugPrintf("Broadcast agent started ( ´∀｀ )b \n");
	BroadcastMessage msg;
	
	while (!exitFlag) {
		while (is_paused) {
			sleep(1);
			if (exitFlag) {
				break;
			}
		}
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += 1; // wait for 1 second

		ssize_t bytesRead = mq_timedreceive(messageQueue, (char *)&msg, sizeof(msg), NULL, &ts);

		if (bytesRead == -1) {
			if (errno == ETIMEDOUT) {
				// Timeout!!!!1
				continue;
			}
		}
		
		if (exitFlag) {
			errnoPrint("Broadcast agent failed Σ(x_x;)!");
			break;
		}
		debugPrint("Broadcast agent received message from '%s':  '%s' ( ´∀｀ )b\n", msg.sender, msg.message);

		// share message with all users
		broadcastServer2Client(msg.sender, msg.message, msg.timestamp);
	}
	return NULL;	

	}


int broadcastAgentInit(void)
{
	struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10, // Max number of messages in the queue
        .mq_msgsize = sizeof(BroadcastMessage), // Must match message size
        .mq_curmsgs = 0
    };
	 // Initialize the semaphore (1 = broadcasting enabled)
	 if (sem_init(&pause_sem, BROADCASTING_DISABLED, BROADCASTING_ENABLED) == -1) {
        errnoPrint("broadcastAgentInit: Failed to initialize semaphore");
        return EXIT_FAILURE;
    }

	//TODONE: create message queue
	messageQueue = mq_open(BROADCAST_QUEUE_NAME, O_CREAT | O_RDWR, 0666, &attr);
	if (messageQueue == (mqd_t)-1) {
		errnoPrint("broadcastAgentInit: Failed to create message queue Σ(x_x;)!");
		return EXIT_FAILURE;
	}
	debugPrint("Broadcast agent: Message queue created successfully ( ´∀｀ )b %d \n" messageQueue);

	//TODONE: start thread
	if (pthread_create(&threadId, NULL, broadcastAgent, NULL) != 0) {
		errnoPrint("broadcastAgentInit: Failed to create thread Σ(x_x;)!");
		mq_close(messageQueue);
		mq_unlink(BROADCAST_QUEUE_NAME);
		return EXIT_FAILURE;
	}
	debugPrint("Broadcast agent: Thread created successfully ( ´∀｀ )b %lu \n", threadId);
	return EXIT_SUCCESS;
}

void broadcastAgentCleanup(void)
{
	exitFlag = 1;

	pthread_cancel(threadId);
	pthread_join(threadId, NULL);
	sem_destroy(&pause_sem);

	//TODONE: stop thread
	if(mq_close(messageQueue) == -1) {
		errnoPrint("broadcastAgentCleanup: Failed to close message queue Σ(x_x;)!");
	}
	//TODONE: destroy message queue
	if(mq_unlink(BROADCAST_QUEUE_NAME) == -1) {
		errnoPrint("broadcastAgentCleanup: Failed to unlink message queue Σ(x_x;)!");
	}
	debugPrint("Broadcast agent: Successfully killed ( ´∀｀ )b \n");
}

//TODONE: Implement broadcastMessage function

int broadcastMessage(const char *sender, const char *text, uint64_t timestamp)
{
	if messageQueue == (mqd_t)-1) {
		errnoPrint("broadcastMessage: Message queue not initialized Σ(x_x;)!");
		return EXIT_FAILURE;
	}

	BroadcastMessage msg;

	strncpy(msg.sender, sender, MAX_NAME);
	msg.sender[MAX_NAME] = '\0'; // Ensure null termination

	strncpy(msg.message, text, MAX_MESSAGE);
	msg.message[MAX_MESSAGE - 1] = '\0'; // Ensure null termination
	
	msg.timestamp = timestamp;
	
	//timeout handling
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 1; // wait for 1 second

	debugPrint("Broadcasting message: trying to send '%s' from '%s' at %lu ( ´∀｀ )b \n", msg.message, msg.sender, msg.timestamp);
	int result = mq_timedsend(messageQueue, (const char *)&msg, sizeof(msg), 0, &ts);

	if result (== -1) {
		if (errno == ETIMEDOUT) {
			errnoPrint("broadcastMessage: Message sent timed out Σ(x_x;)!");
			return EXIT_FAILURE;
		} else {
			errnoPrint("broadcastMessage: Failed to send message Σ(x_x;)!");
			//sendServer2Client(getUserByName(sender)->socket, "Broadcasting failed: Queue full");
			return EXIT_FAILURE;
		}
	}

	debugPrint("Broadcasting message: '%s' from '%s' at %lu ( ´∀｀ )b \n", msg.message, msg.sender, msg.timestamp);
	return EXIT_SUCCESS;
}

//TODONE : Implement pauseBroadcasting function
void pauseBroadcasting() {
	if (!is_paused){
		debugPrint("Broadcasting paused ( ´∀｀ )b \n");
		is_paused = 1;
		
	} else {
		debugPrint("Broadcasting already paused ( ´∀｀ )b \n");
	}
}

// TODONE: Implement resumeBroadcasting function
void resumeBroadcasting() {
	if (is_paused){
		debugPrint("Broadcasting resumed ( ´∀｀ )b \n");
		is_paused = 0;
		
	} else {
		debugPrint("Broadcasting already running ( ´∀｀ )b \n");
	}
}
