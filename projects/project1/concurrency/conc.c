#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "mt19937ar.c"

#define KGRN  "\x1B[32m"
#define KRED  "\x1B[31m"
#define KWHT  "\x1B[37m"

struct object {
  int value;
  int sleeptime;
};

struct buffer {
  struct object buf[32];
  pthread_mutex_t bufflock;
  int size;
};

int rdrand_cap;

int randNum(int num1, int num2) {
  int res;
  unsigned long r;

  if (rdrand_cap == 1) {
    init_genrand(time(NULL));
    r = genrand_int32();
    res = (r % num1) + num2;
  } else {
    asm("rdrand %0;" : "=r" (r));
    res = (r % num1) + num2;
  }
  return res;
}

int check_rdrand() {
  int check;
  asm ("sub %%ecx, %%ecx; cpuid;" : "=c" (check));
  if ( 0 == (check & (1 < 29)) ) {
    printf("rdrand not supported\n");
    return 1;
  }
  printf("rdrand supported\n");
  return 0;
}


void* producerProc(void *p) {
  struct buffer *b = (struct buffer*)p;
  struct object o;
  int psleep = 0;

  while (1) {
    // make sure the buffer isn't full.
    if(b->size != 32) {
      o.value = randNum(1000, 0);
      o.sleeptime = randNum(8, 2);

      // tell producer to sleep for 3 - 7 seconds.
      psleep = randNum(5, 3);

      // lock the buffer so producer has control.
      pthread_mutex_lock(&b->bufflock);

      // write object produced to shared buffer.
      b->buf[b->size] = o;
      b->size = b->size + 1;
      printf("%sProduced\tvalue: %d\ttime: %d\tsleeping for: %d\tbuf size: %d\n%s",KGRN, o.value, o.sleeptime, psleep, b->size, KWHT);

      // release buffer lock for the consumer.
      pthread_mutex_unlock(&b->bufflock);

      sleep(psleep);
    } else {
      usleep(1);
    }
  }
  pthread_exit(0);
}

void* consumerProc(void *p) {
  struct buffer *b = (struct buffer*)p;
  struct object o;

  while (1) {
    if(b->size > 0) {
      pthread_mutex_lock(&b->bufflock);

      b->size = b->size - 1;
      o = b->buf[b->size];
      printf("%sConsumed\tvalue: %d\ttime: %d\tsleeping for: %d\tbuf size: %d\n%s",KRED, o.value, o.sleeptime, o.sleeptime, b->size, KWHT);

      // release buffer object
      pthread_mutex_unlock(&b->bufflock);

      // tell consumer to sleep for however long the consumed object says so.
      sleep(o.sleeptime);
    } else {
      usleep(1);
    }
  }
  pthread_exit(0);
}

int main(int argc, char **argv) {
  pthread_t producer, consumer;
  struct buffer b = {
    .bufflock = PTHREAD_MUTEX_INITIALIZER,
    .size = 0
  };

  rdrand_cap = check_rdrand();

  pthread_create(&producer, NULL, producerProc, (void*)&b);
  pthread_create(&consumer, NULL, consumerProc, (void*)&b);

  pthread_join(producer, NULL);
  pthread_join(consumer, NULL);

  pthread_mutex_destroy(&b.bufflock);

  return 0;
}
