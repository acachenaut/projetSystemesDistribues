#include "requete.h"
#include "common.h"
#include "multicastAddr.h"
#include "listeVente.h"
#include "listeClient.h"
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>

int sockUDP, longueur_adresse, venteEnCours, pseudoMeilleurEnchere, prixMeilleurEnchere;
static struct sockaddr_in adresseUDP;
ListeVente *listeVente;
ListeClient *listeClient;
pthread_t thread_UDP;
pthread_t thread_TCP;

int liaisonSocketTCP(int sock, int port) {
    static struct sockaddr_in adresse;
    bzero((char *) &adresse, sizeof(adresse));
    adresse.sin_family = AF_INET;
    adresse.sin_port = htons(port);
    adresse.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *) &adresse, sizeof(adresse)) == -1) {
        return -1;
    }
    return 0;
}

char *nouveauClientTcp(int pseudo, int sock) {
    char *reponse;
    insertionClient(listeClient, pseudo, sock);
    reponse = (char *) malloc((sizeof(UDPPORT) + strlen(UDPADDR) + 1));
    printf("Nouveau client connecte : %d \n", pseudo);
    sprintf(reponse, "%s;%d", UDPADDR, UDPPORT);
    return reponse;
}

void reqObjetInvendu(struct requete_vente *reqVente, int type, int socketClient){
    char* message;
    int taille_msg;
    reqVente->type_requete = OBJET_INVENDU;
    taille_msg = sizeof(struct requete_vente);
    message = (char *) malloc(sizeof(struct requete_vente));
    memcpy(message, reqVente, sizeof(struct requete_vente));
    sendto(sockUDP, message, taille_msg, 0, (struct sockaddr *) &adresseUDP, longueur_adresse);
    switch(type){
        case 1:
            if (write(socketClient, message, taille_msg) <= 0) {
                perror("reponse vendeur");
                break;
            }
            break;
        default:
            break;
    }
    free(message);
}

void reqFinVente(struct requete_vente *reqVente){
    char* message;
    int taille_msg;
    reqVente->type_requete = FIN_VENTE;
    reqVente->id = pseudoMeilleurEnchere;
    reqVente->prix = prixMeilleurEnchere;
    taille_msg = sizeof(struct requete_vente);
    message = (char *) malloc(sizeof(struct requete_vente));
    memcpy(message, reqVente, sizeof(struct requete_vente));
    sendto(sockUDP, message, taille_msg, 0, (struct sockaddr *) &adresseUDP, longueur_adresse);
    free(message);
}

void reqAcquereur(struct requete_vente *reqVente, int socketClient){
    char* message;
    int taille_msg;
    reqVente->type_requete = ACQUEREUR;
    taille_msg = sizeof(struct requete_vente);
    message = (char *) malloc(sizeof(struct requete_vente));
    memcpy(message, reqVente, sizeof(struct requete_vente));
    if (write(socketClient, message, taille_msg) <= 0) {
        perror("reponse acquereur");
    }
    free(message);
}

void reqVendeur(struct requete_vente *reqVente, int socketClient){
    char* message;
    int taille_msg;
    reqVente->type_requete = VENDEUR;
    taille_msg = sizeof(struct requete_vente);
    message = (char *) malloc(sizeof(struct requete_vente));
    memcpy(message, reqVente, sizeof(struct requete_vente));
    if (write(socketClient, message, taille_msg) <= 0) {
        perror("reponse vendeur");
    }
    free(message);
}

void reqRequeteVente(struct requete_vente *reqVente, int sock_client){
    reqVente->type_requete = REQUETE_VENTE;
    reqVente->id = 0;
    if (write(sock_client, reqVente, sizeof(struct requete_vente)) <= 0) {
        perror("envoi réponse\n");
    }
}

