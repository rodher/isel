CC=gcc
CFLAGS=-g -Wall -O -I.
LDFLAGS=
LDLIBS= -lrt -lwiringPi

all: main

main: main.o display.o fsm.o timing.o #wiringPi.o

clean:
	$(RM) *.o *~ main
