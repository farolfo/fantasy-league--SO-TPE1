#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <setjmp.h>

#include "../includes/serverFunctions.h"
#include "../includes/clserv.h"
#include "../includes/misc.h" 
#include "../includes/HashMapADT.h"
#include "../includes/comparators.h"
#include "../includes/physic.h"
#include "../includes/dbms.h"
#include "../includes/server.h"

#define MAX_DATA	1024
#define MAX_PLAYERS	5
extern HashMap * users;
extern HashMap * teams;
extern HashMap * leagues;
extern HashMap * trades;
extern HashMap * drafts;

extern Player * players[MAX_PLAYERS];

extern pthread_mutex_t saveMutex;

static int
process_signUp(char * buffer, int qty) {
	char username[31], password[31], aux[10];
	int ret;
	strcpy(username, buffer);
	strcpy(password, buffer + strlen(username) + 1 );
	ret = signUp(username, password);
	strcpy(buffer, itoa(aux, ret));
	return strlen(buffer) + 1;
}

static int
process_login(char * buffer, int qty) {
	char username[31], password[31], aux[10];
	int ret;
	strcpy(username, buffer);
	strcpy(password, buffer + strlen(username) + 1 );
	ret = login(username, password);
	strcpy(buffer, itoa(aux, ret));
	return strlen(buffer) + 1;
}

static int
process_requestTrade(char * buffer, int qty) {
	ID teamIDFrom, teamIDTo, offeredPlayer, requestedPlayer;
	char * aux = buffer, itoaAux[10], username[31];
	int ret;
	strcpy(username, aux);
	aux += (strlen(aux) + 1);
	teamIDFrom = atoi(aux);
	aux += (strlen(aux) + 1);
	teamIDTo = atoi(aux);
	aux += (strlen(aux) + 1);
	offeredPlayer = atoi(aux);
	aux += (strlen(aux) + 1);
	requestedPlayer = atoi(aux);
	ret = requestTrade(username, teamIDFrom, teamIDTo, offeredPlayer, requestedPlayer);
	strcpy(buffer, itoa(itoaAux, ret));
	return strlen(buffer) + 1;
}

static int
process_NmsID( char * buffer, NmsID * (*f)() ) {
	NmsID * ret;
	int i = 0;
	char * aux = buffer, itoaAux[10];
	ret = (*f)();
	while ( ret[i].id != -1 ) {
		strcpy(aux, ret[i].name);
		aux += (strlen(aux) + 1);
		strcpy(aux, itoa(itoaAux, ret[i].id));
		aux += (strlen(aux) + 1);
		i++;
	}
	strcpy(aux, "");
	return aux - buffer + 1;	
}


static int
process_getLeaguesNames(char * buffer, int qty) {
	return process_NmsID(buffer, getLeaguesNames);
}

static int
process_getTeamsNames(char * buffer, int qty) {
	return process_NmsID(buffer, getTeamsNames);	
}
/*
static int
tstoa(TeamSnd ts, char * buff) {
	strcpy(buff, ts.teamName);
	buff += (strlen(buff) + 1);
	strcpy(buff, ts.leagueName);
	buff += (strlen(buff) + 1);
	strcpy(buff, itoa(ts.cantPlayers));
	buff += (strlen(buff) + 1);
	j = 0;
	while ( j < ts.cantPlayers ) {
		strcpy(buff, ts.cantPlayers[j].name);
		buff += (strlen(buff) + 1);
		strcpy(buff, itoa(ts.cantPlayers[j].playerID));
		buff += (strlen(buff) + 1);
		strcpy(buff, itoa(ts.cantPlayers[j].score));
		buff += (strlen(buff) + 1);
	}
	strcpy(aux, itoa(ts.points));
	buff += (strlen(buff) + 1);
}
*/

