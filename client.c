#include "requete.h"
#include "common.h"
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include <ctype.h>

#define TAILLEBUF 20
#define TAILLEDESCVENTE 300

int sockTCP, sockUDP, longueur_adresse, venteEnCours, participeVente, prixEnchere, requeteVente;
static struct sockaddr_in adresseUDP;
pthread_t thread_UDP;
pthread_t thread_TCP;
pthread_t thread_Menu;

int saisieEntier(int borneMin, int borneMax) {
    char chaine[255];
    int entier = 0;
    int OK, i;
    do {
        OK = 1;
        scanf("%s", chaine);
        for (i = 0; i < strlen(chaine); i++) {
            if (!isdigit(chaine[i])) {
                OK = 0;
                break;
            }
            entier = atoi(chaine);
        }
        if (OK && (entier <= borneMin || entier > borneMax)) {
            OK = 0;
        }
        if (!OK) {
            printf("Veuillez entrer un nombre entier compris entre %d et %d.\n",
                   borneMin + 1, borneMax);
        }
    } while (!OK);
    return entier;
}

int connexion_serveur(char *nomServeur, int port) {
    static struct sockaddr_in addr_serveur;
    struct hostent *host_serveur;
    host_serveur = gethostbyname(nomServeur);
    if (host_serveur == NULL) {
        perror("Erreur récupération adresse serveur\n");
        return -1;
    }
    bzero((char *) &addr_serveur, sizeof(addr_serveur));
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons(port);
    memcpy(&addr_serveur.sin_addr.s_addr, host_serveur->h_addr, host_serveur->h_length);
    if (connect(sockTCP, (struct sockaddr *) &addr_serveur, sizeof(struct sockaddr_in)) == -1) {
        return -1;
    }
    return 0;
}

char *connexion_tcp(int pseudo) {
    struct requete req;
    char *message;
    int taille_msg;
    char *reponse = malloc(sizeof(char) * TAILLEBUF);
    req.type_requete = CONNEXION_TCP;
    req.id = pseudo;
    taille_msg = sizeof(struct requete);
    message = (char *) malloc(taille_msg);
    memcpy(message, &req, sizeof(struct requete));
    if (write(sockTCP, message, taille_msg) <= 0) {
        free(message);
        return NULL;
    }
    if (read(sockTCP, (char *) reponse, TAILLEBUF) <= 0) {
        free(message);
        return NULL;
    }
    free(message);
    return reponse;
}

int requete_vente(char *description, int prix) {
    struct requete req;
    char *message;
    int taille_msg, reponse, pid;
    pid = getpid();
    taille_msg = sizeof(struct requete) + sizeof(int) + strlen(description) + sizeof(int);
    message = (char *) malloc(taille_msg);
    req.type_requete = REQUETE_VENTE;
    req.id = pid;
    req.taille_requete = strlen(description) + sizeof(int);
    memcpy(message, &req, sizeof(struct requete));
    memcpy(message + sizeof(struct requete), description, strlen(description));
    memcpy(message + sizeof(struct requete) + strlen(description), &prix, sizeof(int));
    if (write(sockTCP, message, taille_msg) <= 0) {
        free(message);
        return -1;
    }
    while (requeteVente != 0);
    reponse = requeteVente;
    requeteVente = -1;
    free(message);
    return reponse;
}

void requete_surenchere(int prix) {
    struct requete_vente reqVente;
    char *message;
    int taille_msg;
    int pid;
    pid = getpid();
    taille_msg = sizeof(struct requete_vente);
    message = (char *) malloc(taille_msg);
    reqVente.type_requete = SURENCHERE;
    reqVente.id = pid;
    reqVente.prix = prix;
    memcpy(message, &reqVente, taille_msg);
    sendto(sockUDP, message, taille_msg, 0, (struct sockaddr *) &adresseUDP, longueur_adresse);
    free(message);
}

static void *ecouteGroupeMulticast(void *p_data) {
    int i;
    struct requete_vente reqVente;
    while (1) {
        recv(sockUDP, &reqVente, sizeof(struct requete_vente), 0);
        switch (reqVente.type_requete) {
            case NOUVELLE_VENTE:
                venteEnCours = 1;
                prixEnchere = reqVente.prix;
                printf("Nouvelle vente de %d : %s , %d €\n", reqVente.id, reqVente.description, reqVente.prix);
                for (i = 0; i < 300; i++) {
                    reqVente.description[i] = '\0';
                }
                break;
            case SURENCHERE:
                if (participeVente) {
                    printf("Surenchère de %d : %d €\n", reqVente.id, reqVente.prix);
                    prixEnchere = reqVente.prix;
                }
                break;
            case FIN_VENTE:
                printf("La vente est remportée par %d au prix de %d €\n", reqVente.id, reqVente.prix);
                venteEnCours = 0;
                participeVente = 0;
                prixEnchere = 0;
                break;
            case OBJET_INVENDU:
                printf("L'objet n'a pas trouvé preneur\n");
                venteEnCours = 0;
                participeVente = 0;
                prixEnchere = 0;
            default:
                break;
        }
    }
    return 0;
}

