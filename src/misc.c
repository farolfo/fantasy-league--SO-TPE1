#include <string.h>
#include <stdio.h>

#include "includes/misc.h"
#include "includes/clserv.h"
#include "includes/server.h"

void
freeAll(int num, ...) {
	int i;
	va_list ap;
	va_start(ap, num);
	for ( i = 0  ; i < num ; i++ ) {
		free(*va_arg(ap, void **));
	}
	va_end(ap);
}

//Debe haberse hecho primero el free de trades!
void
freeTeam(void * team) {
	Team * t = (Team*) team;
	freeList(t->players);
	shallowFreeList(t->incomingTrades);
	shallowFreeList(t->sentTrades);
	free(t);
}

//Debe haberse hecho primero el free de teams!
void
freeLeague(void * league) {
	League * l = (League *) league;
	shallowFreeList(l->teams);
	freeList(l->nonDraftedPlayers);
	free(l);
}

int
cmpUsers(void * u1, void * u2) {
	return strcmp(((User*)u1)->name, ((User*)u2)->name);
}

int
cmpInts(void * i1, void * i2) {
	return *(int*)i1 - *(int*)i2;
}

int
cmpTeams(void* t1, void * t2) {
	return strcmp(((Team*)t1)->name, ((Team*)t2)->name);
}

int
cmpPlayers(void * p1, void * p2) {
	return ((Player *)p1)->playerID - ((Player*)p2)->playerID;
}

int
cmpLeagues(void * l1, void * l2) {
	return ((League *)l1)->leagueID - ((League*)l2)->leagueID;
}

int
cmpTrades(void * t1, void * t2) {
	return ((Trade*)t1)->tradeID - ((Trade*)t2)->tradeID;
}

int
cmpStrings(void * s1, void * s2){
  return strcmp((char*)s1, (char*)s2);
}

int
hashInt(void * id) {
	return *(int *)id;
}

int
hashString(void * string){
	return 10;
}

char *
itoa(char * buff, int num){
	sprintf(buff,"%d",num);
	return buff;
}