static int
process_getLeague(char * buffer, int qty) {
	LeagueSnd ret;
	ID leagueID = atoi(buffer);
	char * aux = buffer, itoaAux[10];
	TeamSnd auxTeam;
	int i = 0, j;
	ret = getLeague(leagueID);
	strcpy(aux, ret.name);
	aux += (strlen(aux) + 1);
	if ( *(ret.name) == '\0' ) {
		return aux - buffer;
	}
	while ( ret.teams[i].points != -1 ) {
		auxTeam = ret.teams[i];
		strcpy(aux, auxTeam.teamName);
		aux += (strlen(aux) + 1);
		strcpy(aux, auxTeam.leagueName);
		aux += (strlen(aux) + 1);
		strcpy(aux, itoa(itoaAux, auxTeam.cantPlayers));
		aux += (strlen(aux) + 1);
		j = 0;
		while ( j < auxTeam.cantPlayers ) {
			strcpy(aux, auxTeam.players[j].name);
			aux += (strlen(aux) + 1);
			strcpy(aux, itoa(itoaAux, auxTeam.players[j].playerID));
			aux += (strlen(aux) + 1);
			strcpy(aux, itoa(itoaAux, auxTeam.players[j].score));
			aux += (strlen(aux) + 1);
			j++;
		}
		strcpy(aux, itoa(itoaAux, auxTeam.points));
		aux += (strlen(aux) + 1);
		i++;
	}
	strcpy(aux, "");
	return aux - buffer + 1;
}

static int
process_joinLeague(char * buffer, int qty) {
	char userName[31], name[31], * aux = buffer, itoaAux[10];
	ID leagueID;
	int ret;
	strcpy(userName, aux);
	aux += (strlen(aux) + 1);
	leagueID = atoi(aux);
	aux += (strlen(aux) + 1);
	strcpy(name, aux);
	aux += (strlen(aux) + 1);
	ret = joinLeague(userName, leagueID, name);
	strcpy(buffer, itoa(itoaAux, ret));
	return strlen(buffer) + 1;
}

static int
process_getTrades(char * buffer, int qty) {
	char userName[31], * aux = buffer, itoaAux[10];
	TradesSnd * ret;
	int i = 0, j;
	strcpy(userName, buffer);
	ret = getTrades(userName);
	while (  *(ret[i].team) != '\0' ) {
		strcpy(aux, ret[i].team);
		aux += (strlen(aux) + 1);
		strcpy(aux, itoa(itoaAux, ret[i].size));
		aux += (strlen(aux) + 1);
		j = 0;
		while ( j < ret[i].size ) {
			strcpy(aux, ret[i].trades[j].name);
			aux += (strlen(aux) + 1);
			strcpy(aux, itoa(itoaAux, ret[i].trades[j].id));
			aux += (strlen(aux) + 1);
			j++;
		}
		i++;		
	}
	strcpy(aux, "");
	return aux - buffer + 1;
}

static int
process_getTeam(char * buffer, int qty){
	char * aux = buffer, itoaAux[10];
	int teamID = atoi(buffer), j;
	TeamSnd ret = getTeam(teamID);
	strcpy(aux, ret.teamName);
	aux += (strlen(aux) + 1);
	strcpy(aux, ret.leagueName);
	aux += (strlen(aux) + 1);
	strcpy(aux, itoa(itoaAux, ret.cantPlayers));
	aux += (strlen(aux) + 1);
	j = 0;
	while ( j < ret.cantPlayers ) {
		strcpy(aux, ret.players[j].name);
		aux += (strlen(aux) + 1);
		strcpy(aux, itoa(itoaAux, ret.players[j].playerID));
		aux += (strlen(aux) + 1);
		strcpy(aux, itoa(itoaAux, ret.players[j].score));
		aux += (strlen(aux) + 1);
		j++;
	}
	strcpy(aux, itoa(itoaAux, ret.points));
	aux += (strlen(aux) + 1);
	return aux - buffer;
}

static int
process_removeTrade(char * buffer, int qty ) {
	ID tradeID;
	int ret;
	char itoaAux[10], username[31];
	strcpy(username, buffer);
	tradeID = atoi(buffer + strlen(buffer) + 1);
	ret = removeTrade(username, tradeID);
	strcpy(buffer, itoa(itoaAux, ret));
	return strlen(buffer) + 1;	
}

