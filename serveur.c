#include "requete.h"
#include "multicastAddr.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define TAILLEBUF 20

int sockUDP, longueur_adresse;
static struct sockaddr_in adresseUDP;

int creerSocket(int type){
  int sock;
  sock = socket(AF_INET, type, 0);
  if (sock == -1) {
    return -1;
  }
  return sock;
}

int creerSocketTCP(){
  return (creerSocket(SOCK_STREAM));
}

int creerSocketUDPMulticast(){
  return (creerSocket(SOCK_DGRAM));
}

int liaisonSocketTCP(int sock, int port){
  static struct sockaddr_in adresse;
  bzero((char *) &adresse, sizeof(adresse));
  adresse.sin_family = AF_INET;
  adresse.sin_port = htons(port);
  adresse.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(sock, (struct sockaddr*)&adresse, sizeof(adresse))== -1) {
    return -1;
  }
  return 0;
}

int connexion_multicast(char * adresseIP, int portUDP){
  struct in_addr ip;
  static struct sockaddr_in ad_multicast;
  struct ip_mreq gr_multicast;
  int reuse;
  sockUDP = socket(AF_INET, SOCK_DGRAM, 0);
  if (inet_aton(adresseIP, &ip) == 0 ){
    printf("Erreur inet_aton : ");
    return -1;
  }
  gr_multicast.imr_multiaddr.s_addr = ip.s_addr;
  gr_multicast.imr_interface.s_addr = htons(INADDR_ANY);
  if (setsockopt(sockUDP, IPPROTO_IP, IP_ADD_MEMBERSHIP, &gr_multicast, sizeof(struct ip_mreq)) == -1) {
    printf("Erreur setsockopt : ");
    return -1;
  }
  reuse = 1;
  if (setsockopt(sockUDP, SOL_SOCKET, SO_REUSEADDR, (int *)&reuse, sizeof(reuse)) == -1) {
    printf("Erreur setsockopt : ");
    return -1;
  }
  bzero((char *) &ad_multicast, sizeof(ad_multicast));
  ad_multicast.sin_family = AF_INET;
  ad_multicast.sin_addr.s_addr = htons(INADDR_ANY);
  ad_multicast.sin_port = htons(portUDP);
  if (bind(sockUDP, (struct sockaddr*)&ad_multicast, sizeof(struct sockaddr_in)) == -1) {
    printf("Erreur bind : ");
    return -1;
  }
  longueur_adresse = sizeof(struct sockaddr_in);
  bzero((char *) &adresseUDP, sizeof(adresseUDP));
  adresseUDP.sin_family = AF_INET;
  adresseUDP.sin_addr.s_addr = ip.s_addr;
  adresseUDP.sin_port = htons(portUDP);
  return 0;
}

char* connexion_tcp(int pseudo){
  char * reponse;
  reponse = (char*)malloc((sizeof(UDPPORT)+strlen(UDPADDR)+1));
  printf("Nouveau client connecte : %d \n", pseudo);
  sprintf(reponse, "%s;%d", UDPADDR, UDPPORT);
  return reponse;
}

void gererClient(int sock_client){
  struct requete req;
  int nb_octets;
  char * message = "test";
  char reponseUDP[TAILLEBUF];
  char description[300];
  char reponseDesc[300];
  int pseudo, prix, reponseReqVente;
  char* res;
  printf(" *** nouveau client connecte ***\n");
  while(1) {
    nb_octets = read(sock_client, &req, sizeof(struct requete));
    if (nb_octets <= 0){
      perror("reception donnees requête");
      break;
    }
    switch (req.type_requete) {
      case CONNEXION_TCP:
        nb_octets = read(sock_client, &pseudo, sizeof(int));
        if (nb_octets <= 0){
          perror("reception donnees pseudo");
          break;
        }
        res = connexion_tcp(pseudo);
        if( write(sock_client, (char *)res, strlen(res)) <= 0){
          perror(" envoi reponse\n");
          break;
        }
        break;
      case REQUETE_VENTE:
        reponseReqVente = 0;
        nb_octets = read(sock_client, description, sizeof(description));
        if (nb_octets <= 0){
            perror("reception donnees description");
            break;
        }
        nb_octets = read(sock_client, &prix, sizeof(int));
        if (nb_octets <= 0){
            perror("reception donnees description");
            break;
        }
        if( write(sock_client, &reponseReqVente, sizeof(int)) <= 0){
            perror(" envoi reponse\n");
            break;
        }
        printf("%s, %d", description, prix);
        sendto(sockUDP, description, strlen(message)+1 , 0, (struct sockaddr*)&adresseUDP, longueur_adresse);
        break;
      default:
      break;
    }
  }
  printf(" *** sortie de la boucle ***\n");
  sendto(sockUDP, message, strlen(message)+1 , 0, (struct sockaddr*)&adresseUDP, longueur_adresse);
  while(1){
    printf("Lu : ");
    recv(sockUDP, (char*)reponseUDP, TAILLEBUF, 0);
    printf("%s\n", reponseUDP );
  }
}

int main(int argc, char *argv[]){
  int socket_ecoute, socket_service;
  static struct sockaddr_in addr_client;
  int lg;
  socket_ecoute = creerSocketTCP();
  if (socket_ecoute == -1){
    perror("creation socket service");
    exit(1);
  }
  if (liaisonSocketTCP(socket_ecoute, atoi(argv[1])) == -1) {
    perror("erreur liaison");
    exit(1);
  }
  if (listen(socket_ecoute, 10) == -1){
    perror("erreur listen");
    exit(1);
  }
  if ((sockUDP = creerSocketUDPMulticast()) == -1){
    perror("creation socket udp");
    return 1;
  }
  if (connexion_multicast(UDPADDR, UDPPORT) == -1){
    perror("erreur connexion multicast");
    return 1;
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
