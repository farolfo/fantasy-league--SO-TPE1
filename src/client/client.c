#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <sys/sem.h>
#include "../includes/client.h"
#include "../includes/clserv.h"
#include "../includes/clstub.h"
#include "../includes/TrieADT.h"
#include "../includes/ListADT.h"
#include "../includes/serverFunctions.h"
#include "../includes/physic.h"
#include "../includes/misc.h"


//Canales de comunicacion
void * channel; 
int syncPipe[2];
int signalPipe[2];//RECIEN AGREGADO

pid_t receiverPid;
pid_t senderPid;

Command commands[] = {
	{"listleagues", listleagues},
	{"listteams", listteams},
	{"listtrades", listtrades},
	{"logout", logout},
	{"showleague", showleague},
	{"showteam", showteam},
	{"showtrade", showtrade},
	{"trade", trade},
	{"tradeaccept", tradeaccept},
	{"tradewithdraw", tradewithdraw},
	{"tradenegotiate", tradenegotiate},
	{"createleague", createleague},
	{"joinleague", joinleague},
	{"joindraft", joindraft},
	{NULL,NULL}
};

extern pthread_mutex_t saveMutex;
char name[31];
Trie * trie = NULL;
//int drafting = 0;

Trie *
createTrie(void) {
	Trie * trie = newTrie();
	int i = 0;
	while( commands[i].name != NULL ) {
		addCommand(trie, commands[i].name, commands[i].com);
		i++;
	}
	return trie;
}	

static void
invalidCommand(){
	printf("Comando invalido.");
	return;
}

static void shell();

static void
draftWait(){
//	printf("Entrando en draftWait\n");
	printf("Please wait...\n");
	char buff[100];
	while(1){
		scanf("%s",buff);
		BORRA_BUFFER
		if(!strcmp(buff,"quit")){
			itoa(buff,QUIT_DRAFT);
			sendPacket(buff,strlen(buff)+1, channel, CLIENT);
			//drafting = 0;
			shell();
			break;
		}
	}
	return;
}

static void
shell() {
	if(trie == NULL){
		trie = createTrie();
	}
	char command[31];
	char buff[10];
	int ans;

	while(1) {
		printf("\n:>");
		scanf("%30s", command);
		BORRA_BUFFER
		ans = doCommand(trie, command);
		if ( ans == 0xCABA110 ) {
			printf("Logging out\n");
			beginShell();
			return;
		}
		
		if( ans == 0xCAFE ){ //0xCAFE == COMANDO INVALIDO
			invalidCommand();		
		}else{
			/*doCommand no deovlvio ni 0xCAFE ni 0xCABA11O, por lo tanto devolvio 1 y se trata de un comando valido
			para sincronizar ambos procesos del cliente espera al llamado del otro proceso*/
			read(syncPipe[0],buff,20);
			if( !strcmp(buff, "DRAFT") ){
				draftWait();
			}else if( strcmp(buff,"CONTINUE") ){
				printf("Error de comuncacion en el cliente.\nRazon: buffer = %s", buff);
				exit(0);
			}
		}
	}
}

static int 
reg() {//OK
	char username[MAX_LENGTH + 1], password[MAX_LENGTH + 1], rpassword[MAX_LENGTH + 1];
	printf("Ingrese nombre de usuario: ");
	scanf("%30s", username);
	BORRA_BUFFER
	printf("Ingrese contraseña: ");
	scanf("%30s", password);
	BORRA_BUFFER
	printf("Repita contraseña: ");
	scanf("%30s", rpassword);
	BORRA_BUFFER
	if ( !strcmp(password, rpassword) ) {
		strcpy(name,username);
		signUpSnd(username, password);	//TODO
		return 0;
	} else {
		printf("Las contraseñas no matchean\n");
		return -1;
	}
}

