CC=gcc
CFLAGS=-O2 -pipe -Wall
LIBS=-lm

all: loadtemp

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	-rm *.o loadtemp
