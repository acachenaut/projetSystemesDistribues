CC= gcc
CFLAGS= -std=c89 -pedantic -Wall -g -D_GNU_SOURCE
RM= rm -fv


.PHONY: all clean

all: commissaire client
%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<
client: client.c
	$(CC) $(CFLAGS) -o $@ $^
commissaire: serveur.c listeVente.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) *.o *.out commissaire client
