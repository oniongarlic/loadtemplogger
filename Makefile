CC=gcc
CFLAGS=-O2 -pipe -Wall

all: loadtemp

loadtemp: loadtemp.o

clean:
	-rm *.o loadtemp
