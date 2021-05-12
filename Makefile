CC= gcc
CFLAGS= -std=c89 -pedantic -Wall -Werror -g -D_REENTRANT -D_GNU_SOURCE
LIBRARY= -lpthread
RM= rm -fv


.PHONY: all clean

all: commissaire client
%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<
client: client.c common.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBRARY)
commissaire: serveur.c listeVente.o listeClient.o common.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBRARY)

clean:
	$(RM) *.o *.out commissaire client
