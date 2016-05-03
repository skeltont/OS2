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

struct linkedList {
	int num;
	sem_t read;
	sem_t write;
	struct linkedList *next;
};

void initListItem (struct linkedList *lst, int count)
{
	lst->next = malloc(sizeof(struct linkedList));
	sem_init(&lst->read, 0, 1);
	sem_init(&lst->write, 0, 1);
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

void *searcher(void *head) {
	struct linkedList *curr;
	int lock_status;
	curr = head;
	for (;;) {
		sem_getvalue(&curr->read, &lock_status);
		if (lock_status == 1) {
			printf(KYEL "SEARCHER: read %d\n" RESET, curr->num);
			curr = curr->next;
		} else {
			printf(KYEL "SEARCHER: %d is a read-locked link, skipping over series.\n" RESET,
				curr->num);
			curr = curr->next->next;
		}
		sleep(1);
	}
}

void *inserter(void *head) {
	struct linkedList *curr;
	int lock_status_curr, lock_status_next;

	curr = head;

	for (;;) {
		sem_getvalue(&curr->write, &lock_status_curr);
		sem_getvalue(&curr->next->write, &lock_status_next);
		if (lock_status_curr == 1 && lock_status_next == 1) {
			if ((rand() % 3 ) == 0) {
				sem_wait(&curr->write);
				sem_wait(&curr->next->write);

				printf(KGRN "INSERTER: inserting, write-locking %d, %d.\n" RESET,
					curr->num, curr->next->num);
				sleep (5);
				printf(KGRN "INSERTER: write releasing %d, %d.\n" RESET,
					curr->num, curr->next->num);

				sem_post(&curr->write);
				sem_post(&curr->next->write);
			} else {
				sleep(1);
			}
			curr = curr->next;
		} else {
			printf(KGRN "INSERTER: %d is a write-locked link, skipping over series.\n" RESET, 
				curr->num);
			curr = curr->next->next;
			sleep(2);
		}
	}
}

void *deleter(void *head) {
	struct linkedList *curr;
	int lock_status_curr, lock_status_next;

	curr = head;

	for(;;) {
		sem_getvalue(&curr->write, &lock_status_curr);
		sem_getvalue(&curr->next->write, &lock_status_next);
		if (lock_status_curr == 1 && lock_status_next == 1) {
			if((rand() % 3) == 0) {
				sem_wait(&curr->write);
                                sem_wait(&curr->next->write);
				sem_wait(&curr->read);
				sem_wait(&curr->next->read);

                                printf(KRED "DELETER: inserting, write/read-locking %d, %d.\n" RESET,
                                        curr->num, curr->next->num);
                                sleep (5);
                                printf(KRED "DELETER: write/read releasing %d, %d.\n" RESET, 
					curr->num, curr->next->num);

                                sem_post(&curr->write);
                                sem_post(&curr->next->write);
				sem_post(&curr->read);
				sem_post(&curr->next->read);
			} else { 
				sleep(1);
			}
			curr = curr->next;
		} else {
			printf(KRED "DELETER: %d is a write-locked link, skipping over series.\n" RESET,
				curr->num);
                        curr = curr->next->next;
                        sleep(2);
		}
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
	struct linkedList *init;
	int i;
	pthread_t search_procs[SEARCHERS],
		  insert_procs[INSERTERS],
		  delete_procs[DELETERS];

	// initialize our singly-linked list stuff
	init = createLinkedListItem(0);
	generateList(init);

	// spin up searchers
	for (i = 0; i < SEARCHERS; i++) {
		pthread_create(&search_procs[i], NULL, searcher, (void*)init);
	}

	for (i = 0; i < INSERTERS; i++) {
		pthread_create(&insert_procs[i], NULL, inserter, (void*)init);
	}

	for (i = 0; i < DELETERS; i++) {
		pthread_create(&delete_procs[i], NULL, deleter, (void*)init);
	}

	// thread termination and clean-up
	cleanUpThreads(search_procs, SEARCHERS);
	cleanUpThreads(insert_procs, INSERTERS);
	cleanUpThreads(delete_procs, DELETERS);

	return 0;
}
