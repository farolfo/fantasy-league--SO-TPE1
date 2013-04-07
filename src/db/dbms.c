#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "../includes/ListADT.h"
#include "../includes/clserv.h"
#include "../includes/server.h"
#include "../includes/HashMapADT.h"
#include "../includes/comparators.h"
#include "../includes/misc.h"

#define MAX_LENGTH	30
#define MAX_LENGHT	30
#define MAX_PLAYERS	5

extern HashMap * users;
extern HashMap * leagues;
extern HashMap * teams;
extern HashMap * trades;
extern Player * players[MAX_PLAYERS];

extern ID maxUserID;
extern ID maxLeagueID;
extern ID maxTeamID;
extern ID maxTradeID;


void cleanBuffer(char *,int);

static int
replacePipe(char * string) {
	string = strrchr(string, '|');
	if ( string == NULL ) {
		return -1;
	}
	*string = '\0';
	return 0;
}


int
load() {
	FILE * playersFile = fopen("../../database/Players", "r");
	if ( playersFile == NULL ) {
		return -1;
	}
	FILE * usersFile = fopen("../../database/Users", "rb");
	FILE * tradesFile = fopen("../../database/Trades", "rb");
	FILE * teamsFile = fopen("../../database/Teams", "rb");
	FILE * leaguesFile = fopen("../../database/Leagues", "rb");
		
	ID userID, teamID, leagueID, playerID;
	char nameBuffer[33],  passBuffer[31];
	int nteams, nplayers;
	int n, i, j, score, idArray[MAX_PLAYERS];
	Team * team;
	ListADT * playersList, * teamsList;
	League * league;
	User * user;
	Player * player;

	//nombre|   \n
	i = 0;
	while ( i < MAX_PLAYERS ) {
		players[i] =  malloc(sizeof(Player));
		if ( players[i] == NULL ) {
			perror("Malloc failed");
			for ( j = 0 ; j < i ; j++ ) {
				free(players[j]);
			}
			return -1;
		}
		players[i]->playerID = i;
		if ( fgets(nameBuffer, 33, playersFile) == NULL ) {
			return -1;
		}
		if ( replacePipe(nameBuffer) ) {
			return -1;
		}
		strcpy(players[i]->name, nameBuffer);
		players[i]->score = 0;
		i++;
	}
	if ( usersFile == NULL || teamsFile == NULL || tradesFile == NULL || leaguesFile == NULL ) {
		if ( usersFile == NULL && teamsFile == NULL && tradesFile == NULL && leaguesFile == NULL ) {
			usersFile = fopen("../../database/Users", "wb");
			tradesFile = fopen("../../database/Trades", "wb");
			teamsFile = fopen("../../database/Teams", "wb");
			leaguesFile = fopen("../../database/Leagues", "wb");
		} else {
			fclose(playersFile);
			if ( usersFile != NULL ) {
				fclose(usersFile);
			}
			if ( teamsFile != NULL ) {
				fclose(teamsFile);
			}
			if ( tradesFile != NULL ) {
				fclose(tradesFile);
			}
			if ( leaguesFile != NULL ) {
				fclose(leaguesFile);
			}

		
			for ( i = 0 ; i < MAX_PLAYERS ; i++ ) {
				free(players[i]);
			}
			return -1;
		}
	}


	
	// 5 id, 31 nombre (terminado en |), 5 league id, 2 can jug, players...
	
	while (1) {
		team = (Team *) malloc(sizeof(Team));
		if ( team == NULL ) {
			perror("Malloc failed");
			return -1;
		}
		playersList = newListADT(cmpPlayers, free);
		if ( playersList == NULL ) {
			free(team);
			return  -1;
		}
		n = fread(&(team->teamID), sizeof(int), 1, teamsFile); 
		if ( n != 1 ) {
			break;
		}
		if ( team->teamID >= maxTeamID ) {
			maxTeamID = team->teamID + 1;
		}
		fgets(nameBuffer, 31, teamsFile);
		strcpy(team->name, nameBuffer);
		fread(&(team->leagueID), sizeof(int), 1, teamsFile);
	       	fread (&(team->points), sizeof(int), 1, teamsFile);
	       	fread(&nplayers, sizeof(int), 1, teamsFile);
		i = 0;
		while ( i < nplayers ) {
			fread(&playerID, sizeof(int), 1, teamsFile);
			idArray[i] = playerID;
		//	add(playersList, players[playerID]);
			i++;
		}
		team->players = playersList;
		addToHash(teams, &(team->teamID), team);
		team->incomingTrades = newListADT(cmpTrades, free);
		team->sentTrades = newListADT(cmpTrades, free);
		league = getFromHash(leagues, &(team->leagueID));
		if ( league == NULL ) {
			league = (League *) malloc(sizeof(League));
			if ( league == NULL ) {
				perror("Malloc failed");
				return -1;
			}
			league->leagueID = team->leagueID;
			if ( league->leagueID >= maxLeagueID ) {
				maxLeagueID = league->leagueID + 1;
			}
			league->teams = newListADT(cmpTeams, freeTeam);
			if ( league->teams == NULL ) {
				return -1;
			}
			league->nonDraftedPlayers = (ListADT*)arrayToList((void **)players, cmpPlayers, free, MAX_PLAYERS, sizeof(Player)); // <--
			addToHash(leagues, &(league->leagueID), league);
		}
		add(league->teams, team);
		//listIteratorReset(team->players);
		//player = listNext(team->players);
		i = 0;
		while ( i < nplayers ) {
			player = removeFromList(league->nonDraftedPlayers, players[idArray[i]]);
			add(team->players, player);
			i++;
		}
	}
	Trade * trade;
	int auxBuffer[5];
	while (1) {
		trade = (Trade *) malloc(sizeof(Trade));
		n = fread(auxBuffer, sizeof(int), 5, tradesFile); 
		if ( n != 5 ) {
			break;
		}
		trade->tradeID = auxBuffer[0];
		if ( trade->tradeID >= maxTradeID ) {
			maxTradeID = trade->tradeID + 1;
		}
		trade->teamFrom = auxBuffer[1];
		trade->incomingPlayerID = auxBuffer[2];
		trade->wantedPlayerID = auxBuffer[3];
		trade->teamTo = auxBuffer[4];
		if ( fread(&(trade->date), sizeof(long), 1, tradesFile) != 1 ) {
			return -1;
		}	
		addToHash(trades, &(trade->tradeID), trade);
		add( ((Team *) getFromHash(teams, &(trade->teamFrom)))->sentTrades, trade);
		add( ((Team *) getFromHash(teams, &(trade->teamTo)))->incomingTrades, trade);
	}
	while (1) {
		n = fread(&leagueID, sizeof(int), 1, leaguesFile); 
		if ( n != 1 ) {
			break;
		}
		league = getFromHash(leagues, &leagueID);
		if ( league == NULL ) {
			return -1;
		}
		league->leagueID = leagueID;
		fgets(nameBuffer, 31, leaguesFile);
		strcpy(league->name, nameBuffer);
		fread(&(league->maxSize), sizeof(int), 1, leaguesFile);
		i = 0;
		while ( i < MAX_PLAYERS ) {
			n = fread(&score, sizeof(int), 1, leaguesFile);
			if ( players[i] == NULL || n != 1) {
				return -1;
			}
			Iterator * it = malloc(iteratorSize());
			listIteratorReset(league->teams, it);
			while ( ( team = listNext(league->teams, it) ) != NULL ) {
				if ( belongsToList(team->players, players[i]) ) {
					Iterator * it1 = malloc(iteratorSize());
					listIteratorReset(team->players, it1);
					while ( ( player = listNext(team->players, it1) ) != NULL ) {
						if ( !cmpPlayers(player, players[i]) ) {
							player->score = score;
							break;
						}
					}
					free(it1);
					break;
				}			
			}
			free(it);

			//(players[i])->score = score;
			i++;
		}
	}
	

	// 5 id, 31 nombre (terminado en |), 31 pass (idem), 2 cant eqiupos, 5xcantequipos.\n

	while (1) {
		user = (User *) malloc(sizeof(User));
		if ( user == NULL ) {
			perror("Malloc failed");
			return -1;
		}
		n = fread(&userID, sizeof(int), 1, usersFile); 
		if ( n != 1 ) {
			break;
		}
		user->userID = userID;		
		if ( userID >= maxUserID ) {
			maxUserID = userID + 1;
		}
		if ( fgets(nameBuffer, 31, usersFile) == NULL || fgets(passBuffer, 31, usersFile) == NULL ) {
			free(user);
			return -1;
		}
		strcpy(user->name, nameBuffer);
		strcpy(user->password, passBuffer);
		n = fread(&nteams, sizeof(int), 1, usersFile);
		if ( n != 1 ) {
			free(user);
			return -1;
		}
		teamsList = newListADT(cmpTeams, freeTeam);
		if ( teamsList == NULL ) {
			free(user);
			return -1;
		}
		for ( i = 0 ; i < nteams ; i++ ) {
			n = fread(&teamID, sizeof(int), 1, usersFile);
			if ( n != 1 ) {
				return -1;
			} 
			add(teamsList, getFromHash(teams, &teamID));
		}
		user->teams = teamsList;
		user->isConnected = 0;
		addToHash(users, user->name, user);
	}

	

	fclose(usersFile);
	fclose(tradesFile);
	fclose(playersFile);
	fclose(teamsFile);
	fclose(leaguesFile);

	return 0;
}