int
regRcv(int ans){
	//printf("Ans2 = %d\n", ans);
	if(ans != 0){
		strcpy(name,"");
		printf("Register failed.\n");
		return -1;
	}
	return 0;
}

static void
logg() {//OK
//	printf("Entrando a logg\n");
	char username[MAX_LENGTH + 1], password[MAX_LENGTH + 1];
	printf("Ingrese nombre de usuario: ");
	scanf("%30s", username);
	BORRA_BUFFER
	printf("Ingrese contraseña: ");
	scanf("%30s", password);
	BORRA_BUFFER
	strcpy(name,username);
	loginSnd(username,password);//TODO
	return;
}

int
loggRcv(int ans){
	if ( ans != 0 ) {
		printf("El login fallo\n");
		strcpy(name,"");
//		printf("El login fallo");
		return -1;
	}
	return 0;
}

int semid;
struct sembuf p = {0, -1, 0}, v = {0, 1, 0};

static void
shutDown(int a) {
	printf("Client is shutting down\n");
	logout();
	semctl(semid, 0,IPC_RMID, NULL);
	disconnectFromChannel(channel);
	free(channel);
	close(syncPipe[0]);
	close(signalPipe[0]);
	kill(receiverPid, SIGHUP);
	exit(0);
}

static void
catchSigPipe(int a) {
	exit(0);
}

int
main(void) {
	char buff[100];
	channel = (void *)malloc(200);
	
	/*---Establezco los canales de comunicacion---*/
	getDefaultChannel(channel);
	if ( connectToChannel(channel,CLIENT) < 0 ) {
		printf("Server not available\n");
		return 0;
	}
	itoa(buff,GET_CHANNEL);
	sendPacket(buff,strlen(buff)+1, channel, CLIENT);
	receivePacket(buff,MAX_SIZE, channel, CLIENT);
	disconnectFromChannel(channel);
	stringToChannel(buff, channel);
	connectToChannel(channel,CLIENT);

	/*---Abro el pipe de sincronismo entre padre e hijo---*/
	if( pipe(syncPipe) == -1 ){
		printf("Error al crear el pipe de sincronismo en el client.\n");
		return 0;
	}
	
	/*---Abro el pipe de pasaje de argumentos para seniales---*/
	if( pipe(signalPipe) == -1 ){
		printf("Error al crear el pipe de args para seniales.\n");
		return 0;	
	}
	
	/*---Seteo el semaforo---*/
	int semkey = getpid();
	semun x;
	x.val = 1;
	int flags = IPC_CREAT | IPC_EXCL;
	if ( ( semid = semget(semkey, 1, 0666 | flags )) == -1 ) {
		printf("Error al crear semaforo: %d\n", semkey);
		return -1;
	}
	if ( semctl(semid, 0, SETVAL, x) == -1 ) {
		printf("Error al configurar semaforo\n");
		return -1;
	}
	/*---Configuracion de manejo de seniales---*/
	/* SIGUSR1 -> SHELL_SIGNAL */
	/* SIGUSR2 -> DRAFT_SIGNAL */
	struct sigaction draftSigAct;
	draftSigAct.sa_handler = draftSignalHandler;
	sigemptyset(&(draftSigAct.sa_mask));
	draftSigAct.sa_flags = SA_NODEFER;
	struct sigaction shellSigAct;
	shellSigAct.sa_handler = shellSignalHandler;
	sigemptyset(&(shellSigAct.sa_mask));
	shellSigAct.sa_flags = SA_NODEFER;
	sigaction( SIGUSR1, &shellSigAct, NULL);
	sigaction( SIGUSR2, &draftSigAct, NULL);
	static struct sigaction act3;
	act3.sa_handler = catchSigPipe;
	sigfillset(&(act3.sa_mask));
	sigaction(SIGPIPE, &act3, NULL);
	static struct sigaction act1;
	static struct sigaction act2;

	
	/*Convenimos a este proceso como programa de envio de datos*/
	senderPid = getpid();
	pid_t auxPid;
	
	switch( auxPid = fork() ) {
		case -1:
			printf("Error al configurar el cliente.\n");
			break;
		case 0:
			receiverPid = getpid();
			close(syncPipe[0]); 
			close(signalPipe[0]);
			receive();
			break;
		default:
	
			act1.sa_handler = shutDown;
			sigfillset(&(act1.sa_mask));
			sigaction(SIGINT, &act1, NULL);
			act2.sa_handler = shutDown;
			sigfillset(&(act2.sa_mask));
			sigaction(SIGTSTP, &act2, NULL);
			receiverPid = auxPid;
			close(signalPipe[1]);
			close(syncPipe[1]);
			raise(SIGUSR1);break;
	}	
	return 0;
}

