/****************
 * serverFunctions.h
 ***************/

#ifndef _serverFunctions_
#define _serverFunctions_

#include "../includes/clserv.h"

/*
 * Funciones de procesamiento del server
 */

int signUp(char username[31], char password[31]);

int createLeague(char userName[31], char leagueName[31], char teamName[31], int);

int choosePlayer(ID playerID, ID teamID);

int requestTrade(char[31], ID teamIDFrom, ID teamIDTo, ID offeredPlayer, ID requestedPlayer);

NmsID * getLeaguesNames();

NmsID * getTeamsNames();

LeagueSnd getLeague(ID leagueID);

int joinLeague(char userName[31], ID leagueID, char name[31]);

TradesSnd * getTrades(char userName[31]);

int login(char[31], char[31]);

TeamSnd getTeam(ID teamID);

int removeTrade(char[31], ID tradeID);

int negotiateTrade(char [31], ID tradeID, ID offeredPlayer, ID requestedPlayer);

TradeSnd getTrade(char[31], ID);

int tradeAccept(char[31], ID);

int playMatch(const char *);

int joinDraft(ID, void *);

void logOut(char *);

#endif