static int
process_negotiateTrade(char * buffer, int qty ) {
	ID tradeID, offeredPlayer, requestedPlayer;
	int ret;
	char itoaAux[10], * aux = buffer, username[31];
	strcpy(username, aux);
	aux += (strlen(aux) + 1);
	tradeID = atoi(aux);
	aux += (strlen(aux) + 1);
	offeredPlayer = atoi(aux);
	aux += (strlen(aux) + 1);
	requestedPlayer = atoi(aux);
	aux += (strlen(aux) + 1);
	ret = negotiateTrade(username, tradeID, offeredPlayer, requestedPlayer);
	strcpy(buffer, itoa(itoaAux, ret));
	return aux - buffer;	
}

static int
process_getTrade(char * buffer, int qty ) {
	ID tradeID;
	TradeSnd ret;
	char * aux = buffer, username[31];
	strcpy(username, buffer);
	tradeID = atoi(buffer + strlen(buffer) + 1);
	ret = getTrade(username, tradeID);
	strcpy(aux, ret.teamFrom);
	aux += (strlen(aux) + 1);
	strcpy(aux, ret.teamTo);
	aux += (strlen(aux) + 1);
	strcpy(aux, ret.playerFrom);
	aux += (strlen(aux) + 1);
	strcpy(aux, ret.playerTo);
	aux += (strlen(aux) + 1);
	return aux - buffer;	
}

static int
process_tradeAccept(char * buffer, int qty) {
	char userName[31], * aux = buffer, itoaAux[10];
	ID tradeID;
	int ret;
	strcpy(userName, aux);
	aux += (strlen(aux) + 1);
	tradeID = atoi(aux);
	aux += (strlen(aux) + 1);
	ret = tradeAccept(userName, tradeID);
	strcpy(buffer, itoa(itoaAux, ret));
	return strlen(buffer) + 1;
}

static int
process_createLeague(char * buffer, int qty) {
	char username[31], leagueName[31], teamName[31], *aux = buffer, auxItoa[10];
	int maxSize, ret;
	strcpy(username, aux);
	aux += (strlen(aux) + 1);
	strcpy(leagueName, aux);
	aux += (strlen(aux) + 1);
	strcpy(teamName, aux);
	aux += (strlen(aux) + 1);
	maxSize = atoi(aux);
	aux += (strlen(aux) + 1);
	ret = createLeague(username, leagueName, teamName, maxSize);
	strcpy(buffer, itoa(auxItoa, ret));
	return strlen(buffer);
}

static int
process_joinDraft(char * buffer, int qty) {
	char * aux = buffer, itoaAux[10];
	void * cl = malloc(200);
	ID teamID;
	int ret;
	teamID = atoi(aux);
	aux += (strlen(aux) + 1);
	stringToChannel(aux, cl); 
	ret = joinDraft(teamID, cl);
	strcpy(buffer, itoa(itoaAux, ret));
	return strlen(buffer) + 1;
}

static int
process_logOut(char * buffer, int qty) {
	char username[31];
	strcpy(username, buffer);
	logOut(username);
	return 0;
}


static int (*proc[])( char * buffer, int qty ) = {
	process_signUp, process_login, process_requestTrade, process_getLeaguesNames, process_getTeamsNames, process_getLeague, process_joinLeague, process_getTrades, process_getTeam, process_removeTrade, process_negotiateTrade, process_getTrade, process_tradeAccept, process_createLeague, process_joinDraft, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, process_logOut
};

static int
processServer( char * buffer, int qty ) {
	int opcode;
	opcode = atoi(buffer);
	qty = (*proc[opcode])(buffer+strlen(buffer)+1, qty);
	return qty;
}

