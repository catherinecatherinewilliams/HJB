#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#define QUEUE_MAX_SIZE 200000	
typedef struct information{
	char addr[19];
	char time[50];
	 char mac[32] ;
}info;

typedef struct infoQueue{
	info data[QUEUE_MAX_SIZE];
	int front;
	int rear;
}infoQueue;

int EnQueue(infoQueue* q, info data);
int DeQueue(infoQueue* q, info * data);
void InitQueue(infoQueue **q);
int QueueLength(infoQueue *q);
int QueueEmpty(infoQueue *q);

