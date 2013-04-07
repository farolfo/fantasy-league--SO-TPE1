#include <stdio.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "../includes/physic.h"
#include "../includes/misc.h"

typedef struct Channel{
	char nameServerListen[60];
	int fdServerListen;
	char nameClientListen[60];
	int fdClientListen;
} Channel;

int
createChannel(void * c) {
	char clName[100] = "/tmp/", slName[100] = "/tmp/";
	strcat(slName, (char*)((Channel*)c)->nameServerListen);
	strcat(clName, (char*)((Channel*)c)->nameClientListen);
	if ( mkfifo(clName, 0666) == -1 || mkfifo(slName, 0666) == -1) {
		return -1;
	}
	return 0;
}

int
connectToChannel(void * c, int who) {
  	int fdcl, fdsv;
	char buffer[60] = "/tmp/";
	if ( c != NULL ) {
		strcat(buffer, ((Channel*)c)->nameClientListen);
		if ( ( fdcl = open(buffer, (who == SERVER) ? O_RDWR : O_RDONLY)) < 0 ) {
			return -1;
		}
		((Channel *)c)->fdClientListen = fdcl;
		strcpy(buffer,"/tmp/");
		strcat(buffer,((Channel*)c)->nameServerListen);
		if ( ( fdsv = open(buffer, (who == SERVER) ? O_RDWR : O_WRONLY)) < 0 ) {
			return -1;
		}
		((Channel *)c)->fdServerListen = fdsv;
		return 0;
	}
	return -1;
}

int
receivePacket(char * buff, int size, void * c, int who) {
	int fd = ( who == SERVER )? (((Channel *)c)->fdServerListen) : (((Channel *)c)->fdClientListen);
	int a = read(fd, buff, size);
	if ( a == -1  ) {
		printf("ERROR: %s\n", strerror(errno));
	}
	return a;
}

int
sendPacket(char * buff, int size, void * c, int who) {
	int fd = ( who == SERVER ) ? (((Channel *)c)->fdClientListen) : (((Channel *)c)->fdServerListen);
	int a =  write(fd, buff, size);
	if ( a == -1  ) {
		printf("ERROR: %s\n", strerror(errno));
	}

	return a;
}
int
disconnectFromChannel(void * c) {
	close(((Channel *)c)->fdServerListen);
	close(((Channel *)c)->fdClientListen);
	return 0;
}

int
getDefaultChannel(void * c) {
	strcpy(((Channel *)c)->nameClientListen, "DefaultClList");
	strcpy(((Channel *)c)->nameServerListen, "DefaultSvList");
	return 0;
}

static char serverName[31] = "0";
static char clientName[32] = "-0";

int
getNextChannel(void * c){
	int serverNum = atoi(serverName);
	serverNum++;
	itoa(serverName,serverNum);
	strcpy(clientName, "-");
	strcat(clientName, serverName);
	strcpy( ((Channel *)c)->nameClientListen, clientName);
	strcpy( ((Channel *)c)->nameServerListen, serverName);	
	return 0;
}

int
channelToString(char * buffer, void * c) {
	char * aux = buffer;
	strcpy(aux, ((Channel *)c)->nameServerListen);
	aux += (strlen(aux) + 1);
	strcpy(aux, ((Channel *)c)->nameClientListen);
	aux += (strlen(aux) + 1);
	return aux - buffer;
}

int
stringToChannel(char * buffer, void * c){
	strcpy(((Channel *)c)->nameServerListen , buffer);
	buffer += (strlen(buffer) + 1);
	strcpy(((Channel *)c)->nameClientListen , buffer);
	return 0;
}

int
destroyChannel(void * c){
	char buff[100] = "/tmp/";
	strcat(buff,((Channel *)c)->nameServerListen);
	remove(buff);
	strcpy(buff,"/tmp/");
	strcat(buff,((Channel *)c)->nameClientListen);
	remove(buff);
	return 0;
}


