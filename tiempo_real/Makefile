CC=gcc
CFLAGS=-g -Wall -O -I. -DNDEBUG
LDFLAGS=$(xeno-config --skin posix --cflags --ldflags)
LDLIBS= #-lwiringPi

all: main

main: main.o fsm.o reactor.o wiringPi.o tasks.o

clean:
	$(RM) *.o *~ main