void *
draft(void * leagueID) {
	char buffer[MAX_DATA], itoaAux[10], *aux;
	League * league = (League *)getFromHash(leagues, leagueID);
	Player * player;
	int size = league->maxSize, i, j, k, r, s, answers, flag1, flag2, chose;
	ID playerIDs[size][2], playersAux[MAX_PLAYERS], playerID;
	Draft * draftObj = (Draft *) getFromHash(drafts, leagueID);
	for ( i = 0 ; i < MAX_PLAYERS ; i++ ) {
		playersAux[i] = i;
	}
	for ( i = 0 ; i < size ; i++ ) {
		for ( j = 0 ; j < MAX_PLAYERS ; j++ ) {
			playerIDs[i][j] = -1;
		}
	}
	while ( draftObj->cant <  size ) {
	}
	while ( 1 ) {
		answers = 0;
		for( i = 0; i < size ; i++){
			if ( draftObj->array[i].pipe[0] != -1 ) {
				fcntl(draftObj->array[i].pipe[0], F_SETFL, O_NONBLOCK);
				strcpy(buffer, itoa(itoaAux, IS_ALIVE));
				pthread_mutex_lock(&(draftObj->draftMutex));
				if ( (draftObj->array)[i].channel != NULL ) {
					sleep(1);
					sendPacket(buffer, strlen(buffer)+1, (draftObj->array)[i].channel, SERVER );
				}
				pthread_mutex_unlock(&(draftObj->draftMutex));
				for ( k = 0 ; k < 10 ; k++ ) {
					if ( read(draftObj->array[i].pipe[0], buffer, MAX_DATA) > 0 ) { 
						answers++;
						break;
					}
					sleep(1);
				}
			}
		}
		if ( answers == size ) {
			break;
		}
	}
	for ( i = 0 ; i < 2 ; i++ ) {
		for ( j = 0 ; j < size  ; j++ ) {
			for ( k = 0, flag1 = 0, chose = 0 ; k < 10 ; k++ ) {
				pthread_mutex_lock(&(draftObj->draftMutex));
				if ( (draftObj->array)[j].channel == NULL ) {
					flag1 = 0;
				} else if ( !flag1 ) {
					flag1 = 1;
					Iterator * it = malloc(iteratorSize());
					listIteratorReset(league->nonDraftedPlayers, it);
					aux = buffer;
					strcpy(aux, itoa(itoaAux,CHOOSE_PLAYER));
					aux += (strlen(aux) + 1);
					for ( r = 0 ; r < MAX_PLAYERS ; r++ ) {
						if ( playersAux[r] != -1 ) {
							player = players[r];
							strcpy(aux, player->name);
							aux += (strlen(aux) + 1);
							strcpy(aux, itoa(itoaAux, player->playerID));
							aux += (strlen(aux) + 1);						
						}
					}
					strcpy(aux, "");
					sleep(1);	
					sendPacket(buffer, aux - buffer + 1, (draftObj->array)[j].channel, SERVER);
					free(it);
				}
				if ( read(draftObj->array[j].pipe[0], buffer, MAX_DATA) > 0 ) {
					playerID = atoi(buffer+strlen(buffer)+1);
					aux = buffer;
					strcpy(aux, itoa(itoaAux, CHOOSE_PLAYER_ANS));
					aux += (strlen(aux) + 1);
					flag2 = 0;	
					if ( playersAux[playerID] == -1 ) {
						strcpy(aux, "-1");
						aux += (strlen(aux) + 1);
						sleep(1);
						sendPacket(buffer, aux - buffer + 1, (draftObj->array)[j].channel, SERVER);
						flag2 = 1;
					}
					if ( !flag2 ) {
						playerIDs[j][i] = playerID;
						playersAux[playerID] = -1;
						chose = 1;
						strcpy(aux, "0");
						aux += (strlen(aux) + 1);
						sleep(1);
						sendPacket(buffer, aux - buffer + 1, (draftObj->array)[j].channel,SERVER);
						pthread_mutex_unlock(&(draftObj->draftMutex));
						break;
					}
				}
				sleep(1);
				pthread_mutex_unlock(&(draftObj->draftMutex));
			}
			if ( !chose ) {
				playerID = (rand() % (MAX_PLAYERS - (i*2 + j)));
				for ( r = 0, s = -1 ; r < MAX_PLAYERS ; r++ ) {
					if ( playersAux[r] != -1 ) {
						s++;
					}
					if ( playerID == s ) {
						playerID = r;
						playerIDs[j][i] = r;
						playersAux[r] = -1;
						chose = 1;
						aux = buffer;
						strcpy(aux, itoa(itoaAux, DRAFT_TIMEOUT));
						aux += (strlen(aux) + 1);
						strcpy(aux,  players[r]->name);
						aux += (strlen(aux) + 1);
						pthread_mutex_lock(&(draftObj->draftMutex));
						if ( (draftObj->array)[j].channel != NULL ) {
							sleep(1);
							sendPacket(buffer, aux - buffer + 1, (draftObj->array)[j].channel, SERVER);
							sleep(1);
						}
						pthread_mutex_unlock(&(draftObj->draftMutex));
						break;
					}
				}
			}
			chose = 0;
			aux = buffer;
			strcpy(aux, itoa(itoaAux, DRAFT_UPDATE));
			aux += (strlen(aux) + 1);
			strcpy(aux, (draftObj->array)[j].team->name);
			aux += (strlen(aux) + 1);
			strcpy(aux, players[playerID]->name);
			aux += (strlen(aux) +1);
			for ( r = 0 ; r < size ; r++ ) {
				if ( r != j ) {
					pthread_mutex_lock(&(draftObj->draftMutex));
					if ( (draftObj->array)[r].channel != NULL ) {
						sleep(1);
						sendPacket(buffer, aux - buffer + 1, (draftObj->array)[r].channel, SERVER);
					}
					pthread_mutex_unlock(&(draftObj->draftMutex));
				}
			}
		}
	}
	for ( r = 0 ; r < size ; r++ ) {
		for ( s = 0 ; s < 2 ; s++ ) {
			choosePlayer(playerIDs[r][s], (draftObj->array)[r].team->teamID);
		}
	}
	for ( r = 0 ; r < size ; r++ ) {
		strcpy(buffer, itoa(itoaAux, END_DRAFT));
		pthread_mutex_lock(&(draftObj->draftMutex));
		if ( (draftObj->array)[r].channel != NULL ) {
			sleep(1);
			sendPacket(buffer, strlen(buffer) + 1, (draftObj->array)[r].channel, SERVER);
		}
		pthread_mutex_unlock(&(draftObj->draftMutex));
	}	
	
	removeFromHash(drafts, &leagueID);
	freeDraft(draftObj);
	pthread_exit(NULL);
}

