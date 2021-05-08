#ifndef LISTE_VENTE_H
#define LISTE_VENTE_H

#include "requete.h"

typedef struct Vente Vente;
typedef struct ListeVente ListeVente;

struct Vente {
  struct requete_vente *vente;
  Vente *suivant;
};

struct ListeVente{
    Vente *premier;
};

ListeVente *initialisation();

void insertion(ListeVente *liste, struct requete_vente vente);

void suppression(ListeVente *liste);

int venteEnCours(ListeVente *liste, struct requete_vente reqVente));

#endif
