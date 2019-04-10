CC=gcc
CFLAGS=-Wall -lm
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)

all: 
	$(CC) $(CFLAGS) TCPclient.c -o TCPclient.o
	$(CC) $(CFLAGS) TCPserver.c -o TCPserver.o

clean:
	rm $(OBJECTS)