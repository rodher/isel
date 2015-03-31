CC=gcc
CFLAGS=-g -Wall -O -I.
LDFLAGS=
LDLIBS= #-lrt -lwiringPi

all: main

main: main.o fsm.o wiringPi.o

clean:
	$(RM) *.o *~ main
