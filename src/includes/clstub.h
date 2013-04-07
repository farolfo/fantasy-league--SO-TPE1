/****************
 * clstub.h
 ***************/

#ifndef _clstub_
#define _clstub_

#include "clserv.h"
#include "client.h"

#define _CONTINUE 0x1A1A


/* ---Funciones del stub de envío de información--- */

void logoutSnd(char * name);

void loginSnd(char * user, char * password);

void signUpSnd(char * user, char * password);

void userPassSnd(char * username, char * password, int opcode);

void sendOpcodeOnly(int op);

void getLeaguesNamesSnd();

void getTeamsNamesSnd();

NmsID * parseNmsID(char * data, int * cantNmsID);

void getTradesSnd(char * username);

void getLeagueSnd(int id);

void getTeamSnd(int id);

void parseTeamSnd(TeamSnd * ans, int pos, char ** index);

void createLeagueSnd(char * username, char * leagueName, char * teamName, int maxSize);

void joinLeagueSnd(char * username, ID id, char * teamName);

void getTradeSnd(char * name, int id);

void requestTradeSnd( char * name, ID teamIDFrom, ID teamIDTo, ID offeredPlayer, ID requestedPlayer);

void removeTradeSnd( char * name, int id);

void negotiateTradeSnd( char * name, ID tradeID, ID offeredPlayer, ID requestedPlayer);

void tradeAcceptSnd(char * username, int id);

void joinDraftSnd(int id);

//------------RECEPCCIÓN DE DATOS -----------

void receive();

void initializeRcvCommands();

int logoutRcv();

int draftTimeoutRcv();

int choosePlayerAnsRcv();

typedef struct DraftUpdateData{
  char teamName[31];
  char playerName[31];
} DraftUpdateData;

int draftUpdateRcv();

int endDraftRcv();

int isAliveRcv();

typedef struct ChoosePlayerData{
  char name[31];
  ID id;
}ChoosePlayerData;

int choosePlayerRcv();

int quitDraftRcv();

int signUpRcv();

int loginRcv();

int tradeAcceptRcv();

int getLeaguesNamesRcv();

int getTeamsNamesRcv();

NmsID * listProcessRcv();

int getTradesRcv();

int getLeagueRcv();

int getTeamRcv();

int createLeagueRcv();

int joinLeagueRcv();

int getTradeRcv();

int requestTradeRcv();

int removeTradeRcv();

int negotiateTradeRcv();

int joinDraftRcv();

#endif
