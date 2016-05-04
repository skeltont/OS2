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

#define LIST_SIZE 	7
#define SEARCHERS	3
#define INSERTERS	2	
#define DELETERS	2


struct monitor {
	struct linkedList *head;
	sem_t read;
	sem_t write;
	int size;
};

struct linkedList {
	int num;
	struct linkedList *next;
};

void initListItem (struct linkedList *lst)
{
	lst->next = malloc(sizeof(struct linkedList));
	lst->num = rand() % 1000;
}

void addLink (struct linkedList *head) {
	struct linkedList *newLink = malloc(sizeof(struct linkedList));
	initListItem(newLink);
	newLink->next = head->next;
	head->next = newLink;
}

void removeLink (struct linkedList *head) {
	head->next = head->next->next;
}

struct linkedList *createLinkedListItem()
{
	struct linkedList *newList = malloc(sizeof(struct linkedList));
	initListItem(newList);
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
	int read_status;
	struct monitor *mon = (struct monitor*)m;
	unsigned int id = (unsigned int) pthread_self();

	curr = mon->head;

	for (;;) {
		sem_getvalue(&mon->read, &read_status);
		if (read_status == 1) {
			printf(KYEL "#%X: read %d" RESET "\n", id, curr->num);
		}
		sleep(rand() % 3);
		curr = curr->next;
	}
}

void *inserter(void *m) {
	struct linkedList *curr;
	int write_status;
	struct monitor *mon = (struct monitor*)m;
	unsigned int id = (unsigned int) pthread_self();

	curr = mon->head;

	for (;;) {
		sem_getvalue(&mon->write, &write_status);
		if (write_status == 1) {
			if ((rand() % 5) == 0) {
				sem_wait(&mon->write);

				printf(KGRN "#%X: inserting, write-locking." RESET "\n", id);
				addLink(curr);
				mon->size++;
				printf(KGRN "#%X: inserted %d, write releasing.\tlist size = %d" RESET "\n", id, curr->next->num, mon->size);
				sleep(rand() % 5);

				sem_post(&mon->write);
			} else {
				sleep(1);
			}
		}
		sleep(1);
		curr = curr->next;
	}
}

void *deleter(void *m) {
	struct linkedList *curr;
	int write_status;
	struct monitor *mon = (struct monitor*)m;
	unsigned int id = (unsigned int) pthread_self();

	curr = mon->head;

	for(;;) {
		sem_getvalue(&mon->write, &write_status);
		if (write_status == 1) {
			if((rand() % 5) == 0 && curr->next != curr) {
				sem_wait(&mon->write);
				sem_wait(&mon->read);

                                printf(KRED "#%X: deleting %d, write/read-locking." RESET "\n", id, curr->next->num);
				removeLink(curr);
				mon->size--;
                                sleep (rand() % 5);
                                printf(KRED "#%X: write/read releasing.\t\tlist size = %d" RESET "\n", id, mon->size);

                                sem_post(&mon->write);
				sem_post(&mon->read);

			} else {
				sleep(1);
			}
		}
		sleep(1);
		curr = curr->next;
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
	struct monitor *mon = malloc(sizeof(struct monitor));
	int i;
	pthread_t search_procs[SEARCHERS],
		  insert_procs[INSERTERS],
		  delete_procs[DELETERS];

	srand(time(NULL));

	// generate our singly-linked list and set up monitor locks
	mon->head = createLinkedListItem();
	mon->size = LIST_SIZE + 1;
	generateList(mon->head);
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
