#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>


#include "../includes/ListADT.h"
#include "../includes/clserv.h"
#include "../includes/server.h"
#include "../includes/HashMapADT.h"
#include "../includes/comparators.h"
#include "../includes/dbms.h"
#include "../includes/misc.h"
#include "../includes/physic.h"

#include "../includes/client.h"

#define MAX_PLAYERS 5

HashMap * users;
HashMap * leagues;
HashMap * teams;
HashMap * trades;
HashMap * drafts;

Player * players[MAX_PLAYERS];

ID maxUserID = 0;
ID maxLeagueID = 0;
ID maxTeamID = 0;
ID maxTradeID = 0;

pthread_mutex_t saveMutex;
//Debe haberse hecho primero el free de teams!
void
freeUser(void * user) {
	shallowFreeList(((User*)user)->teams);
	free(user);
}

void
freeDraft(void * draft) {
	Draft * d = (Draft *) draft;
	freeHashMap(d->hash);
	free(d->array);
	free(d);
}

int
signUp(char * username, char * password) {
	pthread_mutex_lock(&saveMutex);
	User * user = (User *) malloc(sizeof(User));
	if ( user == NULL ) {
		perror("Malloc failed");
		return -1;
	}
	strcpy(user->name, username);
	strcpy(user->password, password);
	user->userID = maxUserID++;
	user->isConnected = 1;
	int ret = addToHash(users, user->name, user);
	switch ( ret ) {
		case -1: case 1:
			free(user);
			pthread_mutex_unlock(&saveMutex);
			return ret; break;
		default:
			user->teams = newListADT(cmpTeams, freeTeam);
			pthread_mutex_unlock(&saveMutex);
			return ret; break;
	}

}

