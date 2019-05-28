#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
#include "bbuff.h"

static void* queue[BUFFER_SIZE];
static int consumer = 0;
static int producer = 0;

static sem_t full;
static sem_t empty;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void bbuff_init(void){
	int sem1 =sem_init(&full, 0, 0);
	if(sem1 == -1){
		printf("Initialization failed");
		exit(-1);
	}
	int sem2 = sem_init(&empty, 0, BUFFER_SIZE);
	if(sem2 == -1){
		printf("Initialization failed");
		exit(-1);
	}
	
}

void bbuff_blocking_insert(void* item){
	sem_wait(&empty);
	pthread_mutex_lock(&mutex);
	
	queue[consumer] = item;
	consumer = consumer + 1;
	if(consumer == BUFFER_SIZE){
		consumer = 0;
	}
	
	pthread_mutex_unlock(&mutex);
	sem_post(&full);
	
}

void* bbuff_blocking_extract(void){
	sem_wait(&full);
	pthread_mutex_lock(&mutex);
	
	void* extracted_candy;
	extracted_candy = queue[producer];
	queue[producer] = NULL;
	producer = producer + 1;
	if(producer == BUFFER_SIZE){
		producer = 0;
	}
	
	pthread_mutex_unlock(&mutex);
	sem_post(&empty);
	
	return extracted_candy;
}

_Bool bbuff_is_empty(void){
	int val;
	sem_getvalue(&full, &val);
	if(val == 0){
		return true;
	}
	return false;
	
}





