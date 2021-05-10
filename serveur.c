#include "requete.h"
#include "multicastAddr.h"
#include "listeVente.h"
#include "listeClient.h"
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
#include <sys/ipc.h>
#include <sys/mman.h>
#include <fcntl.h>

#define NOM_FICHIER "serveur.txt"

int sockUDP, longueur_adresse;
static struct sockaddr_in adresseUDP;
static struct ListeVente *listeVente;
static struct ListeClient *listeClient;
FILE *fptr;

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
  struct timeval tv;
  int reuse;
  sockUDP = socket(AF_INET, SOCK_DGRAM, 0);
  if (inet_aton(adresseIP, &ip) == 0 ){
    perror("Erreur inet_aton");
    return -1;
  }
  gr_multicast.imr_multiaddr.s_addr = ip.s_addr;
  gr_multicast.imr_interface.s_addr = htons(INADDR_ANY);
  if (setsockopt(sockUDP, IPPROTO_IP, IP_ADD_MEMBERSHIP, &gr_multicast, sizeof(struct ip_mreq)) == -1) {
    perror("Erreur setsockopt");
    return -1;
  }
  reuse = 1;
  if (setsockopt(sockUDP, SOL_SOCKET, SO_REUSEADDR, (int *)&reuse, sizeof(reuse)) == -1) {
    perror("Erreur setsockopt");
    return -1;
  }
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  if (setsockopt(sockUDP, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) == -1) {
    perror("Erreur setsockopt");
    return -1;
  }
  bzero((char *) &ad_multicast, sizeof(ad_multicast));
  ad_multicast.sin_family = AF_INET;
  ad_multicast.sin_addr.s_addr = htons(INADDR_ANY);
  ad_multicast.sin_port = htons(portUDP);
  if (bind(sockUDP, (struct sockaddr*)&ad_multicast, sizeof(struct sockaddr_in)) == -1) {
    perror("Erreur bind");
    return -1;
  }
  longueur_adresse = sizeof(struct sockaddr_in);
  bzero((char *) &adresseUDP, sizeof(adresseUDP));
  adresseUDP.sin_family = AF_INET;
  adresseUDP.sin_addr.s_addr = ip.s_addr;
  adresseUDP.sin_port = htons(portUDP);
  return 0;
}

char* nouveauClientTcp(int pseudo, int sock){
  char * reponse;
  insertionClient(listeClient, pseudo, sock);
  reponse = (char*)malloc((sizeof(UDPPORT)+strlen(UDPADDR)+1));
  printf("Nouveau client connecte : %d \n", pseudo);
  sprintf(reponse, "%s;%d", UDPADDR, UDPPORT);
  return reponse;
}

void gererClient(int sock_client){
  struct requete req;
  struct requete_vente reqVente;
  int nb_octets;
  char *message;
  char description[300];
  int prix, reponseReqVente, taille_msg, i;
  char* res;
  printf(" *** nouveau client connecte ***\n");
  while(1) {
    nb_octets = read(sock_client, &req, sizeof(struct requete));
    if (nb_octets <= 0){
      perror("Reception donnees requête/Aucune requête");
      break;
    }
    switch (req.type_requete) {
      case CONNEXION_TCP:
        res = nouveauClientTcp(req.id, sock_client);
        if( write(sock_client, (char *)res, strlen(res)) <= 0){
          perror("envoi reponse\n");
          break;
        }
        break;
      case REQUETE_VENTE:
        reponseReqVente = 0;
        nb_octets = read(sock_client, description, req.taille_requete-sizeof(int));
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
            perror("envoi reponse\n");
            break;
        }
        printf("Nouvelle vente de %d : %s , %d\n", req.id, description, prix);
        reqVente.type_requete = NOUVELLE_VENTE;
        reqVente.id = req.id;
        strcpy(reqVente.description, description);
        reqVente.prix = prix;
        if (insertion(listeVente, reqVente)) {
          perror("insertion dans la liste");
          break;
        }
        printf("Pseudo liste %d\n", listeVente->premier->vente.id);
        printf("Nb vente %d\n", listeVente->nbElement);
        if (!venteEnCours(listeVente)){
          taille_msg = sizeof(struct requete_vente);
          message = (char *) malloc(sizeof(struct requete_vente));
          memcpy(message, &reqVente, sizeof(struct requete_vente));
          sendto(sockUDP, message, taille_msg , 0, (struct sockaddr*)&adresseUDP, longueur_adresse);
          fptr = fopen(NOM_FICHIER, "w+");
          putw(1, fptr);
          putw(reqVente.id, fptr);
          putw(reqVente.prix, fptr);
          fclose(fptr);
          free(message);
        }
        for(i = 0; i<300; i++){
          description[i] = '\0';
          reqVente.description[i]='\0';
        }
        break;
      default:
        break;
    }
  }
  if (estPresent(listeClient, req.id)){
    if (suppressionClient(listeClient, req.id) == -1){
      perror("suppression du client");
    }
  }
  else{
    printf("Client pas present dans la liste\n");
  }
  printf(" *** sortie de la boucle ***\n");
}

