#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <endian.h>
#include <stdlib.h>
#include <time.h>
#include "network.h"
#include "user.h"
#include "util.h"
#include "def.h"

static ssize_t handleRecvReturn(ssize_t tmp, int fd, ssize_t expected_len){
    if(tmp == CONN_CLOSED){
        debugPrint("Socket: %i, performed a shutdown ！Σ(x_x;)!", fd);
        return CONN_CLOSED;
    }
    
    if(tmp == FAILED){
        debugPrint("Socket: %i, failed to receive message (´･ｪ･｀)", fd);
        return FAILED;
    }

    if(tmp < expected_len) {
        debugPrint("Socket: %i, received less than expected. (  -_・)?", fd);
        return FAILED;
    }

    if(tmp == expected_len){
        return RECV_SUCCESS;
    }

    debugPrint("Socket: %i, unexpected Error (._.)", fd);
    return FAILED;
}

int recieveHeader(int fd, Message *buffer)
{
    //recieve type
    debugPrint("search3");
    debugPrint("Test recieve header entered");
    ssize_t tmp = recv(fd, &(buffer->header.type), sizeof(buffer->header.type), MSG_WAITALL);
    debugPrint("type: %ld", (long)tmp);
    tmp = handleRecvReturn(tmp, fd, sizeof(buffer->header.type));
    debugPrint("type after test: %d", buffer->header.type);

    debugPrint("search4");


    if(tmp != RECV_SUCCESS)
    {
        return tmp;
    }

    //recieve length
    tmp = recv(fd, &buffer->header.length, sizeof(buffer->header.length), MSG_WAITALL);
    tmp = handleRecvReturn(tmp, fd, sizeof(buffer->header.length));

    if(tmp != RECV_SUCCESS)
    {
        return tmp;
    }
    buffer->header.length = ntohs(buffer->header.length);


    if(buffer->header.type > TYPE_MAX)
    {
        debugPrint("That was not a valid type ._.");
        return INVALID_TYPE;
    }

    switch(buffer->header.type)
    {
        case LRQ: 
        {
            if(buffer->header.length < LRQ_LEN_MIN || buffer->header.length > LRQ_LEN_MAX)
            {
                debugPrint("Socket: %i, Inavlid len=%i :(. message type: %i", buffer->header.length, fd, buffer->header.type);
                return INVALID_LEN;
            }
            return VALID_LEN;
        }
        case C2S: 
        {
            if(buffer->header.length > TEXT_MAX){
                debugPrint("Invalid len=%i :(. socket: %i, message type: %i", buffer->header.length, fd, buffer->header.type);
                return INVALID_LEN;
            }
            return VALID_LEN;
        }
        default: 
        {
            debugPrint("you shouldn't be here ┐('-`;)┌");
            return INVALID_TYPE;
        }
    }


}

int recieveMessage(int fd, Message *buffer)
{
    switch (buffer->header.type){
        case LRQ: {
            ssize_t tmp = recv(fd, &buffer->body.login_request, buffer->header.length, MSG_WAITALL);
            tmp = handleRecvReturn(tmp, fd, buffer->header.length);

            if (tmp != RECV_SUCCESS){
                return tmp;
            }

            buffer->body.login_request.magic = ntohl(buffer->body.login_request.magic);
            return RECV_SUCCESS;
        }
        case C2S:{
            debugPrint("recieving C2S Message");
            ssize_t tmp = recv(fd, &buffer->body.client_to_server, buffer->header.length, MSG_WAITALL);
            tmp = handleRecvReturn(tmp, fd, buffer->header.length);
            if (tmp != RECV_SUCCESS){
                return tmp;
            }
            return RECV_SUCCESS;
        }
    }
    return INVALID_TYPE;

}

int networkReceive(int fd, Message *buffer)
{
    
    int tmp = recieveHeader(fd, buffer);
    debugPrint("Header type: %d", buffer->header.type);
    debugPrint("Header length: %d", buffer->header.length);

    
    if (tmp != RECV_SUCCESS)
    {
        return FAILED;
    }
	

	//TODONE: Receive text
    tmp = recieveMessage(fd, buffer);
    if (tmp != RECV_SUCCESS)
    {
        return FAILED;
    }
	
	return RECV_SUCCESS;
}

int networkSend(int fd, const Message *buffer)
{
	//TODONE: Send complete message

    debugPrint("networkSend enterd");
    debugPrint("type=%d, length=%u", buffer->header.type, buffer->header.length);

    if (buffer->header.length > MSG_MAX) {
        errorPrint("networkSend: Buffer length > MAX_MESSAGE_SIZE.");
        errno = EINVAL;
        return EXIT_FAILURE;
    }

    debugPrint("NetworkSend");

    // Send the complete message
    size_t total_size = buffer->header.length + 3;

    debugPrint("NetworkSend2");

    unsigned char send_buffer[total_size];
    memset(send_buffer, 0, total_size); // Clear send buffer

    debugPrint("NetworkSend3");

    //write message in puffer
    send_buffer[0] = buffer->header.type;
    *(uint16_t *)(send_buffer + 1) = htons(buffer->header.length); // Length (network byte order) 
    memcpy(send_buffer + 3, &buffer->body, buffer->header.length); // Body


    debugPrint("send(): fd=%d, send_buffer=%p, total_size=%zd\n", fd, send_buffer, total_size);

    ssize_t tmp = send(fd, send_buffer, total_size, 0);

    if(tmp == -1)
    {
        debugPrint("send failed probably because broken pipe");
        return FAILED;
    }

    if(tmp != (ssize_t)total_size)
    {
        errnoPrint("send() %zu", sizeof(buffer->header) + ntohs(buffer->header.length));
        return FAILED;
    }

    debugPrint("Message sent successfully: type=%d, length=%u", buffer->header.type, buffer->header.length);

	return SUCCESS;
}