int//TODO ARREGLAR LOQ  ROMKPIO JORGE
save(){
	/*mutex*/
	remove("../../database/Teams");
	remove("../../database/Trade_Expire");
	remove("../../database/Users");
	remove("../../database/Trades");
	remove("../../database/Leagues");

	FILE * fteams = fopen("../../database/Teams","wb");
	//FILE * ftrade_expire = fopen("../../database/Trade_Expire","wb");
	FILE * ftrades = fopen("../../database/Trades","wb");
	FILE * fusers = fopen("../../database/Users","wb");
	FILE * fleagues = fopen("../../database/Leagues","wb");
	
	ListADT * leaguesList = hashValuesList(leagues, cmpLeagues);
	ListADT * usersList = hashValuesList(users, cmpUsers);
	
	char  nameBuffer[MAX_LENGTH+1], passBuffer[MAX_LENGTH+1];
	int i = 0;
	int size;
	int IDAux;

	/*---Guardado de Users---*/
	Iterator * it = malloc(iteratorSize());
	listIteratorReset(usersList, it);
	User * userAux = (User *)listNext(usersList, it);
	while( userAux != NULL ){
		cleanBuffer(nameBuffer,MAX_LENGHT+1); 
		strcpy(nameBuffer,userAux->name);
	//	strcat(nameBuffer,"|");

		cleanBuffer(passBuffer,MAX_LENGHT+1);
		strcpy(passBuffer,userAux->password);
		//strcat(passBuffer,"|");
				
		fwrite(&(userAux->userID),sizeof(int),1,fusers);
		fwrite(nameBuffer,sizeof(char),30,fusers);
		fwrite(passBuffer,sizeof(char),30,fusers);
		size = getSize(userAux->teams);
		fwrite(&size,sizeof(int),1,fusers);
		Iterator * it1 = malloc(iteratorSize());
		listIteratorReset(userAux->teams, it1);	
		for(i=0; i < getSize(userAux->teams); i++){
			IDAux = ((Team*)listNext(userAux->teams, it1))->teamID;
			fwrite(&IDAux,sizeof(int),1,fusers);
		}
		free(it1);
		userAux = (User *)listNext(usersList, it);
	}
	/*---Guardado de Leagues---*/
	listIteratorReset(leaguesList, it);
	League * leagueAux = (League *)listNext(leaguesList, it);
	Team * taux;
	Player * paux;
	int j;

	while(leagueAux != NULL){
		cleanBuffer(nameBuffer,MAX_LENGHT+1);
		strcpy(nameBuffer,leagueAux->name);
		//strcat(nameBuffer,"|");
				
		fwrite(&(leagueAux->leagueID),sizeof(int),1,fleagues);
	
		fwrite(nameBuffer,sizeof(char),30,fleagues);
		//size = getSize(leagueAux->teams);
		fwrite(&leagueAux->maxSize,sizeof(int),1,fleagues);
		
		long pivot = ftell(fleagues);

		//Agrego los jugadores de cada equipo
		Iterator * it1 = malloc(iteratorSize());
		listIteratorReset(leagueAux->teams, it1);
		for( i=0; i < getSize(leagueAux->teams); i++){
			taux = (Team *)listNext(leagueAux->teams, it1);
			Iterator * it2 = malloc(iteratorSize());
			listIteratorReset(taux->players, it2);
			for( j=0; j < getSize(taux->players); j++){
				paux = (Player *)listNext(taux->players, it2);
				fseek(fleagues,pivot+sizeof(int)*(paux->playerID),SEEK_SET);
				fwrite(&(paux->score),sizeof(int),1,fleagues);
			}
			free(it2);
		}
		
		//Agrego los jugadores que no tienen equipo
		listIteratorReset(leagueAux->nonDraftedPlayers, it1);
		paux = (Player *) listNext(leagueAux->nonDraftedPlayers, it1);
		while(paux != NULL){
			fseek(fleagues,pivot+sizeof(int)*(paux->playerID),SEEK_SET);
			fwrite(&(paux->score),sizeof(int),1,fleagues);
			paux = (Player *) listNext(leagueAux->nonDraftedPlayers, it1);			
		}
		free(it1);

		fseek(fleagues,pivot+sizeof(int)*MAX_PLAYERS,SEEK_SET);

		leagueAux = (League *)listNext(leaguesList, it);
	}
	/*---Guardado de Teams---*/
	listIteratorReset(leaguesList, it);
	leagueAux = (League *)listNext(leaguesList, it);
	
	while(leagueAux != NULL){
		Iterator * it1 = malloc(iteratorSize());
		listIteratorReset(leagueAux->teams, it1);
		taux = (Team *)listNext(leagueAux->teams, it1);
		while(taux != NULL){
			cleanBuffer(nameBuffer,MAX_LENGHT+1);
			strcpy(nameBuffer,taux->name);
		//	strcat(nameBuffer,"|");		

			size = getSize(taux->players);
			fwrite(&(taux->teamID),sizeof(int),1,fteams);
			fwrite(nameBuffer,sizeof(char),30,fteams);
			fwrite(&(taux->leagueID),sizeof(int),1,fteams);
			fwrite(&(taux->points),sizeof(int),1,fteams);
			fwrite(&size,sizeof(int),1,fteams);
			Iterator * it2 = malloc(iteratorSize());
			listIteratorReset(taux->players, it2);
			paux = (Player *)listNext(taux->players, it2);
			while(paux != NULL){
				fwrite(&(paux->playerID),sizeof(int),1,fteams);
				paux = (Player *)listNext(taux->players, it2);
			}
			free(it2);
			taux = (Team *)listNext(leagueAux->teams, it1);
		}
		free(it1);
		leagueAux = (League *)listNext(leaguesList, it);
	}
	/*---Guardado de Trades y Trade_Expire---*/
	listIteratorReset(leaguesList, it);
	leagueAux = (League *)listNext(leaguesList, it);
	Trade * tradeAux;
	while(leagueAux != NULL){
		Iterator * it1 = malloc(iteratorSize());
		listIteratorReset(leagueAux->teams, it1);
		taux = (Team *)listNext(leagueAux->teams, it1);
		while(taux != NULL){
			Iterator * it2 = malloc(iteratorSize());
			listIteratorReset(taux->incomingTrades, it2);
			tradeAux = (Trade *)listNext(taux->incomingTrades, it2);
			while(tradeAux != NULL){
				fwrite(&(tradeAux->tradeID),sizeof(int),1,ftrades);
				fwrite(&(tradeAux->teamFrom),sizeof(int),1,ftrades);
				fwrite(&(tradeAux->incomingPlayerID),sizeof(int),1,ftrades);
				fwrite(&(tradeAux->wantedPlayerID),sizeof(int),1,ftrades);
				fwrite(&(taux->teamID),sizeof(int),1,ftrades);
				fwrite(&(tradeAux->date),sizeof(long),1,ftrades);

				tradeAux = (Trade *)listNext(taux->incomingTrades, it2);
			}
			free(it2);
			taux = (Team *)listNext(leagueAux->teams, it1);
		}
		free(it1);
		leagueAux = (League *)listNext(leaguesList, it);
	}
	free(it);
	shallowFreeList(usersList);
	shallowFreeList(leaguesList);
	fclose(fteams);
//	fclose(ftrade_expire);
	fclose(ftrades);
	fclose(fusers);
	fclose(fleagues);
	
	return 0;
}