int main(int argc, char *argv[]){
  int socket_ecoute, socket_service, taille_msg, pid;
  static struct sockaddr_in addr_client;
  struct requete_vente reqVente;
  char *message = "";
  char *nomFichierServeur = NOM_FICHIER;
  fptr = NULL;
  listeVente = initialiser();
  listeClient = init();
  if((fptr = fopen(nomFichierServeur, "w"))==NULL){
    perror("erreur fichier");
    return 1;
  }
  putw(0, fptr);
  putw(0, fptr);
  putw(0, fptr);
  fclose(fptr);
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
  listeVente = mmap (NULL, (10*sizeof(struct ListeVente)+10*sizeof(struct Vente)+10*sizeof(struct requete_vente)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
  if (listeVente == MAP_FAILED){
    perror("creation map\n");
    return 1;
  }
  listeClient = mmap (NULL, (10*sizeof(ListeClient)+10*sizeof(Client)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
  if (listeClient == MAP_FAILED){
    perror("creation map\n");
    return 1;
  }
  signal(SIGCHLD, SIG_IGN);
  pid = fork();
  switch (pid){
    case -1 :
      perror("Impossible de creer le processus fils\n");
      return 1;
      break;
    case 0 :
      while(1){
        fptr = fopen(nomFichierServeur, "r");
        if (getw(fptr)){
          fclose(fptr);
          if (recv(sockUDP, &reqVente, sizeof(struct requete_vente), 0) == -1){
            fptr = fopen(nomFichierServeur, "r");
            getw(fptr);
            reqVente.type_requete = FIN_VENTE;
            reqVente.id = getw(fptr);
            reqVente.prix = getw(fptr);
            fclose(fptr);
            taille_msg = sizeof(struct requete_vente);
            message = (char *) malloc(sizeof(struct requete_vente));
            memcpy(message, &reqVente, sizeof(struct requete_vente));
            sendto(sockUDP, message, taille_msg , 0, (struct sockaddr*)&adresseUDP, longueur_adresse);
            free(message);
            printf("Pseudo : %d\n", reqVente.id);
            printf("Pseudo liste %d\n", listeVente->premier->vente.id);
            printf("Trouver socket %d\n", trouverSocket(listeClient, reqVente.id));
            /*reqVente.type_requete = ACQUEREUR;
            taille_msg = sizeof(struct requete_vente);
            message = (char *) malloc(sizeof(struct requete_vente));
            memcpy(message, &reqVente, sizeof(struct requete_vente));
            if( write(trouverSocket(listeClient, reqVente.id), message, taille_msg) <= 0){
              perror("reponse acquereur\n");
              break;
            }
            free(message);
            reqVente.type_requete = VENDEUR;
            taille_msg = sizeof(struct requete_vente);
            message = (char *) malloc(sizeof(struct requete_vente));
            memcpy(message, &reqVente, sizeof(struct requete_vente));
            if( write(listeVente->premier->vente.id, message, taille_msg) <= 0){
              perror("reponse acquereur\n");
              break;
            }
            free(message);*/
            fptr = fopen(nomFichierServeur, "w+");
            putw(0, fptr);
            putw(0, fptr);
            putw(0, fptr);
            fclose(fptr);
            suppression(listeVente);
            printf("Nb apres suppr %d\n", listeVente->nbElement);
            if (listeVente->nbElement > 0){
              reqVente.type_requete = NOUVELLE_VENTE;
              reqVente.id = listeVente->premier->vente.id;
              strcpy(reqVente.description, listeVente->premier->vente.description);
              reqVente.prix = listeVente->premier->vente.prix;
              taille_msg = sizeof(struct requete_vente);
              message = (char *) malloc(sizeof(struct requete_vente));
              memcpy(message, &reqVente, sizeof(struct requete_vente));
              sendto(sockUDP, message, taille_msg , 0, (struct sockaddr*)&adresseUDP, longueur_adresse);
              free(message);
              fptr = fopen(nomFichierServeur, "w+");
              putw(1, fptr);
              putw(0, fptr);
              putw(0, fptr);
              fclose(fptr);
            }
          }
          else{
            switch (reqVente.type_requete) {
              case SURENCHERE:
                printf("Surenchere de %d : %d €\n", reqVente.id, reqVente.prix);
                fptr = fopen(nomFichierServeur, "w+");
                putw(1, fptr);
                putw(reqVente.id, fptr);
                putw(reqVente.prix, fptr);
                fclose(fptr);
                break;
              default:
                break;
              }
          }
        }
        else {
          fclose(fptr);
        }
      }
      break;
    default :
      while(1){
        taille_msg = sizeof(struct sockaddr_in);
        socket_service = accept(socket_ecoute,(struct sockaddr*)&addr_client,(socklen_t *) &taille_msg);
        if (fork()==0){
          close(socket_ecoute);
          gererClient(socket_service);
          close(socket_service);
          exit(0);
        }
        close(socket_service);
      }
      break;
  }
  return 0;
}
