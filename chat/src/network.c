#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h> // For htons, htonl, ntohs, ntohl
#include <stdlib.h>
#include <time.h>
#include "network.h"
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

int recieveHeader(int fd, messageHeader *buffer)
{
    //recieve type
    ssize_t tmp = recv(fd, &buffer->type, sizeof(buffer->type), MSG_WAITALL);
    tmp = handleRecvReturn(tmp, fd, sizeof(buffer->type));

    if(tmp != RECV_SUCCESS)
    {
        return tmp;
    }

    //recieve length
    tmp = recv(fd, &buffer->length, sizeof(buffer->length), MSG_WAITALL);
    tmp = handleRecvReturn(tmp, fd, sizeof(buffer->length));

    if(tmp != RECV_SUCCESS)
    {
        return tmp;
    }
    buffer->length = ntohs(buffer->length);

    //replace that
    //return checkHeader(buffer, fd);
    //with this

    if(buffer->type > TYPE_MAX)
    {
        debugPrint("That was not a valid type ._.");
        return INVALID_TYPE;
    }

    switch(buffer->type)
    {
        case LRQ: 
        {
            if(buffer->length < LRQ_LEN_MIN || buffer->length > LRQ_LEN_MAX)
            {
                debugPrint("Socket: %i, Inavlid len=%i :(. message type: %i", buffer->length, fd, buffer->type);
                return INVALID_LEN;
            }
            return VALID_LEN;
        }
        case C2S: 
        {
            if(buffer->length > TEXT_MAX){
                debugPrint("Invalid len=%i :(. socket: %i, message type: %i", buffer->length, fd, buffer->type);
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
    debugPrint("Header type: %d", buffer->header.type);
    debugPrint("Header length: %d", buffer->header.length);
    int tmp = recieveHeader(fd, &buffer->header);
    debugPrint("header is: %d", tmp);
    debugPrint("Header type: %d", buffer->header.type);
    debugPrint("Header length: %d", buffer->header.length);
    
    if (tmp != RECV_SUCCESS)
    {
        return FAILED;
    }
	

	//TODO: Receive text
    tmp = recieveMessage(fd, buffer);
    if (tmp != RECV_SUCCESS)
    {
        return FAILED;
    }
	
	return RECV_SUCCESS;
}

int networkSend(int fd, const Message *buffer)
{
	//TODO: Send complete message

    debugPrint("networkSend enterd");
    debugPrint("type=%d, length=%u", buffer->header.type, buffer->header.length);

    if (buffer->header.length > MSG_MAX) {
        errorPrint("networkSend: Buffer length > MAX_MESSAGE_SIZE.");
        errno = EINVAL;
        return EXIT_FAILURE;
    }

    // Send the complete message
    size_t total_size = buffer->header.length + 3;

    unsigned char send_buffer[total_size];
    memset(send_buffer, 0, total_size); // Clear send buffer

    // nachricht in den puffer schreiben
    send_buffer[0] = buffer->header.type;
    *(uint16_t *)(send_buffer + 1) = htons(buffer->header.length); // Length (network byte order) 
    memcpy(send_buffer + 3, &buffer->body, buffer->header.length); // Body



    ssize_t tmp = send(fd, send_buffer, total_size, 0);

    if(tmp != (ssize_t)total_size)
    {
        errnoPrint("send() %zu", sizeof(buffer->header) + ntohs(buffer->header.length));
        return FAILED;
    }

    debugPrint("Message sent successfully: type=%d, length=%u", buffer->header.type, buffer->header.length);

	return SUCCESS;
}
