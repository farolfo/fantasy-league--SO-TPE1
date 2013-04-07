#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include "../includes/physic.h"
#include "../includes/misc.h"

#define		DEFAULT_SERVER_PORT	7000
#define 	DEFAULT_CLIENT_PORT	7001
#define		SIZE_ADD_LEN		sizeof(struct sockaddr_in)

typedef struct Channel {
	int sockfd;
	int servPort;
	int cliPort;
	struct sockaddr_in addrServ;
	struct sockaddr_in addrCli;
} Channel;

int 
getDefaultChannel(void * c){
	((Channel *)c)->cliPort = DEFAULT_CLIENT_PORT;
	((Channel *)c)->servPort = DEFAULT_SERVER_PORT;
	return 0;
}

static int nextPort = 7002;

int getNextChannel(void * c){
	((Channel *)c)->servPort = nextPort++;
	((Channel *)c)->cliPort = nextPort++;
	return 0;
}

//Importante repsetar los ordenes de los parametros
int createChannel(void * c){
	int a;
	if( ( ((Channel *)c)->sockfd = socket( AF_INET, SOCK_DGRAM, 0)) == -1){
		printf("Error al crear el socket.\n");
		exit(0);
	}
	((Channel *)c)->addrServ.sin_family = AF_INET;
	((Channel *)c)->addrServ.sin_port = ((Channel *)c)->servPort;
	((Channel *)c)->addrServ.sin_addr.s_addr = htonl(INADDR_ANY);
	if ( /*setsockopt(((Channel *)c1)->sockfd, SOL_SOCKET, SO_REUSEADDR, NULL, 0) == -1 ||*/
		(a= bind(((Channel *)c)->sockfd, (struct sockaddr *)&(((Channel *)c)->addrServ), SIZE_ADD_LEN)) == -1 ) {
		if ( a == -1  ) {
			printf("ERROR: %s\n", strerror(errno));
		}
		printf("Error al bindear el socket.\n");
		exit(0);
	}
	((Channel *)c)->addrCli.sin_family = AF_INET;
	((Channel *)c)->addrCli.sin_port = ((Channel *)c)->cliPort;
	((Channel *)c)->addrCli.sin_addr.s_addr = htonl(INADDR_ANY);
	return 0; 
}

int receivePacket(char * buff, int size, void * c, int who){
	int s = SIZE_ADD_LEN;
	//printf("Leo del fd en el puerto %d\n", ((Channel*)channel)->addrOther.sin_port);
	return recvfrom( ((Channel *)c)->sockfd, buff, size, 0, (who==CLIENT) ? ((struct sockaddr *)&(((Channel *)c)->addrServ)) : ((struct sockaddr *)&(((Channel *)c)->addrCli)), (socklen_t *)&s);
	//recv( sockfd, buff, size, 0);
}

int sendPacket(char * buff, int size, void * c, int who){
	//printf("Escribo en el fd en el puerto %d\n",  ((Channel*)channel)->addrOther.sin_port);
	sendto( ((Channel *)c)->sockfd, buff, size, 0, (who==CLIENT)?((struct sockaddr *)&(((Channel *)c)->addrServ)):((struct sockaddr *)&(((Channel *)c)->addrCli)), SIZE_ADD_LEN);
	return 0;
}


int channelToString(char * buff, void * c){
	char * begin = buff;
	itoa(buff, ((Channel *)c)->servPort);
	buff += strlen(buff) + 1;
	itoa(buff, ((Channel *)c)->cliPort);
	buff += strlen(buff) + 1;
	return (int)(buff - begin);
}

int stringToChannel(char * buff, void * c){
	char * index = buff;
	int servPortAux, cliPortAux;
	servPortAux = atoi(index);
	index += strlen(index) + 1;
	cliPortAux = atoi(index);
	((Channel *)c)->servPort = servPortAux;
	((Channel *)c)->cliPort = cliPortAux;
	return 0;
}

int 
connectToChannel(void * c, int who){
	if(who == CLIENT){
		((Channel *)c)->addrServ.sin_family = AF_INET;
		((Channel *)c)->addrServ.sin_port = ((Channel *)c)->servPort;
		((Channel *)c)->addrServ.sin_addr.s_addr = htonl(INADDR_ANY);
		((Channel *)c)->addrCli.sin_family = AF_INET;
		((Channel *)c)->addrCli.sin_port = ((Channel *)c)->cliPort;
		((Channel *)c)->addrCli.sin_addr.s_addr = htonl(INADDR_ANY);
		if( ( ((Channel *)c)->sockfd = socket( AF_INET, SOCK_DGRAM, 0)) == -1){	
			printf("Error al crear el socket.\n");
			exit(0);
		}
	}
	return 0;
}

int disconnectFromChannel( void * c){
	close(((Channel *)c)->sockfd);
	return 0;
}

int
destroyChannel(void * c){
	/*--Non supported operation--*/	
	return 0;
}

