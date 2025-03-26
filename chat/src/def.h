#ifndef DEF_H
#define DEF_H

#include "util.h"
#include <string.h>
#include <time.h>
#include <netinet/in.h>


//chars






//return and write stuff >v<
enum { 
	MSG_MAX = 1024,
	NAME_MAX = 32,
	TEXT_MAX = 512,
	TYPE_MAX = 5,
	LRG_LEN_MIN = 6,
	LRG_LEN_MAX = 36,
	
	MAGIC_LRQ = 0x0badf00d,
	MAGIC_LRE = 0xc001c001,
	SERVER_VERSION = 0,
	
	LRQ = 0,
	LRE = 1,
	C2S = 2,
	S2C = 3,
	UAD = 4,
	URM = 5,


	//Login Responses
	LRE_SUCCESS = 0,
	LRE_NAME_TAKEN = 1,
	LRE_NAME_INVALD = 2,
	LRE_WRONG_VERSION = 3,
	LRE_UNKNOWN_ERROR = 255,

	VALID_TYPE = 1,

	// User removed
	CONN_CLOSED_CLIENT = 0,
	KICKED_FROM_SERVER = 1,
	COMMUNICATION_ERROR = 2,

	};



    #endif