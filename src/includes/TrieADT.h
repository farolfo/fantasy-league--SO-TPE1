/****************
 * TrieADT.h
 ***************/

#ifndef _TrieADT_
#define _TrieADT_

typedef struct Trie Trie;

/*
 * Crea un nuevo trie vac�o
 */
Trie *  newTrie();

/*
 * Agrega un comando y le asocia una funci�n
 */
int addCommand( Trie *, const char *, int (*)(void *) );

/*
 * Ejecuta la funci�n asociada a un comando
 */
int doCommand( Trie *, const char * );

#endif