void
shellSignalHandler(int a){
	beginShell();
	return;
}

/*Jamas termina, si termina , cierra el programa*/
void
beginShell(){
	int op;
	char buff[100];
	while(1){
		printf("Elija una opcion\n");
		printf("\t(1)  Registrarse\n");
		printf("\t(2)  Login\n");
		printf("\t(3)  Salir\n");
		scanf("%d",  &op);
		BORRA_BUFFER
	//	printf("Opcode leido: %d\n", op);
		switch (op) {
			case 1:
				if ( reg() == 0 ) {
					read(syncPipe[0],buff,20);
					if(!strcmp(buff,"GOTO_SHELL")){
					//	printf("Yendo al shell\n");
						shell();
					}
				}
				break;
			case 2:
				logg();
				read(syncPipe[0],buff,20);
			//	printf("Leido del pipe: %s\n", buff);
				if(!strcmp(buff,"GOTO_SHELL"))
					shell();
				break;
			case 3:
				disconnectFromChannel(channel);
				kill(receiverPid, SIGHUP);
				exit(0);
			default:
				printf("Opcion invalida"); break;		
		}
	}
	return;
}

int
listleagues(){ //OK
	getLeaguesNamesSnd();
	return 0;
}

int
listleaguesRcv( NmsID * leaguesList){ //TODO
	/*id = -1 => Termino el array de NmsID*/
	if( leaguesList == NULL || leaguesList[0].id == -1)
		printf("There are no leagues created.\n");
	else{
		printf("League's name \t(ID)\n\n");
		printNmsID(leaguesList,0);
	}
	printf("\n");
	return 0;
}

int
listtrades(){//OK
	getTradesSnd(name);
	return 0;
}

int
listtradesRcv( TradesSnd * tradeList){ //TODO 
	/*id = -1 => Termino el arrayde NmsID*/
	int i = 0, flag = 0;
	if( tradeList == NULL || !strcmp(tradeList[0].team,"")  )
		printf("You've not made any trade, and no trade has been sent to you.\n");
	else{
		i = 0;
		while ( strcmp(tradeList[i].team, "" ) ) {
			if ( strcmp(tradeList[i].trades[0].name, "") ) {
				flag = 1;
			}
			i++;
		}
		if ( flag ) {
			i = 0;
			while( strcmp(tradeList[i].team,"") ){
				printf("Team : %s\n",tradeList[i].team);
				printNmsID(tradeList[i].trades,WITH_TAB);
				i++;
			}
			printf("\n");
		} else {
			printf("You've not made any trade, and no trade has been sent to you.\n");
		}
	}
	return 0;
}

void
printNmsID(NmsID * nmsID, int args){
	int i = 0;
	while(nmsID[i].id != -1){
		printf("%s%s\t(%d)\n",(args == WITH_TAB)?"\t":"",nmsID[i].name, nmsID[i].id);
		i++;
	}
	return;
}

int
listteams(){ //OK
	getTeamsNamesSnd(); 
	return 0;
}

