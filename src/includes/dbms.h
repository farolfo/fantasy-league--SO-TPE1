/****************
 * dbms.h
 ***************/

#ifndef _dbms_
#define _dbms_


/* 
 * Carga toda la información de la base de datos con el formato correspondiente
 * En caso de error devuelve -1
 */
int load();


/*
 * Guarda toda la información en la base de datos con el formato correspondiente
 */
int save();

#endif
