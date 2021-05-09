#ifndef LISTE_CLIENT_H
#define LISTE_CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

typedef struct Client Client;
typedef struct ListeClient  ListeClient;

struct Client {
  int pseudo;
  int socket;
  Client *suivant;
};

struct ListeClient {
    Client *premier;
};

ListeClient *init();

int insertionClient(ListeClient *clients, int pseudo, int socket);

int suppressionClient(ListeClient *clients, int pseudo);

int estPresent(ListeClient *clients, int pseudo);

int trouverSocket(ListeClient *clients, int pseudo);

#endif
