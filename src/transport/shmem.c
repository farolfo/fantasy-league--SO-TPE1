#include <sys/shm.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "../includes/misc.h"
#include "../includes/physic.h"

#define SERVER_CHANNEL_DEF	300
#define CLIENT_CHANNEL_DEF	301
#define SERVER_SEM_DEF		0X20
#define	CLIENT_SEM_DEF		0X21
#define TO_WRITE		1
#define TO_READ			2

typedef struct Channel{
	/*--Server Listen Here--*/
	key_t keyS;
	int shmIDS;
	char * shmDirS;
	int semkeyS;
	int semidS;
	
	/*--Client Listen Here--*/
	key_t keyC;
	int shmIDC;
	char * shmDirC;
	int semkeyC;
	int semidC;
} Channel;

typedef union semun{
	int val;
	struct semid_ds *buf;
	ushort * array;
} semun;

struct sembuf p1 = {0,-1,0}, p2 = {1,-1,0};
struct sembuf v1 = {0,1,0}, v2 = {1,1,0};
static int nextSemKey = 0x22;

int
getsem(int semkey, int who){
	int semid;
	semun x;
	x.val = 0;
	int flags = (who == SERVER) ? (IPC_CREAT | IPC_EXCL) : (IPC_CREAT);
	if( ( semid = semget( semkey, 2, 0666 | flags)) == -1){
		return -1;
	}
	if(semctl(semid, 0, SETVAL, x) == -1){
		return -1;
	}
	if(semctl(semid, 1, SETVAL, x) == -1){
		return -1;
	}
	
	return semid;
}

int
createChannel( void * c){		
	/*--Seteo de SharedMemory--*/
	if( ( ((Channel *)c)->shmIDC = shmget( ((Channel *)c)->keyC, MAX_SIZE, 0666 | IPC_CREAT ) ) == -1 ){
		printf("Error al crear el canal de transmision de cliente: %s\n", strerror(errno));
		return -1;
	}
	if( ( ((Channel *)c)->shmIDS = shmget( ((Channel *)c)->keyS, MAX_SIZE, 0666 | IPC_CREAT ) ) == -1 ){
		printf("Error al crear el canal de transmision de servidor: %s\n", strerror(errno));
		return -1;
	}		
		
	if( ( ((Channel *)c)->semidC = getsem( ((Channel *)c)->semkeyC, SERVER) ) == -1){
		printf("Error al crear el canal de transmision del cliente (Conf. de Semaforos): %s\n", strerror(errno));
		return -1;
	}
	if( ( ((Channel *)c)->semidS = getsem(((Channel *)c)->semkeyS, SERVER)) == -1){
		printf("Error al crear el canal de transmision del servidor (Conf. de Semaforos): %s\n", strerror(errno));
		return -1;
	}
	semun toOne;
	toOne.val = 1;
	semctl(((Channel *)c)->semidS, 0, SETVAL, toOne);
	semctl(((Channel *)c)->semidC, 0, SETVAL, toOne);
	return 0;
}

int 
receivePacket(char * reciveBuff, int size, void * c, int who){
	int semid = (who==SERVER)?(((Channel *)c)->semidS):(((Channel *)c)->semidC);
	char * shmDir = (who==SERVER)?(((Channel *)c)->shmDirS):(((Channel *)c)->shmDirC);
	semop(semid,&p2,1);
	int i = 0, j;
	size = atoi(shmDir);
	i = strlen(shmDir) + 1;
	for( j=0; j < size; i++, j++){
		reciveBuff[j] = (shmDir)[i];
	}	
	semop(semid,&v1,1);
	semun toZero;
	toZero.val = 0;
	semctl(semid, 1, SETVAL, toZero);
	return size;
}

int 
sendPacket(char * sendBuff, int size, void * c, int who){
	int semid = (who==SERVER)?(((Channel *)c)->semidC):(((Channel *)c)->semidS);
	char * shmDir = (who==SERVER)?(((Channel *)c)->shmDirC):(((Channel *)c)->shmDirS);
	semop(semid,&p1,1);
	char sizeS[100];
	itoa( sizeS, size);
	memcpy(shmDir, sizeS, strlen(sizeS)+1);
	memcpy(shmDir + strlen(sizeS) + 1, sendBuff, size);	
	semop(semid,&v2,1);
	semun toZero;
	toZero.val = 0;
	semctl(semid, 0, SETVAL, toZero);
	return 0;
}

int 
getDefaultChannel(void * c){
	((Channel *)c)->keyC = CLIENT_CHANNEL_DEF;
	((Channel *)c)->keyS = SERVER_CHANNEL_DEF;
	((Channel *)c)->semkeyC = CLIENT_SEM_DEF;
	((Channel *)c)->semkeyS = SERVER_SEM_DEF;
	return 0;
}

