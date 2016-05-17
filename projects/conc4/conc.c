/*
   	Author: Ty Skelton
   	Class: CS444 - Operating Systems 2
   	Assignment: Concurrency 3
    	Solution:
 		To solve this concurrency problem, I've implemented a singly
		linked list that is able to have any number of concurrent
		searchers, inserters or deleters.

		As per the instructions, any number of searchers may operate as
		an insertion is being performed, but not while a deletion is
		underway. Insertions are also mutually exclusive with deletions.
		This is controlled by a monitor structure which contains a
		pointer to a head node (even though it's a cyclic list), a lock
		for both read and write operations (searching = read, insert =
		write, delete = read/write), and a running size for sanity
		checks.

		Every searcher, inserter, and deleter (let's call them agents)
		has their own thread of operation and everytime they do a thing
		they print to stdout with their respective color and thread id
		represented in hex form.

		Note: in any event, the deleter will not delete the final node
		in an effort to avoid any strange operations.

		Note: sleep times are to mimic operations and make the terminal
		more readable with outputs.
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

// feel free to change macros to observe different functionality.
#define LIST_SIZE 	7
#define NUM_SEARCHERS	3
#define NUM_INSERTERS	2
#define NUM_DELETERS	2

// the root overseer for this program. possesses a pointer to the list and
// access controls.
struct monitor {
	struct linkedList *head;
	sem_t read;
	sem_t write;
	int size;
};

// list node objects.
struct linkedList {
	int num;
	struct linkedList *next;
};

// allocate resources to a list node. a random value to print and space for a
// new list object.
void initListItem (struct linkedList *lst)
{
	lst->next = malloc(sizeof(struct linkedList));
	lst->num = rand() % 1000;
}

// add a link to the list
void addLink (struct linkedList *head)
{
	struct linkedList *newLink = malloc(sizeof(struct linkedList));
	initListItem(newLink);
	newLink->next = head->next;
	head->next = newLink;
}

// remove the next link from the list
void removeLink (struct linkedList *head)
{
	head->next = head->next->next;
}

// get a link ready to be added
struct linkedList *createLinkedListItem()
{
	struct linkedList *newList = malloc(sizeof(struct linkedList));
	initListItem(newList);
	return(newList);
}

// build our initial linked list based on the size of our list
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

// searcher iterates over the list printing out the num values of the objects
// it reads to the terminal. if it observes a read-lock on the monitor object
// (meaning a delete is in progress) it will block until able to continue.
void *searcher(void *m)
{
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

// inserter also will call addLink when the write lock is not observed. it will
// continue to loop checking for a random chance to occur, telling it to insert
// a link to the list. otherwise it will block.
void *inserter(void *m)
{
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

				printf(KGRN "#%X: inserting, write-locking."
				       RESET "\n", id);
				addLink(curr);
				mon->size++;
				printf(KGRN "#%X: inserted %d, write releasing."
				       "\tlist size = %d" RESET "\n",
				       id, curr->next->num, mon->size);
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

// delete loops over the list at all times, waiting for a random chance to occur
// telling it to delete the next link. if this happens then it will lock both
// read and write access to the list and the rest of the running threads will
// block until this one is finished.
void *deleter(void *m)
{
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

                                printf(KRED "#%X: deleting %d, write/read-locking."
				       RESET "\n", id, curr->next->num);
				removeLink(curr);
				mon->size--;
                                sleep (rand() % 5);
                                printf(KRED "#%X: write/read releasing."
				       "\t\tlist size = %d" RESET "\n",
				       id, mon->size);

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

// abstract out code to wait for threads to terminate.
void cleanUpThreads(pthread_t *procs, int count)
{
	for (int i = 0; i < count; i++) {
		pthread_join(procs[i], NULL);
	}
}

int main(int argc, char const *argv[])
{
	struct monitor *mon = malloc(sizeof(struct monitor));
	int i;
	pthread_t search_procs[NUM_SEARCHERS],
		  insert_procs[NUM_INSERTERS],
		  delete_procs[NUM_DELETERS];

	// seed our pseudo random number generator.
	srand(time(NULL));

	// generate our singly-linked list and set up monitor locks.
	mon->head = createLinkedListItem();
	mon->size = LIST_SIZE + 1;
	generateList(mon->head);
	sem_init(&mon->read, 0, 1);
	sem_init(&mon->write, 0, 1);

	// spin up searchers
	for (i = 0; i < NUM_SEARCHERS; i++) {
		pthread_create(&search_procs[i], NULL, searcher, (void*)mon);
	}

	// spin up inserters
	for (i = 0; i < NUM_INSERTERS; i++) {
		pthread_create(&insert_procs[i], NULL, inserter, (void*)mon);
	}

	// spin up deleters
	for (i = 0; i < NUM_DELETERS; i++) {
		pthread_create(&delete_procs[i], NULL, deleter, (void*)mon);
	}

	// thread termination and clean-up
	cleanUpThreads(search_procs, NUM_SEARCHERS);
	cleanUpThreads(insert_procs, NUM_INSERTERS);
	cleanUpThreads(delete_procs, NUM_DELETERS);

	// huzzah.
	return 0;
}
