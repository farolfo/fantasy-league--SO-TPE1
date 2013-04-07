#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../includes/physic.h"
#include "../includes/misc.h"

int
main(void) {
	/*server*/
	void * defChannel = malloc(200), * newChannel = malloc(100);
	int qty;
	char buffer[200];

	/*Test canal default recibo un uno y mando un 2*/
	getDefaultChannel(defChannel);

	createChannel(defChannel);

	connectToChannel(defChannel, SERVER);
	printf("1 . .Escucha...\n");

	receivePacket(buffer, 200, defChannel, SERVER);

	printf("1 . Alguien se conecto al default, recibi : %s\n",buffer);
	sleep(2);
	int ans = atoi(buffer) + 1;
	itoa(buffer,ans);
	sendPacket(buffer, strlen(buffer) + 1, defChannel, SERVER);
	printf("1 . Envio %s\n",buffer);
	sleep(2);
	sendPacket(buffer, strlen(buffer) + 1, defChannel, SERVER);
	printf("1 . Envio2 %s\n",buffer);
	printf("2 . Abro dos canales nuevos\n");

	getNextChannel(newChannel);

	createChannel(newChannel);

	connectToChannel(newChannel, SERVER);

	qty = channelToString(buffer, newChannel);
	sleep(2);
	sendPacket(buffer,qty,defChannel, SERVER);	
	//disconnectFromChannels(clientList, serverList);

	printf("2 . . Ahora el client tiene el primer nuevo canal.\n");
	sleep(2);
	receivePacket(buffer, 100, newChannel, SERVER);
	printf("2 . Recibio en el canal 2 : %s\n",buffer);
	ans = atoi(buffer) + 1;
	itoa(buffer,ans);
	sleep(2);
	sendPacket(buffer, strlen(buffer) + 1, newChannel, SERVER);
	printf("OK !\n");
	disconnectFromChannel( defChannel);
	disconnectFromChannel( newChannel);
	destroyChannel( defChannel);
	destroyChannel( newChannel);
	return 0;
}