int broadcastServer2Client(const char *orig_sender,const char *text, uint64_t timestamp)
{
    debugPrint("broadcastServer2Client: Broadcasting Server2Client for user %s. ( ´∀｀ )b", orig_sender);
    
    struct {
        uint64_t timestamp;
        const char *original_sender;
        const char *text;
    } context = { .timestamp = timestamp, .original_sender = orig_sender, .text = text};

    forEachUser(broadcast_server2client_callback, &context);
    debugPrint("broadcastServer2Client: Finished broadcasting Server2Client for user %s. ( ´∀｀ )b", orig_sender);

    return SUCCESS;
}


int sendServer2Client(int receiver_client, const char *original_sender, uint64_t timestamp, const char *text)
{
    debugPrint("sendServer2Client: Sending Server2Client to client %d. ( ´∀｀ )b", receiver_client);
    
    Message msg;
    prepareServer2ClientMessage(&msg, original_sender, timestamp, text);

    debugPrint("Sending Server2Client message from %s to client (fd=%d)", original_sender, receiver_client);
    debugPrint("Message Debug: type=%d, length=%u, text='%s'", msg.header.type, msg.header.length, msg.body.server_to_client.text);
   
    if(networkSend(receiver_client, &msg) == -1) {
        debugPrint("Failed to send Server2Client from %s to client (fd=%d). Σ(x_x;)", original_sender, receiver_client);
        return FAILED;
    } else {
        debugPrint("Server2Client sent from %s to client (fd=%d). ( ´∀｀ )b", original_sender, receiver_client);
        return SUCCESS;
    }
    return SUCCESS;
   
}


void prepareServer2ClientMessage(Message *msg, const char *original_sender, uint64_t timestamp, const char *text)
{
    // validate original_sender
    size_t sender_len = strlen(original_sender);
    if (sender_len > NAME_MAX - 1) {
        errorPrint("prepareServer2ClientMessage: original_sender exceeds NAME_MAX ！Σ(x_x;).");
        sender_len = NAME_MAX - 1;
    }
    // validate text
    size_t text_len = strlen(text);
    if (text_len > MSG_MAX - 1) {
        errorPrint("prepareServer2ClientMessage: text exceeds MSG_MAX ！Σ(x_x;).");
        text_len = MSG_MAX - 1;
    }

    // Prepare message
    memset(msg, 0, sizeof(Message));

    msg->header.type = S2C; // Set message type to ServerToClient
    msg->header.length = TIMESTAMP_LEN + NAME_MAX + text_len; // Fixed size: timestamp + full name field + text
    
    // Convert 64-bit timestamp to network byte order
    msg->body.server_to_client.timestamp = htobe64(timestamp); 

    memset(msg->body.server_to_client.originalSender, 0, NAME_MAX);
    memcpy(msg->body.server_to_client.originalSender, original_sender, sender_len);
    msg->body.server_to_client.originalSender[sender_len] = '\0'; // Ensure null termination

    memcpy(msg->body.server_to_client.text, text, text_len);
    msg->body.server_to_client.text[text_len] = '\0'; // Ensure null termination

    debugPrint("prepareServer2ClientMessage: Prepared message for original_sender='%s', timestamp=%lu, text='%s'", 
                msg->body.server_to_client.originalSender, (unsigned long)timestamp, msg->body.server_to_client.text);
}

void broadcast_server2client_callback(User *user, void *context)
{
    if(!user || !context) {
        return;
    }
    struct {
        uint64_t timestamp;
        const char *original_sender;
        const char *text;
    } *ctx = context;

    Message msg;
    prepareServer2ClientMessage(&msg, ctx->original_sender, ctx->timestamp, ctx->text);

    debugPrint("broadcast_server2client_callback: Sending to user %s (fd=%d)", user->name, user->sock);
    debugPrint("Message Debug: type=%d, length=%u, text='%s'", msg.header.type, msg.header.length, msg.body.server_to_client.text);

    if(networkSend(user->sock, &msg) == -1) {
        debugPrint("Failed to send Server2Client to user %s (fd=%d). Σ(x_x;)", user->name, user->sock);
    } else {
        debugPrint("Server2Client sent to user %s (fd=%d). ( ´∀｀ )b", user->name, user->sock);
    }
}

uint64_t htonll(uint64_t value)
{
    // Handle 64-bit network byte-order conversion
    if (__BYTE_ORDER == __LITTLE_ENDIAN) {
        return ((uint64_t)htonl(value & 0xFFFFFFFF) << 32) | htonl(value >> 32);
    } else {
        return value; 
    }
}
