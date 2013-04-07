/****************
 * HashMapADT.h
 ***************/

#ifndef _HashMapADT_
#define _HashMapADT_

typedef struct HashMap HashMap;


/*
 * Retorna un HashMap nuevo dados una función de hashing, una de comparación y una de liberación
 */
HashMap * newHashMap( int (*)(void *), int (*)(void *, void *), void (*)(void*) );

/*
 * Agrega al hashmap un elemento con una clave y un valor
 */
int addToHash(HashMap *, void *, void *);

/*
 * Devuelve el valor asociado a la clave del hashmap correspondiente.
 * En caso de no existir devuelve NULL
 */
void * getFromHash(HashMap *, void *);

/*
 * Retorna una lista de los valores del hashmap.
 */
ListADT * hashValuesList(HashMap *, int (*) (void *, void *));

/*
 * Remueve el elemento del hashmap asociado a la clave.
 */
int removeFromHash(HashMap *, void *);

/*
 * Libera el hashmap
 */
void freeHashMap(HashMap *);

#endif