void reqNouvelleVente(struct requete_vente *reqVente){
    char* message;
    int taille_msg;
    reqVente->type_requete = NOUVELLE_VENTE;
    reqVente->id = listeVente->premier->vente.id;
    strcpy(reqVente->description, listeVente->premier->vente.description);
    reqVente->prix = listeVente->premier->vente.prix;
    taille_msg = sizeof(struct requete_vente);
    message = (char *) malloc(sizeof(struct requete_vente));
    memcpy(message, reqVente, sizeof(struct requete_vente));
    sendto(sockUDP, message, taille_msg, 0, (struct sockaddr *) &adresseUDP, longueur_adresse);
    free(message);
}

static void *gererClient(void *p_data) {
    struct requete req;
    struct requete_vente reqVente;
    int nb_octets, sock_client, prix, taille_msg, i, pseudo;
    char *message;
    char description[300];
    char *res;
    sock_client = *((int *) p_data);
    printf(" *** nouveau client connecte ***\n");
    while (1) {
        nb_octets = read(sock_client, &req, sizeof(struct requete));
        if (nb_octets <= 0) {
            perror("Réception données requête/Aucune requête");
            break;
        }
        switch (req.type_requete) {
            case CONNEXION_TCP:
                res = nouveauClientTcp(req.id, sock_client);
                pseudo = req.id;
                if (write(sock_client, (char *) res, strlen(res)) <= 0) {
                    perror("envoi réponse\n");
                    break;
                }
                break;
            case REQUETE_VENTE:
                nb_octets = read(sock_client, description, req.taille_requete - sizeof(int));
                if (nb_octets <= 0) {
                    perror("réception données description");
                    break;
                }
                nb_octets = read(sock_client, &prix, sizeof(int));
                if (nb_octets <= 0) {
                    perror("réception données description");
                    break;
                }
                reqRequeteVente(&reqVente, sock_client);
                printf("Nouvelle vente de %d : %s , %d\n", req.id, description, prix);
                reqVente.type_requete = NOUVELLE_VENTE;
                reqVente.id = req.id;
                strcpy(reqVente.description, description);
                reqVente.prix = prix;
                if (nbElementListe(listeVente) == 0) {
                    taille_msg = sizeof(struct requete_vente);
                    message = (char *) malloc(sizeof(struct requete_vente));
                    memcpy(message, &reqVente, sizeof(struct requete_vente));
                    sendto(sockUDP, message, taille_msg, 0, (struct sockaddr *) &adresseUDP, longueur_adresse);
                    venteEnCours = 1;
                    pseudoMeilleurEnchere = reqVente.id;
                    prixMeilleurEnchere = reqVente.prix;
                    free(message);
                }
                insertion(listeVente, reqVente);
                for (i = 0; i < 300; i++) {
                    description[i] = '\0';
                    reqVente.description[i] = '\0';
                }
                break;
            default:
                break;
        }
    }
    suppressionClient(listeClient, pseudo);
    printf(" *** client %d deconnecté ***\n", pseudo);
    return 0;
}

