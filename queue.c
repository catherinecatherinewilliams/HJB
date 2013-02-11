#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "queue.h"
infoQueue *infoQ;
infoQueue *unSendQ;
 pthread_mutex_t queueMutex;
int EnQueue(infoQueue* q, info data)
{
	pthread_mutex_lock(&queueMutex);
	if((q->rear+1)%QUEUE_MAX_SIZE == q->front)
	{
		pthread_mutex_unlock(&queueMutex);
		return 0;
	}
	q->rear=(q->rear+1)%QUEUE_MAX_SIZE;
	q->data[q->rear] = data;
	pthread_mutex_unlock(&queueMutex);
	return 1;
}

int DeQueue(infoQueue* q, info * data)
{
	pthread_mutex_lock(&queueMutex);
	if(q->front == q->rear)
	{
		pthread_mutex_unlock(&queueMutex);
		return 0;
	}
	q->front = (q->front+1)%QUEUE_MAX_SIZE;
	memcpy(data,&(q->data[q->front]),sizeof(info));
	pthread_mutex_unlock(&queueMutex);
	return 1;
}

void InitQueue(infoQueue **q)
{
	
	*q = (infoQueue *)malloc(sizeof(infoQueue));
	(*q)->front = (*q)->rear = 0;
}

int QueueLength(infoQueue *q)
{
	return q->rear-q->front;
}

int QueueEmpty(infoQueue *q)
{
	return(q->front == q->rear);
}
