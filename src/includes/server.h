/****************
 * server.h
 ***************/

#ifndef _server_
#define _server_

#include <pthread.h>
#include "HashMapADT.h"

void * draft(void * leagueID);

typedef struct User {
	char name[30];
	char password[30];
	int userID;
	ListADT * teams;
	int isConnected;
} User;

/*
 * Libera al User
 */
void freeUser(void * user);

/*
 * Libera el draft y todos los recursos usados
 */
void freeDraft(void * draft);

typedef struct TeamCh {
	Team * team;
	void * channel;
	int pipe[2];
} TeamCh;

typedef struct Draft {
	int cant;
	TeamCh * array;
	HashMap * hash;
	pthread_t thread;
	pthread_mutex_t draftMutex;
} Draft;

#endif
