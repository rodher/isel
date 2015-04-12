CC=gcc
CFLAGS=$(xeno-config --skin posix --cflags)
LDFLAGS=$(xeno-config --skin posix --ldlibs)
LDLIBS= #-lwiringPi

all: main

main: main.o fsm.o wiringPi.o tasks.o

clean:
	$(RM) *.o *~ main
