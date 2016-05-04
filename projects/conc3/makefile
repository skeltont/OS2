default: all

all:
	gcc -std=c99 -Wall -pthread conc.c -lpthread -o conc

clean:
	rm -f conc

tar:
	tar cvjf CS444_concurrency3_118.tar.bz2 conc.c makefile