int
listteamsRcv(NmsID * teamsList){ //TODO 
	/*id = -1 => Termino el array de NmsID*/
	if( teamsList == NULL || teamsList[0].id == -1)
		printf("There are no teams created.\n");
	else{
		printf("Teams's name \t(ID)\n\n");
		printNmsID(teamsList,0);
	}
	printf("\n");
//	freeTeamsList(teamsList);//TODO	
	return 0;
}

int 
logout(){
	printf("Name = %s\n", name);
	if ( strcmp(name, "") ) {
		logoutSnd(name);
	}
	return 0xCABA110;
}

int 
showleague(){ //OK
	printf("Please enter the league ID : ");
	int id;
	scanf("%d",&id);
	BORRA_BUFFER
	getLeagueSnd(id);
	return 0;
}

int
showleagueRcv(LeagueSnd leagueSnd){ //TODO
	int i;
	if( ! strcmp(leagueSnd.name,"") ){
		printf("Invalid ID.\n");
	}else{
		printf("League Name : %s",leagueSnd.name);
		i = 0;
		while( leagueSnd.teams[i].points != -1 ){
			printTeamSnd(leagueSnd.teams[i]);
			printf("\n");
			i++;
		}
	}
	return 0;
}

int
showteam(){//OK
	printf("Please enter the team ID : ");
	int id;
	scanf("%d",&id);
	BORRA_BUFFER
	getTeamSnd(id); //TODO
	return 0;
}

int
showteamRcv(TeamSnd teamSnd){ //TODO
	if( teamSnd.points == -1){
		printf("Invalid ID.\n");
	}else{
		printf("League : %s\n",teamSnd.leagueName); 
		printTeamSnd(teamSnd);
	}
	return 0;
}

void
printTeamSnd(TeamSnd teamSnd){
	printf("Team : %s\t%dpts.\n",teamSnd.teamName,teamSnd.points);
	int i = 0;
	for(; i < teamSnd.cantPlayers; i++){
		printf("\t%s\t|\t%dpts.",teamSnd.players[i].name,teamSnd.players[i].score);
	}
}

int
createleague(){//OK
	printf("New league's name: ");
	char leagueName[31];
	char teamName[31];
	int maxSize;
	scanf("%s",leagueName);
	BORRA_BUFFER
	printf("You must create a team. Team's name : ");
	scanf("%s",teamName);
	BORRA_BUFFER
	printf("How many teams will the league have? ");
	scanf("%d", &maxSize);
	createLeagueSnd(name, leagueName, teamName, maxSize);
	return 0;
}

int
createleagueRcv(int ans){ //TODO
	if(ans == -1)
		printf("No se pudo crear la liga.\n");
	return 0;
}

int
joinleague(){ //OK
	printf("League's ID: ");
	int id;
	char teamName[31];
	scanf("%d", &id);
	BORRA_BUFFER
	printf("You must create a team. Team's name : ");
	scanf("%s",teamName);
	BORRA_BUFFER
	joinLeagueSnd( name, id, teamName);
	return 0;
}

int
joinleagueRcv(int ans){ //TODO
	if(ans == -1)
		printf("No se pudo crear la liga.\n");

	return 0;
}

int 
showtrade(){ //OK
	int id;
	printf("Please enter the trade's ID :");
	scanf("%d",&id);
	BORRA_BUFFER
	getTradeSnd( name, id);
	return 0;
}

int
showtradeRcv(TradeSnd tradeInfo){ //TODO
	if(!strcmp(tradeInfo.teamFrom,""))
		printf("Invalid ID.\n");
	else{
		printf("Trade's info: ( Team that requested the trade | Offered player | Requested player | Requested player's team )\n");
		printf("%s\t%s\t%s\t%s\n",tradeInfo.teamFrom,tradeInfo.playerFrom,tradeInfo.playerTo,tradeInfo.teamTo);
	}
	return 0;
}

