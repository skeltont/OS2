/*
 *  Author: Ty Skelton
 *  Class: CS444 - Operating Systems 2
 *  Assignment: Concurrency 2
 *  Solution:
 *    My solution for the dining philosophers problem was to introduce some kind
 *  of conductor or "scheduler". This entity has local access to all of the
 *  forks on the table, pointers to all 5 of the philosophers, and a running
 *  "highest priority" that it checks to make sure philosophers do not starve.
 *    The philosophers each have a relevant name, indicators for their
 *  corresponding forks, an enum for the 4 potential statuses they could take
 *  along their dining experience, and a priority value that increments if
 *  they've been waiting for a turn to use the forks.
 *    I first loop five times to initialize all of the philosophers and send
 *  them on their way in their own threads. As I'm doing this, I'm also
 *  initializing the conductor struct. Each philosopher starts to
 *  "philosophize" by initially thinking for a time between 1-20 seconds and
 *  when they're ready to eat they tell the conductor they announce that they
 *  are hungry and wait until a fork is available. Once they get the forks they
 *  need they eat for 2-9 seconds announce that they're done, and allows the
 *  conductor to come take the forks.
 *    The conductor is constantly checking each philospher for a change of
 *  state. It only acts if they're hungry or if they're done. Thinking and
 *  eating philosophers are left alone. If the philospher is hungry the
 *  conductor checks his neighboring forks. If both forks aren't available it
 *  will increment the philosopher's priority value. However, if both forks are
 *  available then he tells the philopher to eat and flags the forks as
 *  unavailable. The conductor always stores the highest priority value and
 *  checks whether or not the current philosopher is >= that value, resetting
 *  it when it gives away forks. Upon finding a philosopher flagged as done
 *  the conductor will free up his forks and tell him to start thinking.
 *
 *  :)
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define RESET "\033[0m"

// a list of relevant names for our philosophers.
const char * const NAMES[] = {
        "Aristotle",
        "Seneca",
        "Augustine",
        "Descartes",
        "Jean-Jacques"
};

// data structure for our philosophers.
struct philosopher {
          const char *name;
          int lfork;
          int rfork;
          enum { HUNGRY, EATING, THINKING, DONE } state;
          long priority;
};

// data structure for our scheduler, the conductor.
struct conductor {
          int forks[5];
          struct philosopher *philos[5];
          long priority;
};

// generate random number within: base .. ((rand % mod) + base).
int randInt (int mod, int base)
{
          int val;
          val = (rand() % mod) + base;
          return val;
}

// tell a philosopher to think for 1 - 20 seconds.
void think()
{
        sleep(randInt(20, 1));
}

// tell a philosopher to eat for 2 - 9 seconds.
void eat()
{
        sleep(randInt(8, 2));
}

// start the dining and philosophizing process.
void *philosophize(void *philo)
{
        struct philosopher *p = (struct philosopher*)philo;
        while(1) {
                // start thinking
                if (p->state == THINKING) {
                        printf(KNRM "%s is now thinking!\n" RESET, p->name);
                        think();
                        // tell conductor that we are hungry.
                        printf(KRED "%s is now hungry!\n" RESET, p->name);
                        p->state = HUNGRY;

                }
                // wait until conductor reserves forks and says start eating.
                if (p->state == EATING) {
                        printf(KGRN "%s is now eating!\n" RESET, p->name);
                        eat();
                        // tell conductor that we're done with the forks.
                        p->state = DONE;
                }
                continue;
        }
        pthread_exit(0);
}

void *conduct (void *c)
{
        struct conductor *con = (struct conductor*)c;
        struct philosopher *curr = NULL;
        while(1) {
                // check the state of all the philosophers.
                for (int i = 0; i < 5; i++) {
                        curr = con->philos[i];
                        if (curr->state == HUNGRY
                        && curr->priority >= con->priority) {
                                if (con->forks[curr->lfork] == 0
                                && con->forks[curr->rfork] == 0) {
                                        // reserve forks
                                        con->forks[curr->lfork] = 1;
                                        con->forks[curr->rfork] = 1;
                                        printf(KYEL "%s has picked up forks: "
                                               "#%d, #%d\n" RESET,
                                               curr->name,
                                               curr->lfork,
                                               curr->rfork);

                                        // tell philosopher to eat
                                        // reset priority.
                                        curr->state = EATING;
                                        curr->priority = 0;
                                        con->priority = 0;
                                } else {
                                        // philosopher has been waiting.
                                        if (curr->priority > con->priority) {
                                                con->priority = curr->priority;
                                        }
                                        curr->priority += 1;
                                }
                        } else if(curr->state == DONE) {
                                // release forks
                                con->forks[curr->lfork] = 0;
                                con->forks[curr->rfork] = 0;
                                printf(KYEL "%s released forks: "
                                       "#%d, #%d\n" RESET,
                                       curr->name,
                                       curr->lfork,
                                       curr->rfork);

                                // tell philosopher to get back to thinking.
                                curr->state = THINKING;
                        }
                }
        }
        pthread_exit(0);
}

int main(int argc, char const *argv[])
{
        struct philosopher philos[5];
        struct conductor c = {
                .priority = 0
        };
        pthread_t con_proc, philo_procs[5];

        // assign values to philosophers and the conuctor and
        // then start things off.
        for (int i = 0; i < 5; i++) {
                philos[i].name = NAMES[i];
                philos[i].lfork = (i) % 5;
                philos[i].rfork = (i + 1) % 5;
                philos[i].state = THINKING;
                philos[i].priority = 0;
                c.forks[i] = 0;
                c.philos[i] = &philos[i];
                pthread_create(&philo_procs[i],
                               NULL,
                               philosophize,
                               (void*)&philos[i]);
        }
        pthread_create(&con_proc, NULL, conduct, (void*)&c);

        for (int i = 0; i < 5; i++) {
                pthread_join(philo_procs[i], NULL);
        }
        pthread_join(con_proc, NULL);

        return 0;
}