void *
expiryDateThread(void * a) {
	ListADT * tradesList;
	Trade * trade;
	while(1) {
		pthread_mutex_lock(&saveMutex);
		tradesList = hashValuesList(trades, cmpTrades);
		Iterator * it = malloc(iteratorSize());
		listIteratorReset(tradesList, it);
		while( (trade = listNext(tradesList, it)) != NULL ) {
			if ( time(NULL) - trade->date >= 0 ) {
				removeFromHash(trades, &(trade->tradeID));
				removeFromList( ((Team *)getFromHash(teams, &(trade->teamFrom)))->sentTrades, trade );
				removeFromList( ((Team *)getFromHash(teams, &(trade->teamTo)))->incomingTrades, trade );
				free(trade);
			}
		}
		free(it);
		pthread_mutex_unlock(&saveMutex);
		sleep(1);
	}
}

void *
playMatches(void * a) {
	struct dirent * d;
	DIR * dp;
	char newName[256], oldName[256];
	
	if ( (dp = opendir("../../matches")) == NULL ) {
		printf("Match directory doesn't exist. Program must abort\n");
		exit(-1);
	}
	while ( 1 ) {
		d = readdir(dp);
		if ( d != NULL ) {
			if ( d->d_ino != 0 && *(d->d_name) != '.' ) {
				strcpy(newName, "../../matches/.");
				strcpy(oldName, "../../matches/");
				strcat(newName, d->d_name);
				strcat(oldName, d->d_name);
				if ( playMatch(d->d_name) ) {
					printf("%s: match not played since file is corrupted.\n", d->d_name);
				}
				rename(oldName, newName);
			}
		} else {
			rewinddir(dp);
		}
		
	}
}

void *
saveThread(void * a) {
	
	while(1) {
		pthread_mutex_lock(&saveMutex);
		save();
		pthread_mutex_unlock(&saveMutex);
		sleep(1);

	}
	pthread_exit(NULL);
}

