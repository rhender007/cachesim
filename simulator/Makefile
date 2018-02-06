CC = cc
CCFLAGS  = -ansi -pedantic -Wall -lm

all: sim.o
	${CC} $(CCFLAGS) -o sim sim.o

clean:
	rm -f *.o sim

sim.o: sim.c sim.h
	${CC} $(CCFLAGS) -lm -c sim.c

