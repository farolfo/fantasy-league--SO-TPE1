/****************
 * dbms.h
 ***************/

#ifndef _dbms_
#define _dbms_


/* 
 * Carga toda la informaci�n de la base de datos con el formato correspondiente
 * En caso de error devuelve -1
 */
int load();


/*
 * Guarda toda la informaci�n en la base de datos con el formato correspondiente
 */
int save();

#endif