int
trade(){//OK
	int teamIDFrom, teamIDTo, offeredPlayer, requestedPlayer;
	printf("Your team's ID :");
	scanf("%d",&teamIDFrom);
	BORRA_BUFFER
	printf("The player's ID you offer :");
	scanf("%d",&offeredPlayer);
	BORRA_BUFFER
	printf("The team's ID you want to trade with :");
	scanf("%d",&teamIDTo);
	BORRA_BUFFER
	printf("The player's ID you request :");
	scanf("%d",&requestedPlayer);
	BORRA_BUFFER
	requestTradeSnd( name, teamIDFrom, teamIDTo, offeredPlayer, requestedPlayer); //TODO
	return 0;
}

int
tradeRcv(int ans){//TODO
	if( ans == -1 ){
		printf("Invalid information.\n");
	}else{
		printf("Trade sent.");
	}
	return 0;
}

int
tradewithdraw(){//OK
	int id;
	printf("Ingrese el ID del trade a remover :");
	scanf("%d",&id);
	BORRA_BUFFER
	removeTradeSnd(name,id); //TODO
	return 0;
}

int
tradewithdrawRcv(int ans){ //TODO
	if( ans == -1 ){
		printf("Fallo la opearcion. Por favor valide los datos.\n");
	}else{
		printf("Trade eliminado con exito.\n");
	}
	return 0;
}

int
tradenegotiate(){
	int id, offeredPlayer, requestedPlayer;
	printf("Ingrese el ID del trade a negociar :");
	scanf("%d",&id);
	BORRA_BUFFER
	printf("Ingrese el ID de su jugador que ahora ofrece :");
	scanf("%d",&offeredPlayer);
	BORRA_BUFFER
	printf("Ingrese el ID del jugador que ahora usted pide :");
	scanf("%d",&requestedPlayer);
	BORRA_BUFFER
	negotiateTradeSnd( name, id, offeredPlayer, requestedPlayer); //TODO
	return 0;
}

int
tradenegotiateRcv(int ans){//TODO
	if( ans == -1){
		printf("Fallo la operacion. Por favor valide los datos.\n");
	}else{
		printf("Solicitud enviada con exito.\n");
	}
	return 0;
}

int
tradeaccept(){//OK
	int id;
	printf("Ingrese el ID del trade : ");
	scanf("%d",&id);
	BORRA_BUFFER
	tradeAcceptSnd(name,id);
	return 0;
}

int
tradeacceptRcv(int ans){//TODO
	if( ans == -1 ){
		printf("ID invalido,\n");
	}else{
		printf("Operacion realizada con exito.\n");
	}
	return 0;
}

int
joindraft(){
	int id;
	printf("Ingrese el ID de su equipo : ");
	scanf("%d",&id);
	BORRA_BUFFER
	joinDraftSnd(id);
	return 0;
}

int
joindraftRcv(int ans){
	if(ans == -1){
		printf("No se pudo unir al draft.\n");
		return -1;
	}
	//drafting = 1;
	return 0;
}


int
drafttimeoutRcv(char * name){
	printf("Tu tiempo ha excedido, se te ha asignado al jugador %s.\n",name);
	int op = DRAFT_TIMEOUT_SIG;
	write( signalPipe[1], &op, sizeof(int));
	kill( senderPid, SIGUSR2);
	return 0;
}

int
chooseplayeransRcv(int ans){
	int op;
	if(ans == -1){
		printf("No se pudo elegir al jugador deseado.\n");
		op = CHOOSE_PLAYER_SIG;
		write( signalPipe[1], &op, sizeof(int));	
		kill( senderPid, SIGUSR2);
	} else if(ans == 0){
			printf("El jugador se ha elegido con exito\n");
			op = CHOOSE_PL_SUCCES_SIG;
			write( signalPipe[1], &op, sizeof(int));
			kill( senderPid, SIGUSR2);
		} else{
			printf("Hubo un error en la conexion.\n");
		}
	return 0;
}

