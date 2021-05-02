#ifndef REQUETE_H
#define REQUETE_H

enum requete_t {
    FIN = 0,
    CONNEXION_TCP,
    CONNEXION_UDP,
    DEMANDE_VENTE,
    VENTE
};

struct requete {
    enum requete_t type_requete;
    int taille_requete;
};

#endif