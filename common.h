#ifndef COMMON_H
#define COMMON_H

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int creerSocket(int type);

int creerSocketTCP();

int creerSocketUDPMulticast();

int connexion_multicast(char *adresseIP, int portUDP, int *sockUDP, int type, int *longueur_adresse, struct sockaddr_in *adresseUDP);

#endif
