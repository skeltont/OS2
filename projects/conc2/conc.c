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
