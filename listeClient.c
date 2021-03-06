#include "listeClient.h"

ListeClient *init(){
    ListeClient *listeClient = malloc(sizeof(*listeClient));
    listeClient->premier = NULL;
    return listeClient;
}

int insertionClient(ListeClient *clients, int pseudo, int socket){
    Client *nouveau = malloc(sizeof(*nouveau));
    Client *clientActuel;
    if (clients == NULL || nouveau == NULL){
      return -1;
    }
    nouveau->pseudo = pseudo;
    nouveau->socket = socket;
    nouveau->suivant = NULL;

    if(clients->premier != NULL){
      clientActuel = clients->premier;
      while(clientActuel->suivant != NULL){
        clientActuel = clientActuel->suivant;
      }
      clientActuel->suivant = nouveau;
    }
    else{
      clients->premier = nouveau;
    }
    return 0;
}

int suppressionClient(ListeClient *clients, int pseudo){
    Client *clientActuel;
    if(clients == NULL){
      return -1;
    }
    clientActuel = clients->premier;
    if (clientActuel->pseudo == pseudo){
      Client *aSupprimer = clientActuel;
      clients->premier = aSupprimer->suivant;
      free(aSupprimer);
    }
    else {
      while (clientActuel->suivant != NULL && clientActuel->suivant->pseudo != pseudo) {
          clientActuel = clientActuel->suivant;
      }
      if (clientActuel->suivant != NULL) {
          Client *aSupprimer = clientActuel->suivant;
          clientActuel = aSupprimer->suivant;
          free(aSupprimer);
      }
      else {
        clientActuel = NULL;
      }
    }
    return 0;
}

int estPresent(ListeClient *clients, int pseudo){
  Client *clientActuel;
  if(clients == NULL){
    return -1;
  }
  clientActuel = clients->premier;
  if (clientActuel == NULL){
    return 0;
  }
  if (clientActuel->pseudo == pseudo){
    return 1;
  }
  else {
    while (clientActuel->suivant != NULL && clientActuel->suivant->pseudo != pseudo) {
        clientActuel = clientActuel->suivant;
    }
    if (clientActuel->suivant != NULL) {
        return 1;
    }
  }
  return 0;
}

int trouverSocket(ListeClient *clients, int pseudo){
  Client *clientActuel;
  if(clients == NULL){
    return -1;
  }
  clientActuel = clients->premier;
  if (clientActuel->pseudo == pseudo){
    return clientActuel->socket;
  }
  else {
    while (clientActuel->suivant != NULL && clientActuel->suivant->pseudo != pseudo) {
        clientActuel = clientActuel->suivant;
    }
    if (clientActuel->suivant != NULL) {
        return clientActuel->suivant->socket;
    }
  }
  return 0;
}
