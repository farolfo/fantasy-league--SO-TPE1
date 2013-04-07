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

//int sockfd_cli;
int sockfd_serv;

main(){
	int a;
	struct sockaddr_in addr_serv, addr_cli;
  	if( (sockfd_serv = socket( AF_INET, SOCK_DGRAM, 0)) == -1){
		printf("Error al crear el socket.\n");
		exit(0);
	}
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = 7000;
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	if ( /*setsockopt(((Channel *)c1)->sockfd, SOL_SOCKET, SO_REUSEADDR, NULL, 0) == -1 ||*/
		(a= bind(sockfd_serv, (struct sockaddr *)&(addr_serv), SIZE_ADD_LEN)) == -1 ) {
		if ( a == -1  ) {
			printf("ERROR: %s\n", strerror(errno));
		}
		printf("Error al bindear el socket.\n");
		exit(0);
	}
	addr_cli.sin_family = AF_INET;
	addr_cli.sin_port = 7001;
	addr_cli.sin_addr.s_addr = htonl(INADDR_ANY);
	/*if ( /*setsockopt(((Channel *)c2)->sockfd, SOL_SOCKET, SO_REUSEADDR, NULL, 0) == -1 ||*/
		/*bind( sockfd_cli, (struct sockaddr *)&(addr_cli), SIZE_ADD_LEN) == -1) {
		if ( -1 == -1  ) {
			printf("ERROR: %s\n", strerror(errno));
		}
		printf("Error al bindear el socket.\n");
		exit(0);
	}*/
	sleep(1);
	char buff[100];
	int s = SIZE_ADD_LEN;
	printf("reciviendo");
	recvfrom( sockfd_serv, buff, 1000, 0, (struct sockaddr *)&(addr_cli), (socklen_t *)&s);
	printf("%s\n",buff);
	sleep(1);
	strcpy(buff,"Chau");
	sendto( sockfd_serv, buff, 5, 0, (struct sockaddr *)&(addr_cli), SIZE_ADD_LEN);
	printf("envio.");
}