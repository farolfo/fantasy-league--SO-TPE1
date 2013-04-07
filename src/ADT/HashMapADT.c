#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../includes/ListADT.h"


#define TABLE_ENTRIES 100

typedef struct MapEntry {
	void * key;
	void * value;
} MapEntry;

typedef struct HashMap {
       	ListADT * array[TABLE_ENTRIES];
       	int (*hash)(void *);
       	int (*cmp)(void *, void *);
	void (*fr)(void *);		
} HashMap;

int
cmpEntry(HashMap * hm, MapEntry * m1, MapEntry * m2) {
	return (hm->cmp)(m1->key, m2->key);
}

static void
freeMapEntry(HashMap * hm, MapEntry * me) {
	hm->fr(me->value);
	free(me);
}

HashMap *
newHashMap( int (*hash)(void *), int (*cmp)(void *, void *),  void (*fr)(void*) ) {
	HashMap * hm = (HashMap *) calloc(sizeof(HashMap),1);
	hm->hash = hash;
	hm->cmp = cmp;
	hm->fr = fr;
	return hm;
}


int
addToHash(HashMap * ht, void * key, void * value) {
	int entry = (ht->hash)(key) % TABLE_ENTRIES;
	if ( (ht->array)[entry] == NULL ) {
		(ht->array)[entry] = newListADT(NULL, NULL);
		if ( (ht->array)[entry] == NULL ) {
			return -1;
		}
	}
	MapEntry * me = (MapEntry *) malloc(sizeof(MapEntry));
	if ( me == NULL ) {
		return -1;
	}
	me->key = key;
	me->value = value;
	Iterator * it = malloc(iteratorSize());
	listIteratorReset((ht->array)[entry], it);
	void * val;
	while ( (val = listNext((ht->array)[entry], it) ) != NULL ) {
		if ( !(ht->cmp)(((MapEntry *)val)->value, value) ) {
			return 1;
		}
	}
	free(it);
	return add((ht->array)[entry], me);
}

void *
getFromHash(HashMap * ht, void * key) {
	ListADT * targetList = (ht->array)[(ht->hash)(key) % TABLE_ENTRIES];
	if ( targetList == NULL ) {
		return NULL;
	}
	Iterator * it = malloc(iteratorSize());
	listIteratorReset(targetList, it);
	MapEntry * me = listNext(targetList, it);
	while ( me != NULL ) {
		if ( !(ht->cmp)(me->key, key) ) {
			free(it);
			return me->value;
		}
		me = listNext(targetList, it);
	}
	free(it);
	return NULL;
}

ListADT *
hashValuesList(HashMap * ht, int (*cmp)(void *, void *)) {
	ListADT * list = newListADT(cmp, ht->fr);
	MapEntry * me;
	int i;
	for ( i = 0 ; i < TABLE_ENTRIES ; i++ ) {
		if ( (ht->array)[i] != NULL ) {
			Iterator * it = malloc(iteratorSize());
			listIteratorReset((ht->array)[i], it);
			while ( (me = ((MapEntry *)listNext((ht->array)[i], it))) != NULL ) {
				add(list, me->value);
			}
			free(it);
		}
	}
	return list;
}

int
removeFromHash(HashMap * hm, void * key) {
	ListADT * targetList = (hm->array)[(hm->hash)(key) % TABLE_ENTRIES];
	if ( targetList == NULL ) {
		return -1;
	}
	Iterator * it = malloc(iteratorSize());
	listIteratorReset(targetList, it);
	MapEntry * me = listNext(targetList, it);
	int i = 0;
	while ( me != NULL ) {
		if ( !(hm->cmp)(me->key, key) ) {
			removeFromListI(targetList, i);
			return 0;
		}
		me = listNext(targetList, it);
		i++;
	}
	return -1;
}

void
freeHashMap(HashMap * hm) {
	int i;
	MapEntry * me;
	for ( i = 0 ; i < TABLE_ENTRIES ; i++ ) {
		if ( hm->array[i] != NULL ) {
			Iterator * it = malloc(iteratorSize());
			listIteratorReset(hm->array[i], it);
			while ( ( me = listNext(hm->array[i], it) ) != NULL ) {
				freeMapEntry(hm, me);
			}
			free(it);
		}
	}
	free(hm);
}


/*
int
hash(void * a) {
	int b = *(int*)a;
	int prime[] = {2,3,5,7,11};
	int pot = b % 10, i = 0, ans = 0;
	b = b / 10;
	while ( pot != 0 ) {
		ans += pow(prime[i], pot);
		i++;
		pot = b % 10;
		b = b / 10;
	}
	printf("%i\n", ans);
	return ans;
}


int
cmp(void * a, void * b) {
	return *(int*)a-*(int*)b;
}



int
main(void) {
	HashMap * hm = newHashMap(hash, cmp);
	int value = 4;
	char * st = "Holdsada";
	addToHash(hm, &value, st);

	printf("%s\n", getFromHash(hm, &value));

}

	

*/
