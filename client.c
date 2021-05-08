#include "requete.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define TAILLEBUF 20
#define TAILLEDESCVENTE 300

int sockTCP, sockUDP, longueur_adresse;
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

int connexion_serveur(char * nomServeur, int port){
  static struct sockaddr_in addr_serveur;
  struct hostent *host_serveur;
  host_serveur = gethostbyname(nomServeur);
  if (host_serveur==NULL)  {
    perror("Erreur récupération adresse serveur\n");
    return -1;
  }
  bzero((char *) &addr_serveur, sizeof(addr_serveur));
  addr_serveur.sin_family = AF_INET;
  addr_serveur.sin_port = htons(port);
  memcpy(&addr_serveur.sin_addr.s_addr, host_serveur->h_addr, host_serveur->h_length);
  if (connect(sockTCP, (struct sockaddr *)&addr_serveur, sizeof(struct sockaddr_in)) == -1) {
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
  inet_aton(adresseIP, &ip);
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

char *connexion_tcp(int pseudo){
  struct requete req;
  char *message;
  int taille_msg;
  char *reponse = malloc(sizeof(char)* TAILLEBUF);
  req.type_requete = CONNEXION_TCP;
  req.id = pseudo;
  taille_msg = sizeof(struct requete);
  message = (char *) malloc(taille_msg);
  memcpy(message, &req, sizeof(struct requete));
  if (write(sockTCP, message, taille_msg) <= 0){
    free(message);
    return NULL;
  }
  if (read(sockTCP, (char *)reponse, TAILLEBUF) <= 0){
    free(message);
    return NULL;
  }
  free(message);
  return reponse;
}

int requete_vente(char *description, int prix){
  struct requete req;
  char *message;
  int taille_msg;
  int reponse;
  int pid;
  pid = getpid();
  taille_msg = sizeof(struct requete) + sizeof(int) + strlen(description) + sizeof(int);
  message = (char *) malloc(taille_msg);
  req.type_requete = REQUETE_VENTE;
  req.id = pid;
  req.taille_requete = strlen(description) + sizeof(int);
  memcpy(message, &req, sizeof(struct requete));
  memcpy(message + sizeof(struct requete), description, strlen(description));
  memcpy(message + sizeof(struct requete) + strlen(description), &prix, sizeof(int));
  if (write(sockTCP, message, taille_msg) <= 0){
    free(message);
    return -1;
  }
  if (read(sockTCP, &reponse, sizeof(int)) <= 0){
    free(message);
    return -1;
  }
  free(message);
  return reponse;
}



int main(int argc, char* argv[]){
  int port, portUDP, choix, prixBien, pid, i;
  char * message = "test";
  char *reponse;
  char reponseUDP[TAILLEBUF];
  char * adresseIP = "";
  char descriptionBien[TAILLEDESCVENTE];
  bool venteEnCours;
  char c;
  char saisieTmp[TAILLEDESCVENTE];
  struct requete_vente reqVente;
  port = atoi(argv[2]);
  if ((sockTCP = creerSocketTCP()) == -1){
    perror("creation socket tcp");
    return 1;
  }
  if (connexion_serveur(argv[1], port) == -1){
    perror("erreur connexion serveur");
    return 1;
  }
  reponse = connexion_tcp(getpid());
  if (reponse == NULL){
    perror("Erreur réception");
    return 1;
  }
  else{
    if ((adresseIP = strtok(reponse, ";")) == NULL){
      perror("Erreur serialisation");
      return 1;
    }
    portUDP=atoi((adresseIP+strlen(adresseIP)+1));
  }
  if ((sockUDP = creerSocketUDPMulticast()) == -1){
    perror("creation socket udp");
    return 1;
  }
  if (connexion_multicast(adresseIP, portUDP) == -1){
    perror("erreur connexion multicast");
    return 1;

  }
  pid = fork();
  switch(pid) {
    case -1 :
      perror("Impossible de creer le processus fils\n");
      return 1;
      break;
    case 0 :
      while(1){
        recv(sockUDP, &reqVente, sizeof(struct requete_vente), 0);
        switch (reqVente.type_requete) {
          case NOUVELLE_VENTE:
            printf("Nouvelle vente de %d : %s , %d\n", reqVente.id, reqVente.description, reqVente.prix);
            for(i=0; i<300; i++){
              reqVente.description[i]='\0';
            }
            c = '';
            printf("Voulez-vous participer à la vente ? y/n");
            c = getchar();
            break;
          default:
              break;
          }
      }
      break;
    default :
      choix = -1;
      venteEnCours = 0;
      while(choix != 0){
        choix = -1;
        printf("0 - Quitter la vente aux encheres\n");
        printf("1 - Proposer une vente\n");
        if (venteEnCours){
          printf("2 - Faire une offre\n");
        }
        scanf("%d", &choix);
        switch (choix) {
          case 0:
            close(sockTCP);
            close(sockUDP);
            kill(pid, SIGKILL);
            break;
          case 1:
            printf("Veuillez saisir la description de votre bien : \n");
            while ((c = (char) getchar()) != EOF && c != '\t' && c != '\n' && c != ' ');
            fgets(saisieTmp, TAILLEDESCVENTE, stdin);
            saisieTmp[strlen(saisieTmp) - 1] = '\0';
            strcat(descriptionBien, saisieTmp);
            printf("Veuillez saisir le prix de votre bien en € : \n");
            scanf("%d", &prixBien);
            if(requete_vente(descriptionBien, prixBien) == -1){
              printf("Erreur demande de vente\n");
              return 1;
            }
            for(i=0; i<300; i++){
              descriptionBien[i]='\0';
            }
          break;
        case 2:
          break;
        default:
          break;
      }
    }
    sendto(sockUDP, message, strlen(message)+1 , 0, (struct sockaddr*)&adresseUDP, longueur_adresse);
    recv(sockUDP, (char*)reponseUDP, TAILLEBUF, 0);
    printf("%s\n", reponseUDP );
    close(sockUDP);
    break;
  }
  return 0;
}