#define MAX_DATA 1024


int maxThreadID = -1;
int * TOArray;
jmp_buf * envs;
jmp_buf * envs2;


pthread_t * miniServers;

static void *
timeout(void * a) {
	int * localTOArray, i, flag = 0, size;
	localTOArray = malloc(0);
	while (1) {
		size = maxThreadID+1;
		localTOArray = realloc(localTOArray, size*sizeof(int));
		memcpy(localTOArray, TOArray, size*sizeof(int));
		sleep(60);
		for ( i = 0 ; i < size ; i++ ) {
			if ( TOArray[i] != -1 && TOArray[i] == localTOArray[i] ) {
				printf("La conexion de %d expiro\n", i);
				goto killMiniServer;
cont:
				longjmp(envs[i], 0);
cont1:				
				flag = 0;
			}
		}
	}
	//Hacer el salto multiple	
killMiniServer:
	setjmp(envs2[i]);
	if ( !flag ) {
		flag = 1;
		goto cont;		
	}
	pthread_cancel(miniServers[i]);
	TOArray[i] = -1;
	goto cont1;

}

static void *
miniServer(void * channels) {
	int qty, leagueID, opc, teamID, flag = 0, myID;
	char buffer[MAX_DATA], itoaAux[10], username[31];
	void * channel = malloc(200);
	Draft * draft;
	Team * team;
       	memcpy(channel, channels, 200);
	connectToChannel(channel, SERVER);
	myID = ++maxThreadID;
	goto exitMiniServer;
cont:
	while (1) {
		qty = receivePacket(buffer, sizeof(buffer), channel, SERVER);
		TOArray[myID]++;
		//Fijate si es un mensaje de draft. Si es join draft
		//inicializa leagueID como corresponde, y segui a process.
		//Para cualquier otro mensaje, si league->draft = 1 retornar
		//al cliente -1. Si no, derivar por el pipe.
		opc = atoi(buffer);
		if ( opc == LOGIN ) {
			strcpy(username, buffer+strlen(buffer)+1);
			qty = processServer(buffer, qty);
			sendPacket(buffer, qty + strlen(buffer) + 1, channel, SERVER);
		} else 	if ( opc == JOIN_DRAFT ) {
			teamID = atoi(buffer+strlen(buffer)+1);
			team = (Team *) getFromHash(teams, &teamID);
			if ( team == NULL ) {
				strcpy(buffer+strlen(buffer)+1, "-1");
				qty = 3;
			}
			else {
				int ret = joinDraft(teamID, channel);
				strcpy(buffer + strlen(buffer) + 1, itoa(itoaAux, ret));
				leagueID = team->leagueID;
				qty = 3;
			}
			sendPacket(buffer, qty + strlen(buffer) + 1, channel, SERVER);
		} else if ( opc == IS_ALIVE || opc == CHOOSE_PLAYER ) {
			draft = getFromHash(drafts, &leagueID);	
			write(draft->array[*(int*)getFromHash(draft->hash, &teamID)].pipe[1], buffer, MAX_DATA);
		} else if ( opc == QUIT_DRAFT ) {
			draft = getFromHash(drafts, &leagueID);
			pthread_mutex_lock(&(draft->draftMutex));
			sendPacket(buffer, qty, channel, SERVER);
			draft->array[*(int*)getFromHash(draft->hash, &teamID)].channel = NULL;
			pthread_mutex_unlock(&(draft->draftMutex));
		} else {
			qty = processServer(buffer, qty);
			sendPacket(buffer, qty + strlen(buffer) + 1, channel, SERVER);
		}
	}
exitMiniServer:
	setjmp(envs[myID]);
	if ( !flag ) {
		flag = 1;
		goto cont;		
	}
	strcpy(buffer, itoa(itoaAux, TIMEOUT));
	sendPacket(buffer, strlen(buffer)+1, channel, SERVER);
	disconnectFromChannel(channel);
	destroyChannel(channel);
	free(channel);
	longjmp(envs2[myID], 0);
	
}



