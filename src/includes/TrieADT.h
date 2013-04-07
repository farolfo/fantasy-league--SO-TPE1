/****************
 * TrieADT.h
 ***************/

#ifndef _TrieADT_
#define _TrieADT_

typedef struct Trie Trie;

/*
 * Crea un nuevo trie vacío
 */
Trie *  newTrie();

/*
 * Agrega un comando y le asocia una función
 */
int addCommand( Trie *, const char *, int (*)(void *) );

/*
 * Ejecuta la función asociada a un comando
 */
int doCommand( Trie *, const char * );

#endif
