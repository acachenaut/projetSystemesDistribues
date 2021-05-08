#ifndef REQUETE_H
#define REQUETE_H

enum requete_t {
    FIN = 0,
    CONNEXION_TCP,
    REQUETE_VENTE,
    NOUVELLE_VENTE,
    SURENCHERE,
    CONNEXION_UDP
};

struct requete {
    enum requete_t type_requete;
    int id;
    int taille_requete;
};

struct requete_vente {
    enum requete_t type_requete;
    int id;
    char description[300];
    int prix;
};

char *connexion_tcp(int pseudo);

int requete_vente(char *description, int prix);

#endif
