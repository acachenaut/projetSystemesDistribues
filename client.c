#include "requete.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

#define TAILLEBUF 20
int sock;

int creerSocket(int port, int type){
    sock = socket(AF_INET, type, 0);
    if (sock == -1) {
        return -1;
    }
    return sock;
}

int creerSocketTCP(int port){
    return (creerSocket(port, SOCK_STREAM));
}

int connexion_serveur(char* argv[]){
    static struct sockaddr_in addr_serveur;
    struct hostent *host_serveur;
    int port;
    host_serveur = gethostbyname(argv[1]);
    if (host_serveur==NULL)  {
        perror("Erreur récupération adresse serveur\n");
        return -1;
    }
    bzero((char *) &addr_serveur, sizeof(addr_serveur));
    addr_serveur.sin_family = AF_INET;
    port = atoi(argv[2]);
    addr_serveur.sin_port = htons(port);
    memcpy(&addr_serveur.sin_addr.s_addr, host_serveur->h_addr, host_serveur->h_length);
    if (connect(sock, (struct sockaddr *)&addr_serveur, sizeof(struct sockaddr_in)) == -1) {
        return -1;
    }
    return 0;
}

char* connexion_tcp(int pseudo){
    struct requete req;
    char *message;
    int taille_msg;
    char* reponse = malloc(sizeof(char)* TAILLEBUF);
    req.type_requete = CONNEXION_TCP;
    req.taille_requete = sizeof(int);
    taille_msg = sizeof(struct requete) + sizeof(int);
    message = (char *) malloc(taille_msg);
    memcpy(message, &req, sizeof(struct requete));
    memcpy(message+sizeof(struct requete), &pseudo, sizeof(int));
    if (write(sock, message, taille_msg) <= 0){
        free(message);
        return NULL;
    }
    if (read(sock, (char *)reponse, TAILLEBUF) <= 0){
        free(message);
        return NULL;
    }
    free(message);
    return reponse;
}

int main(int argc, char* argv[]){
    int port;
    char *reponse;
    port = atoi(argv[2]);
    if (creerSocketTCP(port) == -1){
      perror("creation socket");
      return 1;
    }
    if (connexion_serveur(argv) == -1){
      perror("erreur connexion serveur");
      return 1;
    }
    reponse = connexion_tcp(getpid());
    if (reponse == NULL){
        perror("Erreur réception");
    }
    else{
        printf(" reponse recue : %s\n", reponse);
    }
    close(sock);
    return 0;
}
