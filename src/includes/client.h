/****************
 * client.h
 ***************/

#ifndef _client_
#define _client_

#include "TrieADT.h"
#include "clserv.h"
#include "clstub.h"


typedef struct Command {
	char * name;
	int (*com)();
} Command;

typedef union semun {
	int val;
	struct semid_ds * buf;
	ushort * array;
} semun;

#define BORRA_BUFFER	while(getchar() != '\n');

#define WITH_TAB	1

/*Opcodes del manejo de senailes del draft*/
enum{
	DRAFT_TIMEOUT_SIG, CHOOSE_PLAYER_SIG, CHOOSE_PL_SUCCES_SIG, DRAFT_UPDATE_SIG, IS_ALIVE_SIG, END_DRAFT_SIG, SIZE_SIG/*Tamaño del enum*/
};

/*
 * Manejadores de señales
 */
void draftSignalHandler(int);
void shellSignalHandler(int);
void initializeSigHandler();


/*
 * Imprime el menú principal
 */
void beginShell();

/*
 * Inicializa el trie con sus comandos
 */
Trie * createTrie(void);

/* ---Funciones de manejo de recepción de información--- */
int regRcv(int ans);

int loggRcv(int ans);

int listleaguesRcv( NmsID * leaguesList);

int listtradesRcv( TradesSnd * tradeList);

int listteamsRcv(NmsID * teamsList);

int showleagueRcv(LeagueSnd leagueSnd);

int showteamRcv(TeamSnd teamSnd);

int createleagueRcv(int ans);

int joinleagueRcv(int ans);

int showtradeRcv(TradeSnd tradeInfo);

int tradeRcv(int ans);

int tradewithdrawRcv(int ans);

int tradenegotiateRcv(int ans);

int tradeacceptRcv(int ans);

int joindraftRcv(int ans);

int drafttimeoutRcv(char * name);

int chooseplayeransRcv(int ans);

int draftupdateRcv(DraftUpdateData ans);

int isaliveRcv();

int enddraftRcv();

int chooseplayerRcv(ChoosePlayerData * plData);

/* ---Funciones de atención de comandos--- */

int listleagues();

int listtrades();

void printNmsID(NmsID * nmsID, int args);

int listteams();

int logout();

int showleague();

int showteam();

void printTeamSnd(TeamSnd teamSnd);

int createleague();

int joinleague();

int showtrade();

int trade();

int tradewithdraw();

int tradenegotiate();

int tradeaccept();

int joindraft();

/* ---Funciones de manejo de señales del draft particulares--- */

int drafttimeoutSignal();

int chooseplayersuccesSignal();

int draftUpdateSignal();

int isaliveSignal();

int enddraftSignal();

int chooseplayerSignal();


#endif
