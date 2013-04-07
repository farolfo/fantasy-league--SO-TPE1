#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sem.h>
#include <signal.h>
#include "../includes/misc.h"
#include "../includes/client.h"
#include "../includes/clstub.h"
#include "../includes/clserv.h"
#include "../includes/physic.h"

extern void * channel;

extern int syncPipe[2];

extern pid_t senderPid;

extern int semid;
extern struct sembuf p, v;

//extern int drafting;

static char clsvbuff[MAX_SIZE];

//------- ENVIO DE DATOS -----------
static int
send( int qty ) {
	sendPacket(clsvbuff, qty, channel, CLIENT);
	return 0;
}

void
logoutSnd(char * name){
	char * index = clsvbuff;
	itoa(index, LOGOUT);
	index += strlen(index) + 1;
	strcpy(index,name);
	index += strlen(index) + 1;
	send(index - clsvbuff);
}

void
loginSnd(char * user, char * password){//ok
	userPassSnd(user,password,LOGIN);
	return;
}

void
signUpSnd(char * user, char * password){//ok
	userPassSnd(user,password,REGISTER);
	return;
}

void
userPassSnd(char * username, char * password, int opcode) {
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,opcode));
	index += strlen(buff) + 1;
	strcpy(index,username);
	index += strlen(username) + 1;
	strcpy(index,password);
	index += strlen(password) + 1;
	send(index-clsvbuff);
	return;
}
	
void
sendOpcodeOnly(int op){
	char buff[31];
	strcpy(clsvbuff, itoa(buff,op));
	send(strlen(clsvbuff)+1);
	return;
}

void
getLeaguesNamesSnd(){//OK
	sendOpcodeOnly(GET_LEAGUES_NAMES);
	return ;
}

void
getTeamsNamesSnd(){//OK
	sendOpcodeOnly(GET_TEAMS_NAMES);
	return;
}

NmsID *
parseNmsID(char * data, int * cantNmsID){
	char * index = data;
	NmsID * ans = NULL;
	int i = 0;
	while( strcmp(index,"") ){
		ans = (NmsID *) realloc( ans, sizeof(NmsID) * (i+1));
		strcpy(ans[i].name,index);
		index += (strlen(index) + 1);
		ans[i].id = atoi(index);
		index += (strlen(index) + 1);
		i++;
	}
	(*cantNmsID) = i;
	return ans;
}

void
getTradesSnd(char * username){//OK
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,GET_TRADES));
	index += strlen(buff) + 1;
	strcpy(index,username);
	index += strlen(username) + 1;
	send(index-clsvbuff);
	return;
}

void
getLeagueSnd(int id){//OK
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,GET_LEAGUE));
	index += strlen(index) + 1;
	strcpy(index, itoa(buff,id));
	index += strlen(buff) + 1;
	send(index-clsvbuff);
	return;
}

void
getTeamSnd(int id){//OK
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,GET_TEAM));
	index += strlen(index) + 1;
	strcpy(index, itoa(buff,id));
	index += strlen(buff) + 1;
	send(index-clsvbuff);
	return;
}

void
parseTeamSnd(TeamSnd * ans, int pos, char ** index){
	int i;
	strcpy(ans[pos].teamName,*index);
	(*index) += strlen(*index) + 1;
	strcpy(ans[pos].leagueName,*index);
	(*index) += strlen(*index) + 1;
	ans[pos].cantPlayers = atoi(*index);
	(*index) += strlen(*index) + 1;
	for( i = 0; i < ans[pos].cantPlayers ; i++){
		strcpy((ans[pos].players)[i].name,*index);
		(*index) += strlen(*index) + 1;
		(ans[pos].players)[i].playerID = atoi(*index);
		(*index) += strlen(*index) + 1;
		(ans[pos].players)[i].score = atoi(*index);
		(*index) += strlen(*index) + 1;
	}
	ans[pos].points = atoi(*index);
	(*index) += strlen(*index) + 1;
	return;
}

void
createLeagueSnd(char * username, char * leagueName, char * teamName, int maxSize){//OK
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,CREATE_LEAGUE));
	index += strlen(index) + 1;
	strcpy(index,username);
	index += strlen(username) + 1;
	strcpy(index,leagueName);
	index += strlen(leagueName) + 1;
	strcpy(index, teamName);
	index += strlen(teamName) + 1;
	strcpy(index, itoa(buff,maxSize));
	index += strlen(buff) + 1;
	send(index-clsvbuff);
	return;
}

