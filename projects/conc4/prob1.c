/*
   	Author: Ty Skelton
   	Class: CS444 - Operating Systems 2
   	Assignment: Concurrency 4 - Problem 1
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

#define BUFFER_SIZE 	32
#define CAPACITY    	3
#define NUM_WORKERS	6 

// shared resource data structure
struct resource {
	char *buffer[BUFFER_SIZE];
	sem_t capacity;
	int clear_capacity;
};

// check if more workers can access
int check_capacity(struct resource *res, int capacity) {
	if (res->clear_capacity == 1 && capacity < 3) {
		return 0;
	} else if (capacity == 0) {
	       	res->clear_capacity = 1;
		return 0;
	} else if (capacity == 3) {
		printf("capacity is now empty, allowing all workers\n");
		res->clear_capacity = 0;
	}

	return 1;
}		

// worker thread function
void *worker(void *r) 
{
	struct resource *res = (struct resource*) r;
	unsigned int id = (unsigned int) pthread_self();
	int cap_status;

	printf("Worker #%X starting...\n", id);

	for(;;) {
		if ((rand() % 5) == 0) {
			sem_getvalue(&res->capacity, &cap_status);
			if (check_capacity(res, cap_status) == 1) {
				sem_wait(&res->capacity);
				
				printf(KRED "Worker #%X is using the resource" RESET "\n",
					id);
				
				if (cap_status - 1 == 0) {
					printf("capacity is now full, clearing all workers\n");
					res->clear_capacity = 1;
				}

				sleep(6);


				printf(KGRN "Worker #%X is releasing the resource" RESET "\n",
					id);

				sem_post(&res->capacity);
			}
		} 
		sleep(1);
	}
}

int main(int argc, char const *argv[]) 
{
	struct resource *res = malloc(sizeof(struct resource));
	int i;
	pthread_t worker_procs[NUM_WORKERS];

	// seed pseudo random number generator
	srand(time(NULL));

	// initialize capacity semaphore.
	sem_init(&res->capacity, 0, 3);
	
	// spinning up worker threads.
	for (i = 0; i < NUM_WORKERS; i++) {
		pthread_create(&worker_procs[i], NULL, worker, (void*)res);
	}

	// thread termination and clean-up.
	for (i = 0; i < NUM_WORKERS; i++) {
		pthread_join(worker_procs[i], NULL);
	}

	return 0;
}
