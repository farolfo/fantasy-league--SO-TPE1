/****************
 * clserv.h
 ***************/

#ifndef _clserv_
#define _clserv_

#include "../includes/ListADT.h"
#include <time.h>

#define MAX_LENGTH	30
#define TRUE		0x1

typedef int OPC;


enum
{
	REGISTER = 0, LOGIN, REQUEST_TRADE, GET_LEAGUES_NAMES, GET_TEAMS_NAMES, GET_LEAGUE, 
	JOIN_LEAGUE, GET_TRADES, GET_TEAM, REMOVE_TRADE, NEGOTIATE_TRADE, 
	GET_TRADE, TRADE_ACCEPT, CREATE_LEAGUE, JOIN_DRAFT, QUIT_DRAFT, 
	CHOOSE_PLAYER, CHOOSE_PLAYER_ANS, DRAFT_TIMEOUT, DRAFT_UPDATE, 
	IS_ALIVE, END_DRAFT, GET_CHANNEL, LOGOUT, TIMEOUT, COMMAND_SIZE
};			

typedef int ID;

typedef struct Team {
	char name[MAX_LENGTH+1];
	ID teamID;
	ListADT * players;
	ID leagueID;
	ListADT	* incomingTrades;
	ListADT	* sentTrades;
	int points;
} Team;

typedef struct Player {
	char name[MAX_LENGTH+1];
	ID playerID;
	int score;
} Player;

typedef struct League {
	char name[MAX_LENGTH+1];
	ID leagueID;
	int maxSize;
	ListADT * teams;
	ListADT * nonDraftedPlayers;
} League;

typedef struct Trade {
	ID tradeID;
	ID teamFrom;
	ID teamTo;
	ID wantedPlayerID;
	ID incomingPlayerID;
	time_t date;
} Trade;

typedef struct NmsID{
	char name[100];
	ID id;
} NmsID;

typedef struct TradesSnd {
	char team[MAX_LENGTH+1];
	int size;
	NmsID * trades;
} TradesSnd;


typedef struct TeamSnd {
	char teamName[MAX_LENGTH+1];
	char leagueName[MAX_LENGTH+1];
	int cantPlayers;
	Player players[11];
	int points; //-1 significa fin.
} TeamSnd;

typedef struct LeagueSnd {
	char name[MAX_LENGTH+1];
	TeamSnd * teams;
} LeagueSnd;

typedef struct TradeSnd {
	char teamFrom[MAX_LENGTH+1];
	char teamTo[MAX_LENGTH+1];
	char playerFrom[MAX_LENGTH+1];
	char playerTo[MAX_LENGTH+1];
} TradeSnd;

typedef struct {
	int ans;
} RET_INT;

/* Client -> server */

//Parametros de login o register

typedef struct {
	char username[MAX_LENGTH+1];
	char password[MAX_LENGTH+1];
} CLSV_LOGREG;

//Parametros de createLeague
typedef struct {
	char userName[MAX_LENGTH+1];
	char leagueName[MAX_LENGTH+1];
	char teamName[MAX_LENGTH+1];
	int maxSize;
} CLSV_CLEAGUE;

//Parametros para getLeaguesNames o getTeamsNames
typedef struct {
} CLSV_GETNAMES;


//Paramatros para getLeague

typedef struct {
	ID leagueID;
} CLSV_GETLEAGUE;

//Paramatros para joinLeague

typedef struct {
	char userName[MAX_LENGTH+1];
	ID leagueID;
	char name[MAX_LENGTH+1];
} CLSV_JOINLEAGUE;

//Parametros para getTrades

typedef struct {
	char userName[MAX_LENGTH+1];
} CLSV_GETTRADES;

//Paramatros para getTeam
typedef struct {
	ID teamID;
} CLSV_GETTEAM;






#endif
