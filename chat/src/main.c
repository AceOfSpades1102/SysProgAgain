#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "connectionhandler.h"
#include "util.h"
#include "broadcastagent.h"

int main(int argc, char **argv)
{
	utilInit(argv[0]);
	infoPrint("Chat server, group 23 :)");	//TODONE: group number is 23

	//TODONE: evaluate command line arguments

	int c = 0;
	int port = 8888;
	while((c = getopt(argc, argv, "p:dh")) != -1)
	{
		switch (c)
		{
			case 'p':
			{
				char *tmp = optarg;
				int valid = 1;
				while(*tmp != '\0')
					{
						if(*tmp < 48 || *tmp > 57)
						{
							valid = 0;
						}
						tmp++;
					}
				if(valid)
					{
						port = atoi(optarg);
						if(port < 1024 || port > 49151)
						{
							errorPrint("Port has to be between 1024 and 49151! Not %d!", port);
							return EXIT_FAILURE;
						}
					}
				else
					{
						errorPrint("Port (%s) not valid!", optarg);
						return EXIT_FAILURE;
					}
				break;
			}
			case 'd':
			{
				debugEnable();
				infoPrint("Debug mode enabled!");
				break;
			}
			case 'h':
			{
				infoPrint("Serverhelp:");
				infoPrint("Options are as follows:");
				infoPrint("-p <port> : Sets portnumber that's used. Allowed is everything between 1024 - 49151. Default is set to 8888");
				infoPrint("-d : enabels debug mode");
				infoPrint("-h : shows this help page");
				return EXIT_SUCCESS;
			}
		}
	}

	//TODONE: perform initialization
	pthread_t baThread = broadcastAgentInit();

	if(baThread == (pthread_t) -1)
	{
		return EXIT_FAILURE;
	}

	//TODODONE: use port specified via command line
	const int result = connectionHandler((in_port_t)port);

	//TODO: perform cleanup, if required by your implementation
	return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
