#include <stdio.h>
#include "../includes/physic.h"
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "../includes/misc.h"

#define DEFAULT_MTYPE_CLIENT    1
#define DEFAULT_MTYPE_SERVER	2

#define MAX_LENGTH_CHANNEL 1024

struct q_entry {
	long mtype;
	char mtext[MAX_LENGTH_CHANNEL];
};

key_t msgkey = 0xCABE2A;
int msgid;

typedef struct Channel{
	long mtypeServ;
	long mtypeCli;
} Channel;

int
createChannel(void * c) {
	if ( ( msgid == msgget( msgkey, 0666 | IPC_CREAT )) == -1 ){
		printf("Error al crear la cola de mensajes.");
		exit(1);
	}
	return 0;
}

int
destroyChannel(void * c){
	if( msgctl( msgid, IPC_RMID, NULL) == -1 ){
		printf("Error al destruir la cola de mensajes.\n");
		exit(0);
	}
	return 0;
}


int
connectToChannel(void * c, int who) {
  	if ( who == CLIENT ) {
		if ( ( msgid == msgget( msgkey, 0666 | IPC_CREAT )) == -1 ){
			printf("Error al crear la cola de mensajes.");
			exit(1);
		}
	} 		
	return 0;
}

int
receivePacket(char * buff, int size, void * c, int who){
	struct q_entry r_entry;
	int len;
	long mtype = (who==SERVER)?(((Channel *)c)->mtypeServ):(((Channel *)c)->mtypeCli);
	len = msgrcv( msgid, &r_entry, size, mtype, MSG_NOERROR);
	memcpy(buff, r_entry.mtext, len);
	return len;
}

int
sendPacket(char * buff, int size, void * c, int who) {
	struct q_entry s_entry;
	s_entry.mtype = (who==SERVER)?((Channel *)c)->mtypeCli:((Channel *)c)->mtypeServ;
	memcpy(s_entry.mtext, buff, size);
	return msgsnd( msgid, &s_entry, size, 0);
}

int
disconnectFromChannel(void * c) {
	/*Non supported operation*/
	return 0;
}

int 
getDefaultChannel(void * c) {
	((Channel *)c)->mtypeCli = DEFAULT_MTYPE_CLIENT;
	((Channel *)c)->mtypeServ = DEFAULT_MTYPE_SERVER;
	//*(int *)(clientList+1) = *(int *)(serverList+1) = 0;
	return 0;
}

static long nextChannel = 3; 

int 
getNextChannel(void * c){
	((Channel *)c)->mtypeServ = nextChannel++;
	((Channel *)c)->mtypeCli = nextChannel++;
	return 0;
}

int 
channelToString(char * buff, void * c){
	char * begin = buff;
	itoa( buff, ((Channel *)c)->mtypeServ);
	buff += strlen(buff) + 1;
	itoa( buff, ((Channel *)c)->mtypeCli);
	buff += strlen(buff) + 1;
	return buff-begin;
}

int 
stringToChannel(char * buff, void * c){
	((Channel *)c)->mtypeServ = atoi(buff);
	buff += strlen(buff) + 1;
	((Channel *)c)->mtypeCli = atoi(buff);
	buff += strlen(buff) + 1;
	return 0;
}


