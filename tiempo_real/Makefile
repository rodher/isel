CC=gcc
CFLAGS=$(shell xeno-config --skin posix --cflags)
LDLIBS=$(shell xeno-config --skin posix --ldflags) -lwiringPi
#LDLIBS= -lrt #-lwiringPi

all: main

main: main.o fsm.o wiringPi.o tasks.o

clean:
	$(RM) *.o *~ main