void
cleanBuffer(char * buf, int lenght){
	int i;
	for( i=0; i<lenght; i++){
		buf[i] = 0;
	}
}

/**
 *	Funcion de test de save
 */
/*
int 
main(){
	User u1 = {"Jorge","GAY",0, NULL,0};
	u1.teams = newListADT(NULL);
	User u2 = {"Pepe Argento","caramelo",1, NULL,0};
	u2.teams = newListADT(NULL);

	ListADT * vecUser = newListADT(NULL);
	add(vecUser,&u1);
	add(vecUser,&u2);

	Player p1 = {"Jorge Mozzino",0,10};
	Player p2 = {"Jorge Mozzine",1,30};
	Player p3 = {"Jorge Mozzinu",2,20};
	Player p4 = {"Jorge Mozzini",3,15};
	Player p5 = {"Jorge Mozzina",4,18};


	ListADT * myPlayers = newListADT(NULL);
	ListADT * nonDrafted = newListADT(NULL);
	add(myPlayers,&p1);
	add(myPlayers,&p2);
	add(nonDrafted,&p3);
	add(nonDrafted,&p4);
	add(nonDrafted,&p5);
	
	ListADT * teams = newListADT(NULL);
	ListADT * inTr = newListADT(NULL);
	ListADT * snTr = newListADT(NULL);

	Team myTeam = {"SCONES DE LA CHOLA",15,myPlayers,1,inTr,snTr,12};
	add(u1.teams,&myTeam);	
	add(teams,&myTeam);
	League l1 = {"Liga MALUKO",1,4,teams,nonDrafted};
	ListADT * lligs = newListADT(NULL);
	add(lligs,&l1);
	save(vecUser,lligs);
}*/
