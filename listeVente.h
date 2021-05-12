#ifndef LISTE_VENTE_H
#define LISTE_VENTE_H

#include "requete.h"

typedef struct Vente Vente;
typedef struct ListeVente ListeVente;

struct Vente {
  struct requete_vente vente;
  Vente *suivant;
};

struct ListeVente {
    Vente *premier;
    int nbElement;
};

ListeVente *initialiser();

int insertion(ListeVente *liste, struct requete_vente vente);

int suppression(ListeVente *liste);

int nbElementListe(ListeVente *liste);

int vendeurVenteEnCours(ListeVente *liste);

#endif
