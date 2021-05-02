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

/*int creerSocket(int port, int type)
{
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
int creerSocketUDP(int port){
    return (creerSocket(port, SOCK_DGRAM));
}

int creerSocketTCP(int port){
    return (creerSocket(port, SOCK_STREAM));
}

int connexion_serveur(char* argv[], int sock){
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

int main(int argc, char *argv[]){
    int sock;
    char *message = "bonjour";
    char reponse[TAILLEBUF];
    int port;
    int nb_octets;

    port = atoi(argv[2]);
    if((sock = creerSocketTCP(port)==-1)){
        perror("Erreur création socket");
        return 1;
    }
    if (connexion_serveur(argv, sock) == -1){
        perror("Erreur connexion socket serveur");
        return 1;
    }
    printf(" connexion serveur etablie\n");

    nb_octets = write(sock, message, strlen(message)+1);
    nb_octets = read(sock, reponse, TAILLEBUF);
    if(nb_octets==0){
        perror("Rien reçu");
        return 1;
    }
    printf(" reponse recue : %s\n", reponse);

    close(sock);

    return 0;
}*/

char* connexion_tcp(int pseudo){
    struct requete req;
    char *message;
    int taille_msg;
    char* resultat;
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
    if (read(sock, (char *)&resultat, sizeof(long)) <= 0){
        free(message);
        return NULL;
    }
    free(message);
    return resultat;
}

int main(int argc, char* argv[]){
    static struct sockaddr_in addr_serveur;
    struct hostent *host_serveur;
    int port;
    char pseudo[TAILLEBUF];
    char* reponse;
    sprintf(pseudo,"%d" ,getpid());
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("creation socket");
        exit(1);
    }
    host_serveur = gethostbyname(argv[1]);
    if (host_serveur==NULL)  {
        perror("erreur récupération adresse serveur\n");
        exit(1);
    }
    bzero((char *) &addr_serveur, sizeof(addr_serveur));
    addr_serveur.sin_family = AF_INET;
    port = atoi(argv[2]);
    addr_serveur.sin_port = htons(port);
    memcpy(&addr_serveur.sin_addr.s_addr, host_serveur->h_addr, host_serveur->h_length);
    if (connect(sock, (struct sockaddr *)&addr_serveur, sizeof(struct sockaddr_in)) == -1) {
        perror("erreur connexion serveur");
        exit(1);
    }
    /*nb_octets = write(sock, pseudo, strlen(pseudo)+1);
    nb_octets = read(sock, reponse, TAILLEBUF);
    if(nb_octets==0){
        perror("Rien reçu");
        return 1;
    }*/
    reponse = connexion_tcp(getpid());
    if (reponse == NULL){
        perror("Connexion TCP");
    }
    else{
        printf(" reponse recue : %s\n", reponse);
    }
    close(sock);
    return 0;
}