static void *ecouteGroupeMulticast(void *p_data) {
    int pseudoVendeur, socketClient;
    struct requete_vente reqVente;
    while (1) {
        if (venteEnCours) {
            if (recv(sockUDP, &reqVente, sizeof(struct requete_vente), 0) == -1) {
                if (estPresent(listeClient, reqVente.id)) {
                    if ((socketClient = trouverSocket(listeClient, reqVente.id)) != 0 && socketClient != -1) {
                        if (pseudoMeilleurEnchere == reqVente.id && listeVente->premier->vente.prix == reqVente.prix) {
                            reqObjetInvendu(&reqVente, 1, socketClient);
                        } else {
                            reqFinVente(&reqVente);
                            reqAcquereur(&reqVente, socketClient);
                            if ((pseudoVendeur = vendeurVenteEnCours(listeVente)) == -1) {
                                perror("aucune vente en cours");
                                return NULL;
                            }
                            if (estPresent(listeClient, pseudoVendeur)) {
                                if ((socketClient = trouverSocket(listeClient, pseudoVendeur)) != 0 &&
                                    socketClient != -1) {
                                    reqVendeur(&reqVente, socketClient);
                                } else {
                                    perror("recherche socket\n");
                                    return NULL;
                                }
                            } else {
                                printf("vendeur absent\n");
                            }
                        }
                    } else {
                        if (pseudoMeilleurEnchere == reqVente.id && listeVente->premier->vente.prix == reqVente.prix) {
                            reqObjetInvendu(&reqVente, 2, 0);
                        } else {
                            perror("recherche socket\n");
                            return NULL;
                        }
                    }
                } else {
                    if (pseudoMeilleurEnchere == reqVente.id && listeVente->premier->vente.prix == reqVente.prix) {
                        reqObjetInvendu(&reqVente, 2, 0);
                    } else {
                        reqFinVente(&reqVente);
                    }
                    printf("client absent\n");
                }
                venteEnCours = 0;
                pseudoMeilleurEnchere = 0;
                prixMeilleurEnchere = 0;
                suppression(listeVente);
                if (nbElementListe(listeVente) > 0) {
                    printf("Nouvelle vente de %d : %s , %d\n", listeVente->premier->vente.id,
                           listeVente->premier->vente.description, listeVente->premier->vente.prix);
                    reqNouvelleVente(&reqVente);
                    venteEnCours = 1;
                    pseudoMeilleurEnchere = reqVente.id;
                    prixMeilleurEnchere = reqVente.prix;
                }
            } else {
                switch (reqVente.type_requete) {
                    case SURENCHERE:
                        printf("Surenchere de %d : %d €\n", reqVente.id, reqVente.prix);
                        pseudoMeilleurEnchere = reqVente.id;
                        prixMeilleurEnchere = reqVente.prix;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    return 0;
}

static void *connexionClientTcp(void *p_data) {
    int socket_ecoute, socket_service, taille_msg, thread;
    static struct sockaddr_in addr_client;
    socket_ecoute = *((int *) p_data);
    while (1) {
        pthread_t thread_client;
        taille_msg = sizeof(struct sockaddr_in);
        socket_service = accept(socket_ecoute, (struct sockaddr *) &addr_client, (socklen_t * ) & taille_msg);
        thread = pthread_create(&thread_client, NULL, gererClient, &socket_service);
        if (thread) {
            perror("creation thread client");
            return NULL;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int socket_ecoute, thread;
    listeClient = init();
    listeVente = initialiser();
    thread = 0;
    venteEnCours = 0;
    pseudoMeilleurEnchere = 0;
    prixMeilleurEnchere = 0;
    socket_ecoute = creerSocketTCP();
    if (socket_ecoute == -1) {
        perror("creation socket service");
        exit(1);
    }
    if (liaisonSocketTCP(socket_ecoute, atoi(argv[1])) == -1) {
        perror("erreur liaison");
        exit(1);
    }
    if (listen(socket_ecoute, 10) == -1) {
        perror("erreur listen");
        exit(1);
    }
    if ((sockUDP = creerSocketUDPMulticast()) == -1) {
        perror("creation socket udp");
        return 1;
    }
    if (connexion_multicast(UDPADDR, UDPPORT, &sockUDP, 2, &longueur_adresse, &adresseUDP) == -1) {
        perror("erreur connexion multicast");
        return 1;
    }
    thread = pthread_create(&thread_UDP, NULL, ecouteGroupeMulticast, NULL);
    if (!thread) {
        thread = pthread_create(&thread_TCP, NULL, connexionClientTcp, &socket_ecoute);
        if (thread) {
            perror("creation thread tcp");
            return 1;
        }
    } else {
        perror("creation thread udp");
        return 1;
    }
    pthread_join(thread_UDP, NULL);
    pthread_join(thread_TCP, NULL);
    return 0;
}
