#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "bbuff.h"
#include <time.h>
#include <pthread.h>
#include "stats.h"

typedef struct  {
   int factory_number;
   double creation_ts_ms;
} candy_t;

double current_time_in_ms(void)
{
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return now.tv_sec * 1000.0 + now.tv_nsec/1000000.0;
}

_Bool stopfactories = false;
_Bool stopkids = false;

void* factorythread(void* arg){
	
	int factorynum = *((int *)arg);
	
	while (!stopfactories) {
		int randomnum = rand() % 4;
		printf("Factory %d ships candy and waits %ds \n",factorynum,randomnum);
		candy_t *candy = malloc(sizeof(candy_t));
		candy->factory_number = factorynum;
		candy->creation_ts_ms = current_time_in_ms();
		bbuff_blocking_insert(candy);
		stats_record_produced(candy->factory_number);
		sleep(randomnum);
	}
	printf("Candy-factory %d done\n", factorynum);
	pthread_exit(0);
	
}

void* kidthread(){
	while (!stopkids) {
		int randomnum = rand() % 2;
		candy_t* extracted_candy = bbuff_blocking_extract();
		if(extracted_candy != NULL){
		stats_record_consumed(extracted_candy -> factory_number, current_time_in_ms() - extracted_candy -> creation_ts_ms);
		free(extracted_candy);
		}
		sleep(randomnum);
	}
	pthread_exit(0);
}


int main(int argc, char *argv[]) {

	int factories = atoi(argv[1]);
	int kids = atoi(argv[2]);
	int seconds = atoi(argv[3]);
	
	if(factories <= 0){
		printf("Error, Invalid Value");
		exit(-1);
	}
	if(kids <= 0){
		printf("Error, Invalid Value");
		exit(-1);
	}
	if(seconds <= 0){
		printf("Error, Invalid Value");
		exit(-1);
	}
	
	srand(time(0));
	stats_init(factories);
	bbuff_init();
	pthread_t idfactories[factories];
	int args[factories];
	pthread_t idkids[kids];
	
	for(int i = 0; i < factories; i++){
		args[i] = i;
		pthread_create(&idfactories[i], NULL, factorythread, &args[i]);
	}
	for(int i = 0; i < kids; i++){
		pthread_create(&idkids[i], NULL, kidthread, NULL);
	}
	
	for(int j = 0; j < seconds; j++){
		printf("Time %d:\n", j);
		sleep(1);
	}
	printf("Factories have stopped running\n");
	stopfactories = true;
	
	for(int i = 0; i < factories; i++){
		pthread_join(idfactories[i], NULL);
	}
	printf("Waiting for all candy to be consumed\n");
	while(bbuff_is_empty() != true){
		sleep(1);
	}
	stopkids= true;
	
	for(int i = 0; i < kids; i++){
		pthread_cancel(idkids[i]);
		pthread_join(idkids[i], NULL);
	}
	
	stats_display();
	stats_cleanup();
	

}