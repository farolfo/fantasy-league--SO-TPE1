/****************
 * misc.h
 ***************/

#ifndef _misc_
#define _misc_


#include <stdlib.h>
#include <stdarg.h>

/*
 * Dado un entero, retorna el buffer con el entero pasado a char *
 */

char * itoa(char *,int);
void freeAll(int, ...); 

/*
 * Libera un equipo
 */
void freeTeam(void *);

/*
 * Libera una liga
 */
void freeLeague(void *);

#endif