static key_t nextServerChannel = 302; 
static key_t nextClientChannel = 303; 

int 
getNextChannel(void * c){
	((Channel *)c)->keyC = nextClientChannel;
	((Channel *)c)->keyS = nextServerChannel;
	((Channel *)c)->semkeyC = nextSemKey;
	nextSemKey ++;
	((Channel *)c)->semkeyS = nextSemKey;
	nextServerChannel += 2;
	nextClientChannel += 2;
	nextSemKey ++;
	return 0;
}

int 
channelToString(char * buff, void * c){
	char * begin = buff;
	itoa( buff, ((Channel *)c)->keyS);
	buff += strlen(buff) + 1;
	itoa( buff, ((Channel *) c)->shmIDS);
	buff += strlen(buff) + 1;
	//No pasa el shmDir pues este depende del connectToChannel
	itoa( buff, ((Channel *) c)->semkeyS);
	buff += strlen(buff) + 1;
	itoa( buff, ((Channel *) c)->keyC);
	buff += strlen(buff) + 1;
	itoa( buff, ((Channel *) c)->shmIDC);
	buff += strlen(buff) + 1;
	//No pasa el shmDir pues este depende del connectToChannel
	itoa( buff, ((Channel *) c)->semkeyC);
	buff += strlen(buff) + 1;
	return buff-begin;
}

int 
stringToChannel(char * buff, void * c){
	((Channel *)c)->keyS = atoi(buff);
	buff += strlen(buff) + 1;
	((Channel *)c)->shmIDS = atoi(buff);
	buff += strlen(buff) + 1;
	((Channel *)c)->semkeyS = atoi(buff);
	buff += strlen(buff) + 1;
	((Channel *)c)->keyC = atoi(buff);
	buff += strlen(buff) + 1;
	((Channel *)c)->shmIDC = atoi(buff);
	buff += strlen(buff) + 1;
	((Channel *)c)->semkeyC = atoi(buff);
	buff += strlen(buff) + 1;
	return 0;
}

int 
connectToChannel(void * c, int who){
	if(who == CLIENT){/*--Seteo de Semaforos--*/
		if( ( ((Channel *)c)->semidC = getsem(((Channel *)c)->semkeyC, CLIENT)) == -1){
			printf("Error al crear el canal de transmision del cliente (Conf. de Semaforos): %s\n", strerror(errno));
			return -1;
		}
		if( ( ((Channel *)c)->semidS = getsem(((Channel *)c)->semkeyS, CLIENT)) == -1){
			printf("Error al crear el canal de transmision del servidor (Conf. de Semaforos): %s\n", strerror(errno));
			return -1;
		}

		if( ( ((Channel *)c)->shmIDC = shmget( ((Channel *)c)->keyC, MAX_SIZE, 0666 ) ) == -1 ){
			printf("Error al crear el canal de transmision de cliente: %s\n", strerror(errno));
			return -1;
		}
	
		if( ( ((Channel *)c)->shmIDS = shmget( ((Channel *)c)->keyS, MAX_SIZE, 0666 ) ) == -1 ){
			printf("Error al crear el canal de transmision de servidor: %s\n", strerror(errno));
			return -1;
		}	
	}

	if( ( ((Channel *)c)->shmDirS = (char *)shmat(((Channel *)c)->shmIDS,0,0)) == (void *)-1){ 
		printf("Error al conectar al canal de transmicion: %s\n", strerror(errno));
		return -1;
	}
	if( ( ((Channel *)c)->shmDirC = (char *)shmat(((Channel *)c)->shmIDC,0,0)) == (void *)-1){
		printf("Error al conectar al canal de transmicion: %s\n", strerror(errno));
		return -1;
	}
	semun toOne;
	toOne.val = 1;
	semctl(((Channel *)c)->semidC, 0, SETVAL, toOne);
	semctl(((Channel *)c)->semidS, 0, SETVAL, toOne);		
	return 0;
}

int 
disconnectFromChannel(void * c){
	if( shmdt(((Channel *)c)->shmDirS) == -1 || shmdt(((Channel *)c)->shmDirC) == -1 ){
		printf("Error al desconectar los canales de transmicion: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int
destroyChannel(void * c){
	if( shmctl(((Channel *)c)->shmIDS, IPC_RMID, NULL) == -1 || shmctl(((Channel *)c)->shmIDC, IPC_RMID, NULL) == -1 ){
		printf("Error al destruir los canales de transmicion: %s\n", strerror(errno));
		return -1;
	}
	if( semctl( ((Channel *)c)->semidS, 0,IPC_RMID, NULL) == -1 || semctl(((Channel *)c)->semidC, 0,IPC_RMID, NULL) == -1 ){
		printf("Error al destruir los semaforos: %s\n", strerror(errno));
		return -1;	
	}
	return 0;
}
