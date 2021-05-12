#ifndef REQUETE_H
#define REQUETE_H

enum requete_t {
    CONNEXION_TCP=1,
    REQUETE_VENTE,
    NOUVELLE_VENTE,
    SURENCHERE,
    ACQUEREUR,
    VENDEUR,
    OBJET_INVENDU,
    FIN_VENTE
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

#endif
