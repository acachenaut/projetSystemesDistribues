#include "common.h"

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

int connexion_multicast(char *adresseIP, int portUDP, int *sockUDP, int type, int *longueur_adresse, struct sockaddr_in *adresseUDP) {
    struct in_addr ip;
    static struct sockaddr_in ad_multicast;
    struct ip_mreq gr_multicast;
    struct timeval tv;
    int reuse;

    *sockUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (inet_aton(adresseIP, &ip) == 0) {
        perror("Erreur inet_aton");
        return -1;
    }
    gr_multicast.imr_multiaddr.s_addr = ip.s_addr;
    gr_multicast.imr_interface.s_addr = htons(INADDR_ANY);
    switch(type){
        case 1:
            if (setsockopt(*sockUDP, IPPROTO_IP, IP_ADD_MEMBERSHIP, &gr_multicast, sizeof(struct ip_mreq)) == -1) {
                perror("C1 Erreur setsockopt");
                return -1;
            }
            reuse = 1;
            if (setsockopt(*sockUDP, SOL_SOCKET, SO_REUSEADDR, (int *) &reuse, sizeof(reuse)) == -1) {
                perror("C2 Erreur setsockopt");
                return -1;
            }
            break;
        case 2:
            if (setsockopt(*sockUDP, IPPROTO_IP, IP_ADD_MEMBERSHIP, &gr_multicast, sizeof(struct ip_mreq)) == -1) {
                perror("S1 Erreur setsockopt");
                return -1;
            }
            reuse = 1;
            if (setsockopt(*sockUDP, SOL_SOCKET, SO_REUSEADDR, (int *) &reuse, sizeof(reuse)) == -1) {
                perror("S2 Erreur setsockopt");
                return -1;
            }
            tv.tv_sec = 5;
            tv.tv_usec = 0;
            if (setsockopt(*sockUDP, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv)) == -1) {
                perror("S3 Erreur setsockopt");
                return -1;
            }
            break;
        default:
            break;
    }
    bzero((char *) &ad_multicast, sizeof(ad_multicast));
    ad_multicast.sin_family = AF_INET;
    ad_multicast.sin_addr.s_addr = htons(INADDR_ANY);
    ad_multicast.sin_port = htons(portUDP);
    if (bind(*sockUDP, (struct sockaddr *) &ad_multicast, sizeof(struct sockaddr_in)) == -1) {
        perror("Erreur bind");
        return -1;
    }
    *longueur_adresse = sizeof(struct sockaddr_in);
    bzero((char *) adresseUDP, sizeof(adresseUDP));
    adresseUDP->sin_family = AF_INET;
    adresseUDP->sin_addr.s_addr = ip.s_addr;
    adresseUDP->sin_port = htons(portUDP);
    return 0;
}