/****************
 * physic.h
 ***************/

#ifndef _physic_
#define _physic_

#define SERVER	0
#define CLIENT	1

#define MAX_SIZE 1024
#define MAX_DATA 1024

int createChannel(void *);

int receivePacket(char *, int, void *, int);

int sendPacket(char *, int, void *, int);

int getDefaultChannel(void *);

int getNextChannel(void *);

int channelToString(char *, void *);

int stringToChannel(char *, void *);

int connectToChannel(void *, int );

int disconnectFromChannel(void *);

int destroyChannel(void *);

#endif
