#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../includes/ListADT.h"
#include "../includes/clserv.h"
#include "../includes/server.h"
#include "../includes/HashMapADT.h"

#define MAX_LENGTH 30
#define MAX_PLAYERS 5

static int
cmpUsers(void * u1, void * u2) {
	return strcmp(((User*)u1)->name, ((User*)u2)->name);
}

static int
cmpInts(void * i1, void * i2) {
	return *(int*)i1 - *(int*)i2;
}
static int
cmpTeams(void* t1, void * t2) {
	return strcmp(((Team*)t1)->name, ((Team*)t2)->name);
}
static int
cmpPlayers(void * p1, void * p2) {
	return ((Player *)p1)->playerID - ((Player*)p2)->playerID;
}

static int
hashInt(void * id) {
	return *(int *)id;
}

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
load(ListADT * users, ListADT * leagues, Player * players[MAX_PLAYERS]) {
	FILE * usersFile = fopen("../../database/Users", "rb");
	if (usersFile == NULL ) {
		return -1;
	}
//	FILE * tradesFile = fopen("../../database/Trades", "rb");
//	FILE * expiresFile = fopen("../../database/Trade_Expire", "rb");
	FILE * playersFile = fopen("../../database/Players", "r");
	if ( playersFile == NULL ) {
		fclose(usersFile);
		return -1;
	}
	FILE * teamsFile = fopen("../../database/Teams", "rb");
	if ( teamsFile == NULL ) {
		fclose(usersFile);
		fclose(playersFile);
		return -1;
	}
	FILE * leaguesFile = fopen("../../database/Leagues", "rb");
	if ( leaguesFile == NULL ) {
		fclose(usersFile);
		fclose(playersFile);
		fclose(teamsFile);
		return -1;
	}


		
	ID userID, teamID, leagueID, playerID;
	char * nameBuffer, * passBuffer = (char *) malloc(sizeof(char)*(MAX_LENGTH+2)), * aux;
	int nteams, nplayers;
	int n, i, j, score;
	Team * team;
	ListADT * playersList, * teamsList;
	HashMap * teams = newHashMap(hashInt, cmpInts), * leagues2 = newHashMap(hashInt, cmpInts);
	League * league;
	User * user;
	Player * player;

	//nombre|   \n
	i = 0;
	while ( i < MAX_PLAYERS ) {
		nameBuffer = (char *) malloc(sizeof(char)*(MAX_LENGTH+3));
		if ( nameBuffer == NULL ) {
			perror("Malloc failed");
			return -1;
		}
		players[i] = malloc(sizeof(Player));
		if ( players[i] == NULL ) {
			free(nameBuffer);
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
		players[i]->name = nameBuffer;
		players[i]->score = 0;
		i++;
	}

/*	for ( i = 0 ; i < MAX_PLAYERS ; i++ ) {

		printf("ID: %i\nNAME: %s\n", players[i]->playerID, players[i]->name);
	}*/
	
	// 5 id, 31 nombre (terminado en |), 5 league id, 2 can jug, players...
	
	while (1) {
		team = (Team *) malloc(sizeof(Team));
		if ( team == NULL ) {
			perror("Malloc failed");
			return -1;
		}
		nameBuffer = (char *) malloc(sizeof(char)*(MAX_LENGTH+2));
		if ( nameBuffer == NULL ) {
			free(team);
			perror("Malloc failed");
			return -1;
		}
		playersList = newListADT(cmpPlayers);
		if ( playersList == NULL ) {
			freeAll(2, &team, &nameBuffer);
			return  -1;
		}
		n = fread(&(team->teamID), sizeof(int), 1, teamsFile); 
		if ( n != 1 ) {
			break;
		}
		fgets(nameBuffer, 31, teamsFile);
		team->name = nameBuffer;
		fread(&(team->leagueID), sizeof(int), 1, teamsFile);
	       	fread (&(team->points), sizeof(int), 1, teamsFile);
	       	fread(&nplayers, sizeof(int), 1, teamsFile);
		i = 0;
		while ( i < nplayers ) {
			fread(&playerID, sizeof(int), 1, teamsFile);
			add(playersList, players[playerID]);
			i++;
		}
		team->players = playersList;
		//Cargar Trades!!
		addToHash(teams, &(team->teamID), team);
		league = getFromHash(leagues2, &(team->leagueID));
		if ( league == NULL ) {
			league = (League *) malloc(sizeof(League));
			if ( league == NULL ) {
				perror("Malloc failed");
				return -1;
			}
			league->leagueID = team->leagueID;
			league->teams = newListADT(cmpTeams);
			if ( league->teams == NULL ) {
				return -1;
			}
			league->nonDraftedPlayers = (ListADT*)arrayToList((void **)players, cmpPlayers, MAX_PLAYERS); // <--
			addToHash(leagues2, &(league->leagueID), league);
			add(leagues, league);
		}
		add(league->teams, team);
		listIteratorReset(team->players);
		player = listNext(team->players);
		while ( player != NULL ) {
			removeFromList(league->nonDraftedPlayers, player);
			player = listNext(team->players);
		}
	}
	while (1) {
		nameBuffer = (char *) malloc(sizeof(char)*(MAX_LENGTH+2));
		if ( nameBuffer == NULL ) {
			perror("Malloc failed");
			return -1;
		}
		n = fread(&leagueID, sizeof(int), 1, leaguesFile); 
		if ( n != 1 ) {
			break;
		}
		league = getFromHash(leagues2, &leagueID);
		if ( league == NULL ) {
			free(nameBuffer);
			return -1;
		}
		league->leagueID = leagueID;
		fgets(nameBuffer, 31, leaguesFile);
		league->name = nameBuffer;
		fread(&(league->maxSize), sizeof(int), 1, leaguesFile);
		i = 0;
		while ( i < MAX_PLAYERS ) {
			n = fread(&score, sizeof(int), 1, leaguesFile);
			if ( players[i] == NULL || n != 1) {
				return -1;
			}
			(players[i])->score = score;
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
		if ( (nameBuffer = (char *) malloc(sizeof(char)*(MAX_LENGTH+2))) == NULL ) {
			free(user);
			perror("Malloc failed");
			return -1;
		}
		if ( (passBuffer = (char *) malloc(sizeof(char)*(MAX_LENGTH+2))) == NULL ) {
			freeAll(2, &user, &nameBuffer);
			perror("Malloc failed");
		}
		if ( fgets(nameBuffer, 31, usersFile) == NULL || fgets(passBuffer, 31, usersFile) == NULL ) {
			freeAll(3, user, nameBuffer, passBuffer);
			return -1;
		}
		user->name = nameBuffer;
		user->password = passBuffer;
		n = fread(&nteams, sizeof(int), 1, usersFile);
		if ( n != 1 ) {
			freeAll(3, &user, &nameBuffer, &passBuffer);
			return -1;
		}
		teamsList = newListADT(cmpTeams);
		if ( teamsList == NULL ) {
			freeAll(3, &user, &nameBuffer, &passBuffer);
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
		add(users, user);
	}

	

	fclose(usersFile);
//	fclose(tradesFile);
//	fclose(expiresFile);
	fclose(playersFile);
	fclose(teamsFile);
	fclose(leaguesFile);


}

int
main(void) {
	Player * players[MAX_PLAYERS];
	ListADT * users = newListADT(cmpUsers);
	ListADT * leagues = newListADT(NULL);
	User * user;
	Team * team;
	Player * player;
	League * league;
	if ( load(users, leagues, players) < 0 ) {
		printf("Error al cargar\n");
	}
	else {
		printf("¡Éxito!\n");
	}
	listIteratorReset(users);
	user = listNext(users);
	while (user != NULL) {
		printf("===================\n");
		printf("USERNAME: %s\n", user->name); 
		printf("PASSWORD: %s\n", user->password);
		printf("ID: %d\n", user->userID); 
		printf("TEAMS:\n");
		listIteratorReset(user->teams);
		team = listNext(user->teams);
		while ( team != NULL ) {
			printf("\tTEAM NAME: %s\n", team->name); 
			printf("\tTEAM ID: %d\n", team->teamID);
			printf("\tLEAGUE ID: %d\n", team->leagueID);
			printf("\tTEAM POINTS: %d\n", team->points);
			printf("\tPLAYERS:\n");
			listIteratorReset(team->players);
			player = listNext(team->players);
			while ( player != NULL ) {
				printf("\t\tPLAYER NAME: %s\n", player->name); 
				printf("\t\tPLAYER ID: %d\n", player->playerID);
				printf("\t\tPLAYER SCORE: %d\n", player->score);
				player = listNext(team->players);
			}
			team = listNext(user->teams);
		}
		printf("===================\n");
		user = listNext(users);	
	}
	printf("LEAGUES:\n");
	listIteratorReset(leagues);
	league = listNext(leagues);
	while ( league != NULL ) {
		printf("LEAGUE NAME: %s\n", league->name);
		printf("LEAGUE ID: %d\n", league->leagueID);
		printf("LEAGUE SIZE: %d\n", league->maxSize);
		listIteratorReset(league->teams);
		team = listNext(league->teams);
		printf("TEAMS:\n");
		while ( team != NULL ) {
			printf("\tTEAM NAME: %s\n", team->name);
			team = listNext(league->teams);
		}
		listIteratorReset(league->nonDraftedPlayers);
		player = listNext(league->nonDraftedPlayers);
		printf("NON DRAFTED PLAYERS:\n");
		while ( player != NULL ) {
			printf("\tPLAYER NAME: %s\n", player->name);
			player = listNext(league->nonDraftedPlayers);
		}
		league = listNext(leagues);
	}
	time_t Time = time(NULL);
	printf("%s\n", ctime(&Time));
	Time +=  (24*3600);
	printf("%s\n", ctime(&Time));
}



