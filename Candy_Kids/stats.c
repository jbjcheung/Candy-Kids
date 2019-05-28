#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

typedef struct stat_t {
   int factnum;
   int made;
   int eaten;
   double min_delay;
   double avg_delay;
   double max_delay;
} stat_t;

stat_t *statistics = NULL;
int numfactories = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void stats_init(int num_producers){
	numfactories = num_producers;
	int dataallocated = sizeof(stat_t) * num_producers;
	statistics = malloc(dataallocated);
	memset(statistics, 0, dataallocated);
	for(int i = 0; i < num_producers; i++){
		statistics[i].factnum = i;
	}
}

void stats_cleanup(void){
	free(statistics);
}

void stats_record_produced(int factory_number){
	
	pthread_mutex_lock(&mutex);
	statistics[factory_number].made = statistics[factory_number].made + 1;
	pthread_mutex_unlock(&mutex);
}

void stats_record_consumed(int factory_number, double delay_in_ms){
	
	pthread_mutex_lock(&mutex);
	statistics[factory_number].eaten = statistics[factory_number].eaten + 1;
	
	if(statistics[factory_number].min_delay == 0){
		statistics[factory_number].min_delay = delay_in_ms;
	}
	if(delay_in_ms < statistics[factory_number].min_delay){
		statistics[factory_number].min_delay = delay_in_ms;
	}
	if(delay_in_ms > statistics[factory_number].max_delay){
		statistics[factory_number].max_delay = delay_in_ms;
	}
	statistics[factory_number].avg_delay = (statistics[factory_number].avg_delay + delay_in_ms)/statistics[factory_number].eaten;
	
	pthread_mutex_unlock(&mutex);
	
}

void stats_display(void){
	printf("Statistics:\n");
	printf("%10s%8s%8s%17s%17s%17s\n", "Factory#", "#Made", "#Eaten", "Min Delay[ms]", "Avg Delay[ms]", "Max Delay[ms]");
	pthread_mutex_lock(&mutex);
	for(int i = 0; i < numfactories; i++){
		if(statistics[i].made != statistics[i].eaten){
			printf("ERROR: Mismatch between number made and eaten\n");
			exit(0);
		}
		
		printf("%10d%8d%8d%17.5f%17.5f%17.5f\n", statistics[i].factnum, statistics[i].made, statistics[i].eaten,statistics[i].min_delay,statistics[i].avg_delay,statistics[i].max_delay);
		
	}
	
	
	pthread_mutex_unlock(&mutex);
	
}





