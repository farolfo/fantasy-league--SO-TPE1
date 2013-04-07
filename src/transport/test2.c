#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../includes/physic.h"
#include "../includes/misc.h"

int
main(void) {
	
	void * defChannel = (void *)malloc(100);
	void * newChannel = (void *)malloc(100);

	char buff[200];
	/*---Establezco los canales de comunicacion---*/
	printf("1  .. Inicia cliente.\n");
	getDefaultChannel(defChannel);
	printf("1 .. Obtiene canales default.\n");
	if ( connectToChannel(defChannel,CLIENT) < 0 ) {
		printf("Server not available\n");
		return 0;
	}
	itoa(buff,1);
	
	sendPacket(buff,strlen(buff)+1,defChannel,CLIENT);
	printf("1 .. Envia %s\n",buff);
	sleep(1);
	receivePacket(buff,MAX_SIZE,defChannel,CLIENT);
	printf("1  .  Recibe %s\n",buff);
	sleep(2);
	receivePacket(buff,MAX_SIZE,defChannel,CLIENT);
	printf("1  .  Recibe2 %s\n",buff);
	
	/*--Recibo nuevo canal-*/
	int aux = atoi(buff) + 1;
	printf("2 .. Ahora recibo por dos canales nuevos.\n");
	sleep(2);
	receivePacket(buff,MAX_SIZE,defChannel,CLIENT);
	printf("recibi algo q serian los nuevos\n");
	stringToChannel(buff,newChannel);
	connectToChannel(newChannel,CLIENT);
	printf("2 .. Obtuve el primer nuevo canale\n");
	itoa(buff,1);
	printf("2 . .envio en el nuevo canal2 : %s\n",buff);
	sleep(2);	
	sendPacket(buff, strlen(buff)+1, newChannel,CLIENT);
	sleep(2);	
	receivePacket(buff,MAX_SIZE,newChannel,CLIENT);
	printf("2 .. recibio %s\n",buff);
	printf("OK !\n");
	disconnectFromChannel(newChannel);
	disconnectFromChannel(defChannel);

	return 0;
}