void
joinLeagueSnd(char * username, ID id, char * teamName){//OK
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,JOIN_LEAGUE));
	index += strlen(index) + 1;
	strcpy(index,username);
	index += strlen(username) + 1;
	strcpy(index,itoa(buff,id));
	index += strlen(index) + 1;
	strcpy(index, teamName);
	index += strlen(teamName) + 1;
	send(index-clsvbuff);
	return;
}

void
getTradeSnd(char * name, int id){//OK
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,GET_TRADE));
	index += strlen(index) + 1;
	strcpy(index,name);
	index += strlen(index) + 1;
	strcpy(index, itoa(buff,id));
	index += strlen(buff) + 1;
	send(index-clsvbuff);
	return;
}

void
requestTradeSnd( char * name, ID teamIDFrom, ID teamIDTo, ID offeredPlayer, ID requestedPlayer){//OK
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,REQUEST_TRADE));
	index += strlen(buff) + 1;
	strcpy(index,name);
	index += strlen(index) + 1;
	strcpy(index,itoa(buff,teamIDFrom));
	index += strlen(buff) + 1;
	strcpy(index,itoa(buff,teamIDTo));
	index += strlen(buff) + 1;
	strcpy(index,itoa(buff,offeredPlayer));
	index += strlen(buff) + 1;
	strcpy(index,itoa(buff,requestedPlayer));
	index += strlen(buff) + 1;
	send(index-clsvbuff);
	return;
}

void
removeTradeSnd( char * name, int id){//OK
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,REMOVE_TRADE));
	index += strlen(index) + 1;
	strcpy(index,name);
	index += strlen(index) + 1;
	strcpy(index, itoa(buff,id));
	index += strlen(buff) + 1;
	send(index-clsvbuff);
	return;
}

void
negotiateTradeSnd( char * name, ID tradeID, ID offeredPlayer, ID requestedPlayer){//ok
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,NEGOTIATE_TRADE));
	index += strlen(index) + 1;
	strcpy(index,name);
	index += strlen(index) + 1;
	strcpy(index, itoa(buff,tradeID));
	index += strlen(buff) + 1;
	strcpy(index, itoa(buff,offeredPlayer));
	index += strlen(buff) + 1;
	strcpy(index, itoa(buff,requestedPlayer));
	index += strlen(buff) + 1;
	send(index-clsvbuff);
	return;
}

void
tradeAcceptSnd(char * username, int id){//ok
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,TRADE_ACCEPT));
	index += strlen(index) + 1;
	strcpy(index,username);
	index += strlen(username) + 1;
	strcpy(index, itoa(buff,id));
	index += strlen(buff) + 1;
	send(index-clsvbuff);
	return;
}

void
joinDraftSnd(int id){	
	char * index = clsvbuff;
	char buff[31];
	strcpy(index,itoa(buff,JOIN_DRAFT));
	index += strlen(index) + 1;
	strcpy(index,itoa(buff,id));
	index += strlen(index) + 1;
	int size = channelToString( index, channel);
	send( (index-clsvbuff) + size);
	return;
}

//------------RECIVO DE DATOS -----------

int (*fun[COMMAND_SIZE])(void); //COMMAND_SIZE = Tamanio del enum de clserv

void
receive(){
	initializeRcvCommands();
	int opcode;
	int ans;
	char buff[20];
	while(1){
		int z;
		if ( ( z = receivePacket(clsvbuff, MAX_SIZE, channel, CLIENT) ) <= 0 ) { //bloqueante
			printf("La conexion con el servidor se corto\n");
			kill(senderPid, SIGHUP);
			exit(-1);
		}
		opcode = atoi(clsvbuff);
		//printf("Drafting = %d\n", drafting);
		//if ( opcode > JOIN_DRAFT && opcode <= END_DRAFT && drafting == 0 ) {
		//	printf("No esta drafteando\n");
		//	continue;
		//}	
		ans = fun[opcode] ();
		switch(opcode){
			case REGISTER: case LOGIN:
				if( ans == 0 ) {
					strcpy(buff,"GOTO_SHELL");
				}
				else {
					strcpy(buff,"FAILED");
				}
				write( syncPipe[1], buff, 20);
				break;
			case JOIN_DRAFT:
				if(ans == -1)
					strcpy(buff,"CONTINUE");
				else
					strcpy(buff,"DRAFT");
				write( syncPipe[1], buff, 20);
				break;
			case QUIT_DRAFT: case IS_ALIVE: case CHOOSE_PLAYER: case END_DRAFT:
			case DRAFT_TIMEOUT: case CHOOSE_PLAYER_ANS: case DRAFT_UPDATE: case LOGOUT:
				break;
			default:
				strcpy( buff, "CONTINUE");
				write( syncPipe[1], buff, 20);
				break;
		}
	}
	return;
}

