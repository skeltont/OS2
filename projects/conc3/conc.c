/*
 *  	Author: Ty Skelton
 *  	Class: CS444 - Operating Systems 2
 *  	Assignment: Concurrency 3
 *  	Solution:
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

#define LIST_SIZE 	5
#define SEARCHERS	3
#define INSERTERS	1
#define DELETERS	1

struct monitor {
	struct linkedList *head;
	sem_t read;
	sem_t write;
};

struct linkedList {
	int num;
	struct linkedList *next;
};

void initListItem (struct linkedList *lst, int count)
{
	lst->next = malloc(sizeof(struct linkedList));
	lst->num = count;
}

struct linkedList *createLinkedListItem(int count)
{
	struct linkedList *newList = malloc(sizeof(struct linkedList));
	initListItem(newList, count);
	return(newList);
}

void generateList(struct linkedList *init)
{
	struct linkedList *curr, *prev;
	for (int i = 0; i < LIST_SIZE; i++) {
		curr = createLinkedListItem(i + 1);
		switch (i) {
			case 0:
				init->next = curr;
				break;
			case LIST_SIZE - 1:
				curr->next = init;
				prev->next = curr;
				break;
			default:
				prev->next = curr;
				break;
		}
		prev = curr;
	}
}

void *searcher(void *m) {
	struct linkedList *curr;
	struct monitor *mon;
	int read_status;

	mon = m;
	curr = mon->head;

	for (;;) {
		sem_getvalue(&mon->read, &read_status);
		if (read_status == 1) {
			printf(KYEL "SEARCHER: read %d\n" RESET, curr->num);
			curr = curr->next;
		}
		sleep(1);
	}
}

void *inserter(void *m) {
	struct linkedList *curr;
	struct monitor *mon;
	int write_status;

	mon = m;
	curr = mon->head;

	for (;;) {
		sem_getvalue(&mon->write, &write_status);
		if (write_status == 1) {
			if ((rand() % 3 ) == 0) {
				sem_wait(&mon->write);

				printf(KGRN "INSERTER: inserting, write-locking.\n");
				sleep (5);
				printf(KGRN "INSERTER: write releasing.\n" RESET);

				sem_post(&mon->write);
			} else {
				sleep(1);
			}
			curr = curr->next;
		}
		sleep(1);
	}
}

void *deleter(void *m) {
	struct linkedList *curr;
	struct monitor *mon;
	int write_status;

	mon = m;
	curr = mon->head;

	for(;;) {
		sem_getvalue(&mon->write, &write_status);
		if (write_status == 1 ) {
			if((rand() % 3) == 0) {
				sem_wait(&mon->write);
				sem_wait(&mon->read);

                                printf(KRED "DELETER: inserting, write/read-locking.\n" RESET);
                                sleep (5);
                                printf(KRED "DELETER: write/read releasing.\n" RESET);

                                sem_post(&mon->write);
				sem_post(&mon->read);

			} else {
				sleep(1);
			}
			curr = curr->next;
		}
		sleep(1);
	}
}

// abstract out code to wait for threads to terminate
void cleanUpThreads(pthread_t *procs, int count) {
	for (int i = 0; i < count; i++) {
		pthread_join(procs[i], NULL);
	}
}

int main(int argc, char const *argv[])
{
	struct monitor *mon;
	struct linkedList *init;
	int i;
	pthread_t search_procs[SEARCHERS],
		  insert_procs[INSERTERS],
		  delete_procs[DELETERS];

	// generate our singly-linked list
	init = createLinkedListItem(0);
	generateList(init);

	// set up our monitor object
	mon->head = init;
	sem_init(&mon->read, 0, 1);
	sem_init(&mon->write, 0, 1);

	// spin up searchers
	for (i = 0; i < SEARCHERS; i++) {
		pthread_create(&search_procs[i], NULL, searcher, (void*)mon);
	}

	for (i = 0; i < INSERTERS; i++) {
		pthread_create(&insert_procs[i], NULL, inserter, (void*)mon);
	}

	for (i = 0; i < DELETERS; i++) {
		pthread_create(&delete_procs[i], NULL, deleter, (void*)mon);
	}

	// thread termination and clean-up
	cleanUpThreads(search_procs, SEARCHERS);
	cleanUpThreads(insert_procs, INSERTERS);
	cleanUpThreads(delete_procs, DELETERS);

	return 0;
}
