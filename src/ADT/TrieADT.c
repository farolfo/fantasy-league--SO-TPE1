#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Trie {
	struct Trie * array[27];
	int (* command)(void *);
} Trie;

Trie *
newTrie(){
	Trie * trie = (Trie *) calloc( sizeof(Trie),1 );
	
	if( trie == NULL ) {
		perror("Malloc  failed");
		return NULL;
	}
	return trie;
}

int
addCommand( Trie * trie, const char * commandName, int (* command)(void *) ) {
	if ( (*commandName) == '\0' ) {
		trie->command = command;
		return 0;
	}
	char c = tolower(*commandName);
	if ( c < 'a' || c > 'z' ) {
		perror("Invalid command name");
		return -1;
	}
	if ( (trie->array)[c-'a'] == NULL ) {
		if ( ( (trie->array)[c-'a'] = newTrie() ) == NULL ) {
			return -1;
		}
	}
	++commandName;
	return addCommand((trie->array)[c-'a'], commandName, command);
}

int
doCommand( Trie * trie, const char * commandName ) {
	if ( trie == NULL ) {
		return 0xCAFE;
	}
	if ( (*commandName) == '\0' ) {
		if ( trie->command == NULL ) {
			return 0xCAFE;
		}
		return trie->command(NULL);
	}

	char c = tolower(*commandName);
	const char * aux = commandName + 1;
	if ( c < 'a' || c > 'z' ) {
		return 0xCAFE;
	}		
	return doCommand((trie->array)[c - 'a'], aux);
}
			
/*
int
hola(void * pt) {
	printf("Hola\n");
	return 1;
}

int
main() {
	Trie * trie = newTrie();
	addCommand(trie, "hola", hola);
	doCommand(trie, "HolA");
	return 0;
}

	       
*/