int
timeoutRcv() {
	printf("La conexión con el server expiro\n");
	logout();
	disconnectFromChannel(channel);
	kill(senderPid, SIGHUP);
	exit(0);
}



void
initializeRcvCommands(){
	(fun[TRADE_ACCEPT]) = tradeAcceptRcv;//ok
	(fun[GET_LEAGUE]) = getLeagueRcv;//OK
	(fun[REGISTER]) = signUpRcv;//OK
	(fun[LOGIN]) = loginRcv;//OK
	(fun[REQUEST_TRADE]) = requestTradeRcv;//OK
	(fun[GET_LEAGUES_NAMES]) = getLeaguesNamesRcv;//OK
	(fun[GET_TEAMS_NAMES]) = getTeamsNamesRcv;//OK
	(fun[JOIN_LEAGUE]) = joinLeagueRcv;//OK
	(fun[GET_TRADES]) = getTradesRcv;//OK
	(fun[GET_TEAM]) = getTeamRcv;//OK
	(fun[REMOVE_TRADE]) = removeTradeRcv;//OK
	(fun[NEGOTIATE_TRADE]) = negotiateTradeRcv;//ok
	(fun[GET_TRADE]) = getTradeRcv;//OK
	(fun[CREATE_LEAGUE]) = createLeagueRcv;//OK
	(fun[JOIN_DRAFT]) = joinDraftRcv;//OK
	(fun[QUIT_DRAFT]) = quitDraftRcv;//OK
	(fun[CHOOSE_PLAYER]) = choosePlayerRcv;//OK
	(fun[IS_ALIVE]) = isAliveRcv;//OK
	(fun[END_DRAFT]) = endDraftRcv;//OK
	(fun[CHOOSE_PLAYER_ANS]) = choosePlayerAnsRcv;//OK
	(fun[DRAFT_UPDATE]) = draftUpdateRcv;//OK
	(fun[DRAFT_TIMEOUT]) = draftTimeoutRcv;//OK
	(fun[LOGOUT]) = logoutRcv;
	(fun[TIMEOUT]) = timeoutRcv;
	return;
}

int
logoutRcv(){
	return 0;
}

int
draftTimeoutRcv(){
	char name[31];
	strcpy( name, clsvbuff + strlen(clsvbuff) + 1);
	drafttimeoutRcv(name);
	return 0;
}

int
choosePlayerAnsRcv(){
	chooseplayeransRcv( atoi(clsvbuff + strlen(clsvbuff) + 1) );
	return 0;
}

int
draftUpdateRcv(){
	//Bajar semaforo y bloquear.
//	semop(semid, &p, 1);
	DraftUpdateData ans;
	char * index = clsvbuff + strlen(clsvbuff) +1;
	strcpy( ans.teamName, index);
	index += strlen(index) + 1;
	strcpy( ans.playerName, index);
	draftupdateRcv(ans);
	return 0;
}

int 
endDraftRcv(){
	enddraftRcv();
	return 0;
}

int
isAliveRcv(){
	isaliveRcv();
	return 0;
}

int
choosePlayerRcv(){
	//Bajar semaforo. se deberia bloquear por draftupdate.
//	semop(semid, &p, 1);
	char * index = clsvbuff + strlen(clsvbuff) + 1;
	ChoosePlayerData * plData = NULL;
	int i = 0;
	while( strcmp(index,"") ){
		plData = (ChoosePlayerData *)realloc( plData, sizeof(ChoosePlayerData) * (i+1) );
		strcpy( plData[i].name, index);
		index += strlen(index) + 1;
		plData[i].id = atoi(index);
		index += strlen(index) + 1;
		i++;
	}
	plData = (ChoosePlayerData *)realloc( plData, sizeof(ChoosePlayerData) * (i+1) );
	strcpy(plData[i].name,"");//Fin de coleccion
	chooseplayerRcv(plData);
	return 0;
}

int
quitDraftRcv(){
	//El server solo me envia esto para verificar que se hizo QUIT_DRAFT correctamente
	return 0;
}

int
signUpRcv(){
	return regRcv(atoi(clsvbuff+strlen(clsvbuff)+1));
}

int
loginRcv(){
	return loggRcv(atoi(clsvbuff+strlen(clsvbuff)+1));	
}

int
tradeAcceptRcv(){
	tradeacceptRcv(atoi(clsvbuff + strlen(clsvbuff) + 1));
	return 0;
}

