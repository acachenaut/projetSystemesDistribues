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
#include <signal.h>

#define TAILLEBUF 20

/*int main(int argc, char* argv[]){
    static struct sockaddr_in addr_client;
    static struct sockaddr_in addr_serveur;
    int lg_addr, opt, port;
    int socket_ecoute, socket_service;
    char message[TAILLEBUF];
    char *chaine_recue;
    char *reponse = "bien recu";
    int nb_octets;
    socket_ecoute = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ecoute == -1) {
        perror("creation socket");
        exit(1);
    }
    bzero((char *) &addr_serveur, sizeof(addr_serveur));
    opt = 1;
    if (setsockopt(socket_ecoute, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    addr_serveur.sin_family = AF_INET;
    port = atoi(argv[1]);
    addr_serveur.sin_port = htons(port);
    addr_serveur.sin_addr.s_addr=htonl(INADDR_ANY);
    if( bind(socket_ecoute, (struct sockaddr*)&addr_serveur, sizeof(addr_serveur))== -1 ) {
        perror("erreur bind socket écoute");
        exit(1);
    }
    if (listen(socket_ecoute, 5) == -1) {
        perror("erreur listen");
        exit(1);
    }
    lg_addr = sizeof(struct sockaddr_in);
    socket_service = accept(socket_ecoute, (struct sockaddr *) &addr_client, (socklen_t *) &lg_addr);
    if (socket_service == -1) {
        perror("erreur accept");
        exit(1);
    }

    nb_octets = read(socket_service, message, TAILLEBUF);
    chaine_recue =(char *)malloc(nb_octets * sizeof(char));
    memcpy(chaine_recue, message, nb_octets);
    printf("reçu pseudo %s\n", chaine_recue);
    write(socket_service, reponse, strlen(reponse)+1);
    close(socket_service);
    close(socket_ecoute);

    return 0;
}*/

int creerSocket(int port, int type){
    static struct sockaddr_in adresse;
    int sock;
    sock = socket(AF_INET, type, 0);
    if (sock == -1) {
        return -1;
    }
    bzero((char *) &adresse, sizeof(adresse));
    adresse.sin_family = AF_INET;
    adresse.sin_port = htons(port);
    adresse.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr*)&adresse, sizeof(adresse))== -1) {
        return -1;
    }
        return sock;
}

int creerSocketTCP(int port){
    return (creerSocket(port, SOCK_STREAM));
}

char* connexion_tcp(int pseudo){
    char* reponse = "bien reçu";
    return reponse;
}

void gererClient(int sock_client){
    struct requete req;
    int nb_octets;
    printf(" *** nouveau client connecte ***\n");
    while(1) {
        nb_octets = read(sock_client, &req, sizeof(struct requete));
        if (nb_octets <= 0){
            perror(" reception donnees");
            break;
        }
        if (req.type_requete==CONNEXION_TCP){
            int pseudo;
            char* res;
            nb_octets = read(sock_client, &pseudo, sizeof(int));
            if (nb_octets <= 0){
                perror(" reception donnees");
                break;
            }
            res = connexion_tcp(pseudo);
            if( write(sock_client, (char *)&res, sizeof(res)) <= 0){
                perror(" envoi reponse\n");
                break;
            }
        }
    }
    printf(" *** sortie de la boucle ***\n");
}

int main(int argc, char *argv[]){
    int socket_ecoute, socket_service;
    static struct sockaddr_in addr_client;
    int lg;
    socket_ecoute = creerSocketTCP(atoi(argv[1]));
    if (socket_ecoute == -1){
        perror("creation socket service");
        exit(1);
    }
    if (listen(socket_ecoute, 10) == -1){
        perror("erreur listen");
        exit(1);
    }
    signal(SIGCHLD, SIG_IGN);
    while(1){
        lg = sizeof(struct sockaddr_in);
        socket_service = accept(socket_ecoute,(struct sockaddr*)&addr_client,(socklen_t *) &lg);
        if (fork()==0){
            close(socket_ecoute);
            gererClient(socket_service);
            close(socket_service);
            exit(0);
        }
        close(socket_service);
    }
}