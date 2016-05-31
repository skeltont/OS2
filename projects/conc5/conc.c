/*
   	Author: Ty Skelton
   	Class: CS444 - Operating Systems 2
   	Assignment: Concurrency 4 - Problem 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define KNRM  	"\x1B[0m"
#define KRED  	"\x1B[31m"
#define KGRN  	"\x1B[32m"
#define KYEL  	"\x1B[33m"
#define RESET 	"\033[0m"

#define SMOKING_TIME	5 

struct game {
	sem_t avail;
	sem_t tobacco;
	sem_t paper;
	sem_t match;
};

void *agent_1(void *g) {
	struct game *gm = (struct game*) g;

	for(;;) {
		sem_wait(&gm->avail);

		printf(KGRN "Agent 1 made tobacco & paper available! " RESET "\n");

		sem_post(&gm->tobacco);
		sem_post(&gm->paper);

		sleep(1);
	}
}

void *agent_2(void *g) {
	struct game *gm = (struct game*) g;

	for(;;) {
		sem_wait(&gm->avail);

		printf(KGRN "Agent 2 made paper & matches available! " RESET "\n");

		sem_post(&gm->paper);
		sem_post(&gm->match);

		sleep(1);
	}
}

void *agent_3(void *g) {
	struct game *gm = (struct game*) g;

	for(;;) {
		sem_wait(&gm->avail);

		printf(KGRN "Agent 3 made tobacco & matches available! " RESET "\n");

		sem_post(&gm->match);
		sem_post(&gm->tobacco);
		sleep(1);
	}
}

void come_back_and_check() {
	int wait;
	
	wait = rand() % 5;
	sleep(wait);
}


void *smoker_1(void *g) {
	struct game *gm = (struct game*) g;
	int check_paper;

	for (;;) {
		sem_wait(&gm->tobacco);

		printf(KYEL "Smoker with matches picked up the tobacco." RESET "\n");

		sem_getvalue(&gm->paper, &check_paper);
		
		if(check_paper == 1) {
			sem_wait(&gm->paper);

			printf(KRED "Smoker with matches is now smoking." RESET "\n");

			sem_post(&gm->avail);

			sleep(SMOKING_TIME);
		} else {
			printf("Smoker with matches released tobacco.\n");

			sem_post(&gm->tobacco);
			come_back_and_check();
		}	
	}
}

void *smoker_2(void *g) {
	struct game *gm = (struct game*) g;
	int check_matches;
	
	for (;;) {
		sem_wait(&gm->paper);
		
		printf(KYEL "Smoker with tobacco picked up the paper." RESET "\n");
		
		sem_getvalue(&gm->match, &check_matches);
		
		if(check_matches == 1) {
			sem_wait(&gm->match);

			printf(KRED "Smoker with tobacco is now smoking." RESET "\n");

			sem_post(&gm->avail);

			sleep(SMOKING_TIME);
		} else {
			printf("Smoker with tobacco released paper.\n"); 

			sem_post(&gm->paper);
			come_back_and_check();
		}
		
	}
}

void *smoker_3(void *g) {
	struct game *gm = (struct game*) g;
	int check_tobacco;

	for (;;) {
		sem_wait(&gm->match);
		
		printf(KYEL "Smoker with paper picked up the matches." RESET "\n");
		
		sem_getvalue(&gm->tobacco, &check_tobacco);
		
		if(check_tobacco == 1) {
			sem_wait(&gm->tobacco);
			
			printf(KRED "Smoker with paper is now smoking." RESET "\n");

			sem_post(&gm->avail);

			sleep(SMOKING_TIME);
		} else {
			printf("Smoker with paper released matches.\n"); 

			sem_post(&gm->match);
			come_back_and_check();
		}	
	}
}

int main() {
	struct game *gm = malloc(sizeof(struct game));
	pthread_t agent1, agent2, agent3, smoker1, smoker2, smoker3;

	// seed pseudo random number generator
	srand(time(NULL));
	
	sem_init(&gm->avail, 0, 1);
	sem_init(&gm->tobacco, 0, 0);
	sem_init(&gm->paper, 0, 0);
	sem_init(&gm->match, 0, 0);

	// thread create blob
	pthread_create(&agent1, NULL, agent_1, (void*) gm);
	pthread_create(&agent2, NULL, agent_2, (void*) gm);
	pthread_create(&agent3, NULL, agent_3, (void*) gm);
	pthread_create(&smoker1, NULL, smoker_1, (void*) gm);
	pthread_create(&smoker2, NULL, smoker_2, (void*) gm);
	pthread_create(&smoker3, NULL, smoker_3, (void*) gm);

	// thread cleanup blob
	pthread_join(agent1, NULL);
	pthread_join(agent2, NULL);  
	pthread_join(agent3, NULL);  
	pthread_join(smoker1, NULL);  
	pthread_join(smoker2, NULL);  
	pthread_join(smoker3, NULL);  

	return 0;
}










