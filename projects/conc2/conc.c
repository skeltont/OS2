/*
  Author: Ty Skelton
  Class: CS444 - Operating Systems 2
  Assignment: Concurrency 2
  Solution:
      My solution for the dining philosophers problem was to introduce some kind
    of conductor or "scheduler". This entity has local access to all of the
    forks on the table, pointers to all 5 of the philosophers, and a running
    "highest priority" that it checks to make sure philosophers do not starve.
      The philosophers each have a relevant name, indicators for their
    corresponding forks, an enum for the 4 potential statuses they could take
    along their dining experience, and a priority value that increments if
    they've been waiting for a turn to use the forks.
      I first loop five times to initialize all of the philosophers and send
    them on their way in their own threads. As I'm doing this, I'm also
    initializing the conductor struct. Each philosopher starts to
    "philosophize" by initially thinking for a time between 1-20 seconds and
    when they're ready to eat they tell the conductor they announce that they
    are hungry and wait until a fork is available. Once they get the forks they
    need they eat for 2-9 seconds announce that they're done, and allows the
    conductor to come take the forks.
      The conductor is constantly checking each philospher for a change of
    state. It only acts if they're hungry or if they're done. Thinking and
    eating philosophers are left alone. If the philospher is hungry the
    conductor checks his neighboring forks. If both forks aren't available it
    will increment the philosopher's priority value. However, if both forks are
    available then he tells the philopher to eat and flags the forks as
    unavailable. The conductor always stores the highest priority value and
    checks whether or not the current philosopher is >= that value, resetting
    it when it gives away forks. Upon finding a philosopher flagged as done
    the conductor will free up his forks and tell him to start thinking.

    :)
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

const char * const NAMES[] = { "Aristotle", "Seneca", "Augustine", "Descartes", "Jean-Jacques" };

struct philosopher {
  const char *name;
  int lfork;
  int rfork;
  enum { HUNGRY, EATING, THINKING, DONE } state;
  long priority;
};

struct conductor {
  int forks[5];
  struct philosopher *philos[5];
  long high_priority;
};

/*
  generate random number within: base .. ((rand % mod) + base)
*/
int randInt (int mod, int base) {
  int val;
  val = (rand() % mod) + base;
  return val;
}

/*
  tell a philosopher to think for 1 - 20 seconds
*/
void think() {
  sleep(randInt(20, 1));
}

/*
  tell a philosopher to eat for 2 - 9 seconds
*/
void eat() {
  sleep(randInt(8, 2));
}

/*
  start the dining and philosophizing process.
*/
void *philosophize(void *philo) {
  struct philosopher *p = (struct philosopher*)philo;
  while(1) {
    // start thinking
    if (p->state == THINKING) {
      printf("%s is now thinking!\n", p->name);
      think();
      p->state = HUNGRY;
      printf("%s is now hungry!\n", p->name);
    }

    // wait to eat
    if (p->state == EATING) {
      printf("%s is now eating!\n", p->name);
      eat();
      // release forks
      p->state = DONE;
    }
    continue;
  }
  pthread_exit(0);
}

void *conduct (void *c) {
  struct conductor *con = (struct conductor*)c;
  while(1) {
    // check the state of all the philosophers to see if they're hungry.
    for (int i = 0; i < 5; i++) {
      struct philosopher *curr = con->philos[i];
      if (curr->state == HUNGRY && curr->priority >= con->high_priority) {
        if (con->forks[curr->lfork] == 0 && con->forks[curr->rfork] == 0) {
          con->forks[curr->lfork] = 1;
          con->forks[curr->rfork] = 1;
          printf("%s has picked up forks: #%d, #%d\n", curr->name, curr->lfork, curr->rfork);
          curr->state = EATING;
          curr->priority = 0;
        } else {
          curr->priority += 1;
        }
      } else if(curr->state == DONE) {
        con->forks[curr->lfork] = 0;
        con->forks[curr->rfork] = 0;
        printf("%s released forks: #%d, #%d\n", curr->name, curr->lfork, curr->rfork);
        curr->state = THINKING;
      }
    }
  }
  pthread_exit(0);
}

int main(int argc, char const *argv[]) {
  struct philosopher philos[5];
  struct conductor c = {
    .high_priority = 0
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
    pthread_create(&philo_procs[i], NULL, philosophize, (void*)&philos[i]);
  }
  pthread_create(&con_proc, NULL, conduct, (void*)&c);

  for (int i = 0; i < 5; i++) {
    pthread_join(philo_procs[i], NULL);
  }
  pthread_join(con_proc, NULL);

  return 0;
}
