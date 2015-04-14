CC=gcc
CFLAGS=$(shell xeno-config --skin posix --cflags)
LDFLAGS=$(shell xeno-config --skin posix --ldlibs)
LDLIBS= -lrt #-lwiringPi

all: main

main: main.o fsm.o wiringPi.o tasks.o

clean:
	$(RM) *.o *~ main
