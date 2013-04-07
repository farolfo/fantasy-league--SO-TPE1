#include <stdio.h>
#include <stdlib.h>

typedef struct Node{
	void * data;
	struct Node * next; 
} Node;

typedef struct QueueADT{
	Node * first;
	Node * last;
	int size;
} QueueADT;

QueueADT *
newQueue(){
	QueueADT * queue = (QueueADT *) malloc(sizeof(QueueADT));
	if( queue == NULL ) {
		perror("Mallc failed.");
		return NULL;
	}

	queue->first = NULL;
	queue->last = NULL;
	queue->size = 0;
	
	return queue;
}


/**
 *	0 -> COLA CON DATOS
 *	1 -> COLA VACIA
 * */
int
queueIsEmpty(QueueADT * queue){
	return (queue->size) ? 0 : 1 ;
}


/**
 *	Retorna NULL en caso de no haber elementos a desencolar.
 */
void *
dequeue(QueueADT * queue){
	if( queue->first == NULL )
		return NULL;

	void * ans;
	ans = queue->first->data;

	if( queue->size == 1 ){
		queue->last = queue->first = NULL;	
	} else if( queue->size == 2 ){
		queue->first = queue->first->next; 
		queue->last = queue->first;
	} else	
		queue->first = queue->first->next;
	

	queue->size --;
	return ans;
}


/**
 *	Retorna -1 en caso de error al encolar. 
 */
int
enqueue(QueueADT * queue, void * data){
	Node * newNode = (Node *) malloc(sizeof(Node));
	if( newNode == NULL ){
		perror("Malloc failed.");
		return -1;
	}
	
	newNode->data = data;
	newNode->next = NULL;

	if( queue->size == 0 ){
		queue->first = newNode;
	}else{
		queue->last->next = newNode;
	}
	queue->last = newNode;
	queue->size++;
	return 0;
}


/**
 *	Funcion de testing
 *	Descomentar el main sieguiente y correr solo QueueADT.c
 */
/*
int 
main(void){
	QueueADT * queue = newQueue();

	printf("Cola vacia : %s\n",(queueIsEmpty(queue))? "OK" : "FAILED");
	
	int f = 1;
	int g = 2;
	int h = 3;

	enqueue(queue,&f);
	enqueue(queue,&g);
	enqueue(queue,&h);
	
	printf("Size : %s\n",(queue->size == 3)?"OK":"FAILED");
	printf("Cola con datos (isEmpty test) : %s\n", (!queueIsEmpty(queue)) ? "OK" : "FAILED" );

	int *a = (int*)dequeue(queue);//1
	int *b = (int*)dequeue(queue);//2
	int *c = (int*)dequeue(queue);//3

	printf("Enqueue y dequeue : %s\n", (*a==1 && *b==2 && *c==3 && queueIsEmpty(queue))? "OK" : "FAILED" );



	return 0;
}
*/
