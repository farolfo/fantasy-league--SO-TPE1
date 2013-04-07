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


#define		DEFAULT_SERVER_PORT	1000
#define 	DEFAULT_CLIENT_PORT	1001
#define		SIZE_ADD_LEN		sizeof(struct sockaddr_in)

int sockfd_cli;
main(){
	int a;
	struct sockaddr_in addr_serv, addr_cli;
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = 7000;
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if( ( sockfd_cli = socket( AF_INET, SOCK_DGRAM, 0)) == -1){
		printf("Error al crear el socket.\n");
		exit(0);
	}
	addr_cli.sin_family = AF_INET;
	addr_cli.sin_port = 7001;
	addr_cli.sin_addr.s_addr = htonl(INADDR_ANY);

	char buff[100];
	sleep(1);
	strcpy(buff,"Hello");
	sendto( sockfd_cli, buff, 7, 0, (struct sockaddr *)&(addr_serv), SIZE_ADD_LEN);
	printf("envio.");
	sleep(1);
	int s = SIZE_ADD_LEN;
	printf("reciviendo");
	recvfrom( sockfd_cli, buff, 1000, 0, (struct sockaddr *)&(addr_serv), (socklen_t *)&s);
        printf("%s\n",buff);
	return 0;
}