#define MAX_PLAYERS 5
static void
shutDown(int a) {
	pthread_mutex_lock(&saveMutex);
	int i;
	printf("Server is shutting down\n");
	save();
	for ( i = 0 ; i < MAX_PLAYERS ; i++ ) {
		free(players[i]);
	}
	freeHashMap(trades);
	freeHashMap(teams);
	freeHashMap(leagues);
	freeHashMap(users);
	pthread_mutex_unlock(&saveMutex);
	exit(a);	
}


int
main( void ) {
	pthread_t savingThread, matchThread, tradesThread, timeoutThread;
	pthread_attr_t attr;
	pthread_mutex_init(&saveMutex,NULL);
	pthread_attr_init(&attr);
	
	static struct sigaction act1;
	static struct sigaction act2;
	act1.sa_handler = shutDown;
	sigfillset(&(act1.sa_mask));
	sigaction(SIGINT, &act1, NULL);
	act2.sa_handler = shutDown;
	sigfillset(&(act2.sa_mask));
	sigaction(SIGTSTP, &act2, NULL);
	users = newHashMap(hashString, cmpStrings, freeUser);
	leagues = newHashMap(hashInt, cmpInts, freeLeague);
	teams = newHashMap(hashInt, cmpInts, freeTeam);
	trades = newHashMap(hashInt, cmpInts, free);
	drafts = newHashMap(hashInt, cmpInts, freeDraft);
	envs = malloc(0);
	envs2 = malloc(0);
	TOArray = NULL;

	int rc, servers = 0;
	if ( load() ) {
		printf("No se pudo cargar\n");
		freeHashMap(trades);
		freeHashMap(teams);
		freeHashMap(leagues);
		freeHashMap(users);
		freeHashMap(drafts);
		return 1;
	}

	rc = pthread_create(&savingThread, NULL, saveThread, NULL); //Save no debe recibir nada...las variables que "recibe" son extern o global
	if ( rc ) {
		perror("Thread couldn't be created");
		exit(-1);
	}

	rc = pthread_create(&tradesThread, NULL, expiryDateThread, NULL);
	if ( rc ) {
		perror("Thread couldn't be created");
		exit(-1);
	}


	rc = pthread_create(&matchThread, NULL, playMatches, NULL); //playMatches esta buscando en el directorio "Partidos" y cuando encuentra uno lo juega.
	if  ( rc ) {
		perror("Thread couldn't be created");
		exit(-1);
	}	

	char buffer[200];
	void * defaultChannel = malloc(200), * newChannel = malloc(200);
	int qty;
	getDefaultChannel(defaultChannel);
	if ( createChannel(defaultChannel) == -1 ) {
		exit(-1);
	}
	connectToChannel(defaultChannel, SERVER);
	miniServers = (pthread_t*)malloc(0);
	while (1) {
		receivePacket(buffer, sizeof(buffer), defaultChannel, SERVER);
		getNextChannel(newChannel);
		if ( createChannel(newChannel) == -1 ) {
			exit(-1);
		}
		if ( servers % 10 == 0 ) {
			miniServers = realloc(miniServers, (servers + 10) * sizeof(pthread_t) );
			envs = realloc(envs, (servers+10)*sizeof(jmp_buf));
			envs2 = realloc(envs2, (servers+10)*sizeof(jmp_buf));
			TOArray = realloc(TOArray, (servers+10)*sizeof(int));
			int i;
			for ( i = servers ; i < servers + 10 ; i++ ) {	
				TOArray[i] = 0;
			}
			if ( servers == 0 ) { 
				rc = pthread_create(&timeoutThread, NULL, timeout, NULL);
				if  ( rc ) {
					perror("Thread couldn't be created");
					exit(-1);
				}	
			}
		}
		pthread_create(&miniServers[servers], NULL, miniServer, newChannel);
		servers++;
		qty = channelToString(buffer, newChannel);
		sendPacket(buffer, qty, defaultChannel, SERVER);
	}	

	if ( load() ) {
		printf("falla");
	}
	pthread_mutex_destroy(&saveMutex);
	pthread_cancel(savingThread);
	pthread_exit(NULL);
}