int
getLeaguesNamesRcv(){
	listleaguesRcv(listProcessRcv());
	return 0;
}

int
getTeamsNamesRcv(){
	listteamsRcv(listProcessRcv());
	return 0;
}

NmsID *
listProcessRcv(){
	int cantNmsID=0;
	NmsID * ans = parseNmsID( (clsvbuff + strlen(clsvbuff) + 1), &cantNmsID);
	ans = (NmsID *) realloc( ans, sizeof(NmsID) * (cantNmsID+1));
	strcpy(ans[cantNmsID].name,"");
	ans[cantNmsID].id = -1;
	return ans;
}

int
getTradesRcv(){
	TradesSnd * ans = NULL;
	int i =0, j=0;
	char * index = clsvbuff + strlen(clsvbuff) + 1;
	while( strcmp(index,"") ){
		ans = (TradesSnd *) realloc(ans, sizeof(TradesSnd) * (i+1));
		strcpy(ans[i].team,index);
		index += (strlen(index) + 1);
		ans[i].size = atoi(index);
		index += (strlen(index) + 1);
		ans[i].trades = (NmsID *)calloc( ans[i].size + 1, sizeof(NmsID));
		for( j=0; /*index != '\0'*/ j < ans[i].size; j++){
			strcpy( ans[i].trades[j].name, index); 
			index += strlen(index) + 1;
			ans[i].trades[j].id = atoi(index);
			index += strlen(index) + 1;
		}
		//strcpy( ans[i].trades[j].name, ""); 
		//ans[i].trades[j].id = -1;
		i++;
	}
	ans = (TradesSnd *) realloc(ans,sizeof(TradesSnd) * (i+1));
	strcpy( ans[i].team, "");
	ans[i].size = 0;
	ans[i].trades = NULL;
	listtradesRcv(ans);
	return 0;
}

int
getLeagueRcv(){
	LeagueSnd ans;
	char * index = clsvbuff + strlen(clsvbuff) + 1;
	strcpy(ans.name,index);
	index += strlen(index) + 1;
	int i = 0, j;
	ans.teams = NULL;
	if ( *(ans.name) == '\0' ) {
		showleagueRcv(ans);
	}
	while( strcmp(index,"") ){
		ans.teams = (TeamSnd *) realloc( ans.teams, sizeof(TeamSnd) * (i+1));
		parseTeamSnd(ans.teams,i,&index);
		i++;
	}
	ans.teams = (TeamSnd *) realloc( ans.teams, sizeof(TeamSnd) * (i+1));
	strcpy((ans.teams)[i].teamName,"");
	strcpy((ans.teams)[i].leagueName,"");
	(ans.teams)[i].cantPlayers = -1;
	for( j = 0; j < 11; j++){
		strcpy(((ans.teams)[i].players)[j].name,"");
		((ans.teams)[i].players)[j].playerID = -1;
		((ans.teams)[i].players)[j].score = -1;
	}
	(ans.teams)[i].points = -1; 
	showleagueRcv(ans);
	return 0;
}

int
getTeamRcv(){
	char * index = clsvbuff + strlen(clsvbuff) + 1;
	TeamSnd ans;
	parseTeamSnd(&ans,0,&index);
	showteamRcv(ans);
	return 0;
}

int
createLeagueRcv(){
	createleagueRcv(atoi(clsvbuff + strlen(clsvbuff) + 1));
	return 0;
}

int
joinLeagueRcv(){
	joinleagueRcv(atoi(clsvbuff + strlen(clsvbuff) + 1));
	return 0;
}

int
getTradeRcv(){
	TradeSnd ans;
	char * index = clsvbuff + strlen(clsvbuff) + 1;
	strcpy(ans.teamFrom,index);
	index += strlen(index) +1 ;
	strcpy(ans.teamTo,index);
	index += strlen(index) +1 ;
	strcpy(ans.playerFrom,index);
	index += strlen(index) +1 ;
	strcpy(ans.playerTo,index);
	index += strlen(index) +1 ;
	showtradeRcv(ans);
	return 0;
}

int
requestTradeRcv(){
	tradeRcv(atoi(clsvbuff + strlen(clsvbuff) + 1));
	return 0;
}

int
removeTradeRcv(){
	tradewithdrawRcv(atoi(clsvbuff + strlen(clsvbuff) + 1));
	return 0;
}

int
negotiateTradeRcv(){
	tradenegotiateRcv(atoi(clsvbuff + strlen(clsvbuff) + 1));
	return 0;
}

int
joinDraftRcv(){
	return joindraftRcv(atoi(clsvbuff + strlen(clsvbuff) + 1));
}
