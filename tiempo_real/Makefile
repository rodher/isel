CC=gcc
CFLAGS=$(xeno-config --skin posix --cflags --ldflags)
LDFLAGS=
LDLIBS= #-lwiringPi

all: main

main: main.o fsm.o wiringPi.o tasks.o

clean:
	$(RM) *.o *~ main
