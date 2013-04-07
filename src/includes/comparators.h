/****************
 * comparators.h
 ***************/

#ifndef _comparators_
#define _comparators_

/* ---Funciones de Comparacion--- */

int cmpUsers(void * u1, void * u2);

int cmpInts(void * i1, void * i2);

int cmpTeams(void* t1, void * t2);

int cmpPlayers(void * p1, void * p2); 

int cmpLeagues(void * l1, void * l2); 

int cmpTrades(void * t1, void * t2); 

int cmpStrings(void *, void *);

/* ---Funciones de Hashing--- */

int hashInt(void * id); 

int hashString(void * string);


#endif