int
login(char * username, char * password) {
	pthread_mutex_lock(&saveMutex);
	User * user = (User *) getFromHash(users, username);
	if ( user == NULL ) {
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	if ( user->isConnected ) {
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	int ret = strcmp(user->password, password);
	if (ret == 0) {
		user->isConnected = 1;
	}
	pthread_mutex_unlock(&saveMutex);
	return ret;
}

void
logOut(char * username) {
	pthread_mutex_lock(&saveMutex);
	User * user = (User *) getFromHash(users, username);
	if ( user == NULL ) {
		pthread_mutex_unlock(&saveMutex);
		return;
	}
	user->isConnected = 0;
	pthread_mutex_unlock(&saveMutex);
}

static Team *
createTeam(char * name) {
	Team * team = (Team *) malloc(sizeof(Team));
	if ( team == NULL ) {
		return NULL;
	}
	strcpy(team->name, name);
	team->teamID = maxTeamID++;
	team->players = newListADT(cmpPlayers, free);
	team->incomingTrades = newListADT(cmpTrades, free);
	team->sentTrades = newListADT(cmpTrades, free);
	team->points = 0;
	return team;
}

int
createLeague(char userName[31], char leagueName[31], char teamName[31], int maxSize) {
	pthread_mutex_lock(&saveMutex);
	League * league = (League *) malloc(sizeof(League));
	User * user;
	Team * team;
	if ( league == NULL ) {
		perror("Malloc failed");
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	user = getFromHash(users, userName);
	if ( user == NULL ) {
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	strcpy(league->name, leagueName);
	league->leagueID = maxLeagueID++;
	league->teams = newListADT(cmpTeams, freeTeam);
	if ( league->teams == NULL ) {
		free(league);
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	league->maxSize = maxSize;
	team = createTeam(teamName);
	team->leagueID = league->leagueID;
	add(league->teams, team);
	league->nonDraftedPlayers = arrayToList((void**)players, cmpPlayers, free, MAX_PLAYERS, sizeof(Player));
	if ( league->nonDraftedPlayers == NULL ) {
		freeAll(2,league->teams, league);
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	addToHash(leagues, &(league->leagueID), league);
	addToHash(teams, &(team->teamID), team);
	add(user->teams, team);
	pthread_mutex_unlock(&saveMutex);
	return 0;
}

int
choosePlayer(ID playerID, ID teamID) {
	pthread_mutex_lock(&saveMutex);
	Team * team = (Team *) getFromHash(teams, &teamID);
	League * league  = getFromHash(leagues, &(team->leagueID));
	Player * player = players[playerID], * playerAux;
	if ( ( playerAux = removeFromList(league->nonDraftedPlayers, player) ) == NULL ) {
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	add(team->players, playerAux);
	pthread_mutex_unlock(&saveMutex);
	return 0;
}

static int
requestTradeAux(char username[31], ID teamIDFrom, ID teamIDTo, ID offeredPlayer, ID requestedPlayer) {
	User * user = getFromHash(users, username);
	Team * from = (Team *) getFromHash(teams, &teamIDFrom);
	Team * to = (Team *) getFromHash(teams, &teamIDTo);
	Player * player;
	if ( from == NULL ) {
		return -1;
	}
	if ( teamIDFrom == teamIDTo ) {
		return -1;
	}
	League * league = (League *) getFromHash(leagues, &(from->leagueID));
	if ( belongsToList( league->nonDraftedPlayers, players[requestedPlayer] ) ) {
		player = removeFromList(from->players, players[offeredPlayer]);
		add(league->nonDraftedPlayers, player);
		player = removeFromList(league->nonDraftedPlayers, players[requestedPlayer]);
		add(from->players, player);
		pthread_mutex_unlock(&saveMutex);
		return 0;		
	}
	if ( to == NULL ) {
		return -1;
	}
	if ( !belongsToList(user->teams, from) || !belongsToList(from->players, players[offeredPlayer])  || ! belongsToList(to->players, players[requestedPlayer]) || from->leagueID != to->leagueID ) {
		return -1;
	}
	Trade * trade = (Trade *) malloc(sizeof(Trade));
	if ( trade == NULL ) {
	}
	trade->tradeID = maxTradeID++;
	trade->teamFrom = teamIDFrom;
	trade->teamTo = teamIDTo;
	trade->wantedPlayerID = requestedPlayer;
	trade->incomingPlayerID = offeredPlayer;
	trade->date = time(NULL) + 24*3600; // En un dia vence el trade...
	add(from->sentTrades,  trade);
	add(to->incomingTrades, trade);
	addToHash(trades, &(trade->tradeID), trade);
	return 0;
}





NmsID * //League names structure 
getLeaguesNames(){
	pthread_mutex_lock(&saveMutex);
	ListADT * leaguesList = hashValuesList(leagues, cmpLeagues);
	League * league;
	NmsID * ans = (NmsID *) malloc((getSize(leaguesList)+1)*sizeof(NmsID));
	Iterator * it = malloc(iteratorSize());
	listIteratorReset(leaguesList, it);
	int i = 0;
	while( (league = listNext(leaguesList, it)) != NULL ) {
		strcpy(ans[i].name, league->name);
	       	ans[i].id = league->leagueID;	
		i++;
	}
	free(it);
	ans[i].id = -1;
	pthread_mutex_unlock(&saveMutex);
	return ans;
}

NmsID *
getTeamsNames(){
	pthread_mutex_lock(&saveMutex);
	ListADT * leaguesList = hashValuesList(leagues, cmpLeagues);
	League * league;
	Team * team;
	int size = 0, i = 0;
	NmsID * ans = NULL;
	Iterator * it = malloc(iteratorSize());
	listIteratorReset(leaguesList, it);
	while((league = listNext(leaguesList, it)) != NULL){
		Iterator * it1 = malloc(iteratorSize());
		listIteratorReset(league->teams, it1);
		size += (getSize(league->teams) * sizeof(NmsID));
		ans = realloc(ans, size); 
		while((team = listNext(league->teams, it1)) != NULL){
			strcpy(ans[i].name,team->name);
			ans[i].id = team->teamID;
			i++;
		}
		free(it1);
	}
	free(it);
	ans = realloc(ans, size+sizeof(NmsID));
	ans[i].id = -1;	
	pthread_mutex_unlock(&saveMutex);
	return ans;
}

LeagueSnd
getLeague(ID leagueID) {
	pthread_mutex_lock(&saveMutex);
	League * league = getFromHash(leagues, &leagueID);
	if ( league == NULL ) {
		LeagueSnd * ans = (LeagueSnd *) malloc(sizeof(LeagueSnd));
		strcpy(ans->name, "");
		pthread_mutex_unlock(&saveMutex);
		return *ans;
	}
	LeagueSnd * ans = (LeagueSnd *) malloc(sizeof(LeagueSnd));
	ans->teams = (TeamSnd *) malloc(sizeof(TeamSnd)*(1+getSize(league->teams)));
	strcpy(ans->name, league->name);
	Team * team;
	int i = 0, j;
	Iterator * it = malloc(iteratorSize());
	listIteratorReset(league->teams, it);
	Player * player;
	while ( (team = listNext(league->teams, it)) != NULL ) {
		j = 0;
		strcpy((ans->teams)[i].teamName, team->name);
		strcpy((ans->teams)[i].leagueName, ((League*)getFromHash(leagues, &leagueID))->name);
		(ans->teams)[i].cantPlayers = getSize(team->players);
		(ans->teams)[i].points = team->points;
		Iterator * it1 = malloc(iteratorSize());
		listIteratorReset(team->players, it1);
		while ( (player = listNext(team->players, it1)) != NULL ) {
			(ans->teams)[i].players[j] = *player;
			j++;
		}
		i++;
		free(it1);
	}
	free(it);
	ans->teams[i].points = -1;
	pthread_mutex_unlock(&saveMutex);
	return *ans;
}



int
joinLeague(char userName[31], ID leagueID, char * name) {
	pthread_mutex_lock(&saveMutex);
	League * league = getFromHash(leagues, &leagueID);
	if ( league == NULL ) {
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	if ( getSize(league->teams) == league->maxSize ) {
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	User * user = getFromHash(users, userName);
	Team * team;
	Iterator * it = malloc(iteratorSize());
	listIteratorReset(user->teams, it);
	while ( ( team = listNext(user->teams, it) ) != NULL ) {
		if ( team->leagueID == leagueID ) {
			pthread_mutex_unlock(&saveMutex);
			return -1;
		}
	}
	listIteratorReset(league->teams, it);
	while ( (team = listNext(league->teams, it) ) != NULL ) {
		if ( !strcmp(team->name, name) ) {
			pthread_mutex_unlock(&saveMutex);
			return -1;
		}
	}
	free(it);
	team = createTeam(name);

	if (user == NULL) {
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	add(user->teams, team);
	add(league->teams, team);
	addToHash(teams, &(team->teamID), team);
	team->leagueID = leagueID;
	pthread_mutex_unlock(&saveMutex);
	return 0;
}



TradesSnd *
getTrades(char userName[31]) {
	pthread_mutex_lock(&saveMutex);
	User * user = getFromHash(users, userName);
	if ( user == NULL ) {
		TradesSnd * ret = (TradesSnd *) malloc(sizeof(TradesSnd));
		strcpy(ret[0].team, "");
		pthread_mutex_unlock(&saveMutex);
		return NULL;
	}
	Team * team;
	Trade * trade;
	TradesSnd * ans = (TradesSnd *) malloc((1+getSize(user->teams))*sizeof(TradesSnd));
	int size = 0, i = 0, j;
	Iterator * it = malloc(iteratorSize());
	listIteratorReset(user->teams, it);
	while ( (team = listNext(user->teams, it)) != NULL ) {
		strcpy(ans[i].team, team->name);
		j = 0;
		size = getSize(team->incomingTrades)+getSize(team->sentTrades)+1;
		ans[i].size = size;
		ans[i].trades = malloc(sizeof(NmsID)*size);
		Iterator * it1 = malloc(iteratorSize());
		listIteratorReset(team->incomingTrades, it1);
		while ( (trade = listNext(team->incomingTrades, it1)) != NULL ) {
			strcpy(ans[i].trades[j].name, ((Team*)getFromHash(teams, &(trade->teamFrom)))->name);
			strcat(ans[i].trades[j].name, " -> ");
			strcat(ans[i].trades[j].name, team->name);
			ans[i].trades[j].id = trade->tradeID;
			j++;
		}
		listIteratorReset(team->sentTrades, it1);
		while ( (trade = listNext(team->sentTrades, it1)) != NULL ) {
			strcpy(ans[i].trades[j].name, team->name);
			strcat(ans[i].trades[j].name, " -> ");
			strcat(ans[i].trades[j].name, ((Team*)getFromHash(teams, &(trade->teamTo)))->name);
			ans[i].trades[j].id = trade->tradeID;
			j++;
		}
		free(it1);
		strcpy(ans[i].trades[j].name, "");
		ans[i].trades[j].id = -1;
		i++;
	}
	strcpy(ans[i].team, "");
	pthread_mutex_unlock(&saveMutex);
	return ans;
}

TeamSnd
getTeam(ID teamID) {
	pthread_mutex_lock(&saveMutex);
	Team * team = getFromHash(teams, &teamID);
	TeamSnd * ans = (TeamSnd *) malloc(sizeof(TeamSnd));
	Player * player;
	int i;
	if ( team == NULL ) {
		ans->points = -1;
		pthread_mutex_unlock(&saveMutex);
		return *ans;
	}
	strcpy(ans->teamName, team->name);
	strcpy(ans->leagueName, ((League *)getFromHash(leagues, &(team->leagueID)))->name);
	ans->cantPlayers = getSize(team->players);
	ans->points = team->points;
	Iterator * it = malloc(iteratorSize());
	listIteratorReset(team->players, it);
	i = 0;
	while ( (player = listNext(team->players, it)) != NULL ) {
		ans->players[i] = *player;
		i++;
	}
	pthread_mutex_unlock(&saveMutex);
	return *ans;
}


TradeSnd
getTrade(char username[31], ID tradeID) {
	pthread_mutex_lock(&saveMutex);
	Trade * trade = getFromHash(trades, &tradeID);
	User * user = getFromHash(users, username);
	TradeSnd * ret = malloc(sizeof(TradeSnd));
	if ( trade == NULL ) {
		strcpy(ret->teamFrom, "");
		pthread_mutex_unlock(&saveMutex);
		return *ret;
	}
	Team * teamFrom = getFromHash(teams, &(trade->teamFrom)), * teamTo = getFromHash(teams, &(trade->teamTo));
	if ( ( !belongsToList(user->teams, teamFrom) && !belongsToList(user->teams, teamTo) ) || teamFrom == NULL || teamTo == NULL || trade->incomingPlayerID >= MAX_PLAYERS || trade->wantedPlayerID >= MAX_PLAYERS )  {
		strcpy(ret->teamFrom, "");
		pthread_mutex_unlock(&saveMutex);
		return *ret;
	}
	strcpy(ret->teamFrom,  teamFrom->name);
	strcpy(ret->teamTo, teamTo->name);
	strcpy(ret->playerFrom, players[trade->incomingPlayerID]->name);
	strcpy(ret->playerTo, players[trade->wantedPlayerID]->name);
	pthread_mutex_unlock(&saveMutex);
	return *ret;
}


static int
removeTradeAux(char username[31], ID tradeID) {
	Trade * trade = getFromHash(trades, &tradeID);
	if ( trade == NULL ) {
		return -1;
	}
	User * user = getFromHash(users, username);
	Team * from = (Team *)getFromHash(teams, &(trade->teamFrom)), * to = (Team *)getFromHash(teams, &(trade->teamTo));
	if ( !belongsToList(user->teams, from) && !belongsToList(user->teams, to) ) {
		return -1;
	}
	removeFromHash(trades, &(trade->tradeID));
	removeFromList( from->sentTrades, trade );
	removeFromList( to->incomingTrades, trade );
	free(trade);
	return 0;
}

int
requestTrade(char username[31], ID teamIDFrom, ID teamIDTo, ID offeredPlayer, ID requestedPlayer) {
	int ret;
	pthread_mutex_lock(&saveMutex);
	ret = requestTradeAux(username, teamIDFrom, teamIDTo, offeredPlayer, requestedPlayer);
	pthread_mutex_unlock(&saveMutex);
	return ret;
}

int
removeTrade(char username[31], ID tradeID) {
	int ret;
	pthread_mutex_lock(&saveMutex);
	ret = removeTradeAux(username, tradeID);
	pthread_mutex_unlock(&saveMutex);
	return ret;
}



int
negotiateTrade(char username[31], ID tradeID, ID offeredPlayer, ID requestedPlayer) {
	pthread_mutex_lock(&saveMutex);
	Trade * trade = getFromHash(trades, &tradeID);
	if ( trade == NULL ) {
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	User * user = getFromHash(users, username);
	Team * to = getFromHash(teams, &(trade->teamTo));
	if ( !belongsToList(user->teams, to) ) {
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	requestTradeAux(username, trade->teamTo, trade->teamFrom, offeredPlayer, requestedPlayer);
	removeTradeAux(username, tradeID);
	pthread_mutex_unlock(&saveMutex);
	return 0;
}

int
tradeAccept(char username[31], ID tradeID) {
	pthread_mutex_lock(&saveMutex);
	Trade * trade = getFromHash(trades, &tradeID);
	User * user = getFromHash(users, username);
	Team * teamAux, * teamFrom, * teamTo;
	ID reqPlayer, offeredPlayer;
	Player * player;
	if ( trade == NULL ) {
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	teamFrom = getFromHash(teams, &(trade->teamFrom));
	teamTo = getFromHash(teams, &(trade->teamTo));
	reqPlayer = trade->wantedPlayerID;
	offeredPlayer = trade->incomingPlayerID;
	if (  !belongsToList(teamFrom->players, players[offeredPlayer]) || !belongsToList(teamTo->players, players[reqPlayer]) ) {
		pthread_mutex_unlock(&saveMutex);
		return -1;
	}
	Iterator * it = malloc(iteratorSize());
	listIteratorReset(user->teams, it);
	while ( ( teamAux = listNext(user->teams, it) ) != NULL ) {
		if ( removeFromList(teamAux->incomingTrades, trade) != NULL ) {
			if ( !belongsToList(user->teams, teamAux) ) {
				add(teamAux->incomingTrades, trade);
				free(it);
				pthread_mutex_unlock(&saveMutex);
				return -1;
			}
			player = removeFromList(teamAux->players, players[reqPlayer]);
			add(teamFrom->players, player);
			player = removeFromList(teamFrom->players, players[offeredPlayer]);
			add(teamAux->players, player);
			removeFromList(teamFrom->sentTrades, trade);
			removeFromHash(trades, &(trade->tradeID));
			free(trade);
			pthread_mutex_unlock(&saveMutex);
			return 0;
		}
	}
	free(it);
	pthread_mutex_unlock(&saveMutex);
	return -1;
}


//Quizas convenga fijarse si la variable drafted de leagueID esta seteada, entonces devolver -1 si es asi.
int
joinDraft(ID teamID, void * channel) {
	Draft * draftObj;
	int i = 0, * p;
	pthread_attr_t attr;
	Team * team = getFromHash(teams, &teamID), * teamAux;
	if ( team == NULL ) {
		return -1;
	}
	League * league = getFromHash(leagues, &(team->leagueID));
	if ( getSize(team->players) > 0 ) {
		return -1;
	}
	if ( getSize(league->teams) == league->maxSize ) {
		if ( ( draftObj = getFromHash( drafts, &(league->leagueID) ) ) == NULL ) {
			draftObj = ( Draft * ) malloc(sizeof(Draft));
			draftObj->cant = 0;
			pthread_mutex_init(&(draftObj->draftMutex),NULL);
			pthread_attr_init(&attr);
			draftObj->array = malloc( (league->maxSize)*sizeof(TeamCh) );
			draftObj->hash = newHashMap(hashInt, cmpInts, free);
			addToHash(drafts, &(league->leagueID), draftObj);
			Iterator * it = malloc(iteratorSize());
			listIteratorReset(league->teams, it);
			while ( ( teamAux = listNext(league->teams, it) ) != NULL ) {
				p = malloc(sizeof(int));
				*p = i;
				addToHash(draftObj->hash, &(teamAux->teamID), p);
				draftObj->array[i].channel = NULL;
				draftObj->array[i].pipe[0] = -1;
				draftObj->array[i].pipe[1] = -1;
				i++;
			}
			free(it);
			if ( pthread_create(&(draftObj->thread), NULL, draft, &(league->leagueID)) ) {
				perror("Thread not created\n");
				exit(-1);
			}
		}
		int index = *(int*)getFromHash(draftObj->hash, &(team->teamID));
		draftObj->array[index].team = team;
		draftObj->array[index].channel = malloc(200);
		memcpy(draftObj->array[index].channel,channel, 200);
		pipe(draftObj->array[index].pipe);
		draftObj->cant++;
		return 0;
	}
	return -1;//liga no esta llena
}

int
playMatch(const char * file) {
	int i, score[MAX_PLAYERS], teamPoints;
	char aux[30] = "../../matches/", buffer[10];
	strcat(aux, file);
	League * league;
	Team * team;
	Player * player;
	//Hacer que se sumen los puntajes a todos los jugadores de todos los equipos
	ListADT * leaguesList = hashValuesList(leagues, cmpLeagues);  
	FILE * match = fopen(aux, "r");
	for ( i = 0 ; i < MAX_PLAYERS ; i++ ) {
		if ( fgets(buffer, 10, match ) == NULL ) {
			fclose(match);
			return -1;
		}
		buffer[strlen(buffer)] = '\0';
		score[i] = atoi(buffer);
	}
	pthread_mutex_lock(&saveMutex);
	Iterator * it = malloc(iteratorSize());
	listIteratorReset(leaguesList, it);
	while ( ( league = listNext(leaguesList, it) ) != NULL ) {
		Iterator * it1 = malloc(iteratorSize());
		listIteratorReset(league->teams, it1);
		while ( ( team = listNext(league->teams, it1) ) != NULL ) {
			teamPoints = 0;
			Iterator * it2 = malloc(iteratorSize());
			listIteratorReset(team->players, it2);
			while ( ( player = listNext(team->players, it2) ) != NULL ) {
				player->score += score[player->playerID];
				teamPoints += score[player->playerID];
			}
			free(it2);
			team->points += teamPoints;
		}
		free(it1);
	}
	free(it);
	fclose(match);
	pthread_mutex_unlock(&saveMutex);
	return 0;
}

void
generateMatch(ID matchID) {
	char name[256], itoaAux[10];
	strcpy(name, "../../matches/");
	strcat(name, itoa(itoaAux, matchID));
	FILE * match = fopen(name, "wb");
	int i, score;
	for ( i = 0 ; i < MAX_PLAYERS ; i++ ) {
		score = (int)(rand() * 10);
		fwrite(&score, sizeof(int), 1, match);
	}
	fclose(match);
}

