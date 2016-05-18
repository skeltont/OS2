/*
   	Author: Ty Skelton
   	Class: CS444 - Operating Systems 2
   	Assignment: Concurrency 4 - Problem 2
    	Solution:
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

#define NUM_CUSTOMERS 	10
#define LOBBY_SIZE	5
#define HAIRCUT_TIME	5 
#define HAIRCUT_CHANCE  10

const char *names[NUM_CUSTOMERS] = { "Ty", "Tristan", "Chris", "Jonah", "John", 
		       "Cooper", "Trung", "Heather", "Carlos", "Shane" };

struct monitor {
	struct queue *head;
	sem_t chair;
	int size;
};

struct queue {
	struct person *pers;
	struct queue *next;
	struct queue *prev;
};

struct person {
	struct monitor *mon;
	const char *name;
};

void _add_to_queue(struct queue *link, struct queue *prev, struct person *pers) {
	link->next = prev->next; 
	link->prev = prev;
	link->pers = pers;
	prev->next = link;
}

void _leave_queue(struct monitor *mon, struct person *pers) {
	struct queue *next, *prev, *curr = mon->head->next;

	while (curr != mon->head) {
		if (curr->pers == pers) {
			next = curr->next;
			prev = curr->prev;
			curr->prev->next = next;
			curr->next->prev = prev;
			free(curr);
			mon->size--;
			break;
		}
		curr = curr->next;
	}
}

void _print_queue(struct person *pers) {
	struct queue *curr = pers->mon->head->next;

	printf(KYEL "%s has sat in the lobby, line status: ", pers->name);
	
	while(curr != pers->mon->head) {
		printf(" [%s] ", curr->pers->name);
		curr = curr->next;
	}

	printf(RESET "\n");
}

int _join_queue(struct person *pers) {
	struct queue *seat, *curr = pers->mon->head;

	if (pers->mon->size <= LOBBY_SIZE) {
		seat = malloc(sizeof(struct queue));

		if (curr->next == curr) { 
			_add_to_queue(seat, pers->mon->head, pers);
		} else {
			while(curr->next != pers->mon->head) {
				if (curr->next->next == pers->mon->head) {
					_add_to_queue(seat, curr->next, pers);
					break;
				}
				curr = curr->next;
			}
		}
		pers->mon->size++;
		
		_print_queue(pers);

		return 1;
	}

	return 0;
}

void cut_hair() {
	sleep(HAIRCUT_TIME);
};

void get_hair_cut(struct person *pers) {
	printf(KGRN "%s is the getting their hair cut" RESET "\n",
		pers->name);
	sleep(HAIRCUT_TIME);
};

void ready_for_haircut(struct person *pers) {
	int chair_status;
	int ready = 0;
	
	// block until it's this person's turn
	while(ready == 0) {
		sem_getvalue(&pers->mon->chair, &chair_status);
		if (chair_status == 1 && pers->mon->head->next->pers == pers) ready = 1;
		sleep(1);
	}
}

void *barber(void *m) {
	struct monitor *mon = (struct monitor*) m;
	int chair_status;

	for (;;) {
		sem_getvalue(&mon->chair, &chair_status);
		if (chair_status == 0) {
			cut_hair();
			printf(KRED "Barber has completed the hair cut, ");
			if (mon->size == 0) {
				printf("going to sleep..." RESET "\n");
			} else {
				printf("moving on to next customer." RESET "\n");
			}
		}
	}
}

void *customer(void *p) {
	struct person *pers = (struct person*) p;

	//printf("%s has spawned from the depths of the Uruk Highlands and is considering a haircut\n", pers->name);
	sleep(1);

	for (;;) {
		if ((rand() % HAIRCUT_CHANCE) == 0) {
			//printf("ready for haircut!\n");

			// join queue if not full
			if(!_join_queue(pers)) continue;

			// block until first in line
			ready_for_haircut(pers);
			
			// signal barber
			sem_wait(&pers->mon->chair);

			// leave queue
			_leave_queue(pers->mon, pers);

			// get hair cut
			get_hair_cut(pers);	

			// get out of barber chair
			sem_post(&pers->mon->chair);
		} else {
			sleep(4);
		}
	}
}

void introduction() {
	// title
	printf("\t---------------\n"
	       "\t| Barber shop |\n"
	       "\t---------------\n\n"
	       "\tby Ty Skelton \n\n");

	// info
	printf("\t# of Customers: \t%d\n"
	       "\t# of lobby spots: \t%d\n"
	       "\tchance of haircut: \t%d\%\n"
	       "\thaircut time (sec): \t%d\n\n",
	       NUM_CUSTOMERS, LOBBY_SIZE, 100 / HAIRCUT_CHANCE, HAIRCUT_TIME);

	// customer names
	printf("\tCustomer names: \n\t");
	for(int i = 0; i < NUM_CUSTOMERS; i++) {
		if (i == NUM_CUSTOMERS - 1) {
			printf("& %s.\n\n\n", names[i]);
		} else {
			printf("%s, ", names[i]);
		}
	}
}


int main(int argc, char const *argv[])
{
	struct monitor *mon = malloc(sizeof(struct monitor));
	struct person *curr;
       	int i;
	pthread_t barber_proc, customer_procs[NUM_CUSTOMERS];

	// seed pseudo random number generator
	srand(time(NULL));

	// initialize chair semaphore
	sem_init(&mon->chair, 0, 1);

	// initialize waiting room queue
	mon->head = malloc(sizeof(struct queue));
	mon->head->next = mon->head;
	mon->head->prev = mon->head;

	// introduce the program.
	introduction();

	// spinning up barber thread
	pthread_create(&barber_proc, NULL, barber, (void*) mon);

	// spinning up customer threads and defining their attributes
	for(i = 0; i< NUM_CUSTOMERS; i++) {
		curr = malloc(sizeof(struct person));
		curr->name = names[i];
		curr->mon = mon;
		pthread_create(&customer_procs[i], NULL, customer, (void*) curr);
	}

	// clean up barber and customer threads
	pthread_join(barber_proc, NULL);
	for(i = 0; i < NUM_CUSTOMERS; i++) {
		pthread_join(customer_procs[i], NULL);
	}

	return 0;
}