static void *ecouteTcp(void *p_data) {
    struct requete_vente reqVente;
    while (1) {

        if (read(sockTCP, &reqVente, sizeof(struct requete_vente)) <= 0) {
            return NULL;
        }
        switch (reqVente.type_requete) {
            case ACQUEREUR:
                printf("Vous avez gagné l'enchère au prix de %d €\n", reqVente.prix);
                break;
            case VENDEUR:
                printf("Votre objet a été acheté par %d au prix de %d €\n", reqVente.id, reqVente.prix);
                break;
            case OBJET_INVENDU:
                printf("Votre objet n'a pu être vendu\n");
                break;
            case REQUETE_VENTE:
                requeteVente = reqVente.id;
                break;
            default:
                break;
        }
    }
    return 0;
}

static void *gestionMenu(void *p_data) {
    int choix, prixBien, i;
    char saisieTmp[TAILLEDESCVENTE];
    char descriptionBien[TAILLEDESCVENTE];
    char c;
    choix = -1;
    while (choix != 0) {
        choix = -1;
        printf("\n0 - Quitter la vente aux enchères\n");
        printf("1 - Proposer une vente\n");
        printf("2 - Participer a la vente (vous ne pourrez pas proposer de vente tant que vous participez a la vente)\n\n");
        choix = saisieEntier(-1, 2);
        switch (choix) {
            case 0:
                close(sockTCP);
                close(sockUDP);
                pthread_cancel(thread_TCP);
                pthread_cancel(thread_UDP);
                break;
            case 1:
                printf("Veuillez saisir la description de votre bien : \n");
                while ((c = (char) getchar()) != EOF && c != '\t' && c != '\n' && c != ' ');
                fgets(saisieTmp, TAILLEDESCVENTE, stdin);
                saisieTmp[strlen(saisieTmp) - 1] = '\0';
                strcat(descriptionBien, saisieTmp);

                printf("Veuillez saisir le prix de votre bien en € : \n");
                prixBien = saisieEntier(0, INT_MAX);

                if (requete_vente(descriptionBien, prixBien) == -1) {
                    printf("Erreur demande de vente\n");
                    return NULL;
                }
                for (i = 0; i < 300; i++) {
                    descriptionBien[i] = '\0';
                }
                break;
            case 2:
                if (venteEnCours) {
                    participeVente = 1;
                    while (!(choix == 3 || !venteEnCours)) {
                        choix = -1;
                        printf("\n3 - Quitter la vente\n");
                        printf("4 - Faire une enchère\n\n");
                        choix = saisieEntier(2, 4);
                        switch (choix) {
                            case 3:
                                participeVente = 0;
                                break;
                            case 4:
                                printf("Veuillez saisir votre prix :\n");
                                prixBien = saisieEntier(prixEnchere, INT_MAX);
                                requete_surenchere(prixBien);
                                break;
                            default:
                                break;
                        }
                    }
                } else {
                    printf("Aucune vente en cours\n");
                }
                break;
            default:
                break;
        }
    }
    return 0;
}


int main(int argc, char *argv[]) {
    int port, portUDP, thread;
    char *reponse;
    char *adresseIP = "";
    venteEnCours = 0;
    participeVente = 0;
    port = atoi(argv[2]);
    if ((sockTCP = creerSocketTCP()) == -1) {
        perror("creation socket tcp");
        return 1;
    }
    if (connexion_serveur(argv[1], port) == -1) {
        perror("erreur connexion serveur");
        return 1;
    }
    reponse = connexion_tcp(getpid());
    if (reponse == NULL) {
        perror("Erreur réception");
        return 1;
    } else {
        if ((adresseIP = strtok(reponse, ";")) == NULL) {
            perror("Erreur serialisation");
            return 1;
        }
        portUDP = atoi((adresseIP + strlen(adresseIP) + 1));
    }
    if ((sockUDP = creerSocketUDPMulticast()) == -1) {
        perror("creation socket udp");
        return 1;
    }
    if (connexion_multicast(adresseIP, portUDP, &sockUDP, 1, &longueur_adresse, &adresseUDP) == -1) {
        perror("erreur connexion multicast");
        return 1;

    }
    thread = pthread_create(&thread_UDP, NULL, ecouteGroupeMulticast, NULL);
    if (!thread) {
        thread = pthread_create(&thread_Menu, NULL, gestionMenu, NULL);
        if (!thread) {
            thread = pthread_create(&thread_TCP, NULL, ecouteTcp, NULL);
            if (thread) {
                perror("creation thread tcp");
                return 1;
            }
        } else {
            perror("creation thread tcp");
            return 1;
        }
    } else {
        perror("creation thread udp");
        return 1;
    }
    pthread_join(thread_UDP, NULL);
    pthread_join(thread_Menu, NULL);
    pthread_join(thread_TCP, NULL);
    return 0;
}
