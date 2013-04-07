#include <stdio.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../includes/physic.h"
#include "../includes/misc.h"

typedef struct Channel{
	char name[100];
	int fd;
} Channel;

int
createChannels(void * clientList, void * serverList) {
	if ( mkfifo(((Channel *)serverList)->name, 0666) == -1 || mkfifo(((Channel *)clientList)->name, 0666) == -1) {
		return -1;
	}
	return 0;
}

int
connectToChannels(void * clientList, void * serverList, int who) {
	if ( ( ((Channel *)serverList)->fd = open( ((Channel *)serverList)->name, (who == SERVER) ? O_RDWR : O_RDONLY)) < 0 ) {
		return -1;
	}
	if ( ( ((Channel *)clientList)->fd = open( ((Channel *)clientList)->name, (who == SERVER) ? O_RDWR : O_WRONLY)) < 0 ) {
		return -1;
	}
	return 0;
}

int
receivePacket(char * buff, int size, void * channel) {
	return read(((Channel *)channel)->fd, buff, size);
}

int
sendPacket(char * buff, int size, void * channel) {
	return write(((Channel *)channel)->fd, buff, size);
}

int
disconnectFromChannels(void * c1, void * c2) {
	close(((Channel *)c1)->fd);
	close(((Channel *)c2)->fd);
	return 0;
}

int
getDefaultChannels(void * clientList, void * servList) {
	strcpy(((Channel *)clientList)->name, "/tmp/DefaultClList");
	strcpy(((Channel *)servList)->name, "/tmp/DefaultSvList");
	return 0;
}

static char serverName[31] = "0";
static char clientName[32] = "-0";

int
getNextChannels(void * clientList, void * serverList){
	int serverNum = atoi(serverName);
	serverNum++;
	itoa(serverName,serverNum);
	strcpy(clientName, "-");
	strcat(clientName, serverName);
	char buff[31] = "/tmp/";
	strcat(buff, clientName);
	strcpy( ((Channel *)clientList)->name, buff);
	strcpy(buff,"/tmp/");
	strcat(buff, serverName);
	strcpy( ((Channel *)serverList)->name, buff);	
	return 0;
}

int
channelsToString(char * buffer, void * c1, void * c2) {
	char * aux = buffer;
	printf("name: %s, fd: %d\n", ((Channel *)c1)->name, ((Channel *)c1)->fd);
	printf("name: %s, fd: %d\n", ((Channel *)c2)->name, ((Channel *)c2)->fd);
	strcpy(aux, ((Channel *)c1)->name);
	aux += (strlen(aux) + 1);
	strcpy(aux, ((Channel *)c2)->name);
	aux += (strlen(aux) + 1);
	return aux - buffer;
}

int
stringToChannels(char * buffer, void * c1, void * c2){
	strcpy(((Channel *)c1)->name , buffer);
	printf("name: %s\n", ((Channel *)c1)->name);
	buffer += (strlen(buffer) + 1);
	strcpy(((Channel *)c2)->name , buffer);
	printf("name: %s\n", ((Channel *)c2)->name);
	return 0;
}

int
main(){
	void * cL, * sL;
	cL = malloc(100);
	sL = malloc(100);
	
	void * cL1, * sL1;
	cL1 = malloc(100);
	sL1 = malloc(100);

	
	getNextChannels(cL, sL);
	printf("tienen q ser strings %s %s\n",((Channel *)cL)->name,((Channel *)sL)->name);
	
	createChannels(cL,sL);
	char buff[200];
	channelsToString(buff,cL,sL);

	stringToChannels(buff,cL1,sL1);
	printf("tienen q ser strings %s %s\n",((Channel *)cL1)->name,((Channel *)sL1)->name);
	getNextChannels(cL, sL);
	printf("tienen q ser strings %s %s\n",((Channel *)cL)->name,((Channel *)sL)->name);
	
	createChannels(cL,sL);
	channelsToString(buff,cL,sL);

	stringToChannels(buff,cL1,sL1);
	printf("tienen q ser strings %s %s\n",((Channel *)cL1)->name,((Channel *)sL1)->name);
	return 0;
}


