default: all

all: prob1 prob2

prob1:
	gcc -std=c99 -Wall -pthread prob1.c -lpthread -o prob1

prob2:
	gcc -std=c99 -Wall -pthread prob2.c -lpthread -o prob2

clean:
	rm -f prob1
	rm -f prob2

tar:
	tar cvjf CS444_concurrency4_118.tar.bz2 prob1.c prob2.c makefile