int
draftupdateRcv(DraftUpdateData ans){
	int op;
	printf("El equipo %s ha elegido al jugador %s.\n", ans.teamName, ans.playerName);
	op = DRAFT_UPDATE_SIG;
	write( signalPipe[1], &op, sizeof(int));
	kill( senderPid, SIGUSR2);
	return 0;
}

int
isaliveRcv(){
	int op;
	op = IS_ALIVE_SIG;
	write( signalPipe[1], &op, sizeof(int));
	kill( senderPid, SIGUSR2);
	return 0;
}

int
enddraftRcv(){
	int op;
	op = END_DRAFT_SIG;
	write( signalPipe[1], &op, sizeof(int));
	kill( senderPid, SIGUSR2);
	return 0;
}

int
chooseplayerRcv(ChoosePlayerData * plData){
	int i = 0, op;
	printf("Estos son los jugadores disponibles ( Name - ID ):\n");
	while( strcmp(plData[i].name,"")){
		printf("\t%s\t%d\n",plData[i].name,plData[i].id);
		i++;
	}
	printf("A partir de ahora tienes 1 minuto para elegir.");
	op = CHOOSE_PLAYER_SIG;
	write( signalPipe[1], &op, sizeof(int));
	kill( senderPid, SIGUSR2);
	return 0;
}

/*---Funciones de handling de seniales del draft---*/

int (*draftHandlingFun[SIZE_SIG])(void);

void
draftSignalHandler(int sig){
	initializeSigHandler();
	int op;
	read( signalPipe[0], &op, sizeof(int));
	draftHandlingFun[op] ();
	return;
}

void
initializeSigHandler(){
	(draftHandlingFun[DRAFT_TIMEOUT_SIG]) = drafttimeoutSignal;
	(draftHandlingFun[CHOOSE_PLAYER_SIG]) = chooseplayerSignal;
	(draftHandlingFun[CHOOSE_PL_SUCCES_SIG]) = chooseplayersuccesSignal;
	(draftHandlingFun[DRAFT_UPDATE_SIG]) = draftUpdateSignal;
	(draftHandlingFun[IS_ALIVE_SIG]) = isaliveSignal;
	(draftHandlingFun[END_DRAFT_SIG]) = enddraftSignal;
	return;
}

int
chooseplayerSignal(){
	printf("Indica el ID del jugador que desea, apresurate que tienes poco tiempo: ");
	int id;
	scanf("%d",&id);
	printf("El ID elegido es %d\n", id);
	BORRA_BUFFER
	char buff[21];
	itoa(buff,CHOOSE_PLAYER);
	int size = strlen(buff) + 1;
	itoa(buff+size,id);
	sendPacket(buff,size+strlen(buff+size)+1,channel, CLIENT);
	//semop(semid, &v, 1);
	draftWait();
	return 0;
}

int
enddraftSignal(){
//	drafting = 0;
	shell();
	raise(SIGUSR1);//Lanza el shell inicial
	return 0;//Jamas llega a este punto
}

int
isaliveSignal(){
	char buff[20];  
	while(1){
		printf("Estas listo? [y o n] \n:>");
		char ans;
		scanf("%c",&ans);
		BORRA_BUFFER
		if( tolower(ans) == 'y' ){
			itoa(buff,IS_ALIVE);
			sendPacket(buff,strlen(buff)+1,channel, CLIENT);
			//vuelve de donde termino o //TODO 
			draftWait();
			return 0;
		}else if( tolower(ans) == 'n'){
			itoa(buff,QUIT_DRAFT);
			sendPacket(buff,strlen(buff)+1,channel, CLIENT);
			shell();
			raise(SIGUSR1);
			return 0;
		}
	}
	return 0;
}

int
draftUpdateSignal(){
	//semop(semid, &v, 1);	
	draftWait();
	return 0;
}

int
chooseplayersuccesSignal(){
	draftWait();
	return 0;
}

int
drafttimeoutSignal(){
	draftWait();
	return 0;
}
