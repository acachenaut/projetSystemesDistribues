#include "listeVente.h"
#include "requete.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

ListeVente *initialiser()
{
    ListeVente *liste = malloc(sizeof(*liste));
    liste->premier = NULL;
    liste->nbElement = 0;

    return liste;
}

int insertion(ListeVente *liste, struct requete_vente vente){
    Vente *nouveau = malloc(sizeof(*nouveau));
    if (liste == NULL || nouveau == NULL){
        return -1;
    }
    nouveau->vente.type_requete = vente.type_requete;
    nouveau->vente.id = vente.id;
    strcpy(nouveau->vente.description, vente.description);
    nouveau->vente.prix = vente.prix;
    nouveau->suivant = NULL;

    if(liste->premier != NULL){
      Vente *venteActuelle = liste->premier;
      while(venteActuelle->suivant != NULL){
        venteActuelle = venteActuelle->suivant;
      }
      venteActuelle->suivant = nouveau;
      liste->nbElement ++;
    }
    else{
      liste->premier = nouveau;
      liste->nbElement = 1;
    }
    return 0;
}

int suppression(ListeVente *liste){
    if (liste == NULL){
        return -1;
    }
    if (liste->premier != NULL){
        /*Vente *aSupprimer = liste->premier;*/
        liste->premier = liste->premier->suivant;
        /*free(aSupprimer);*/
        liste->nbElement--;
    }
    return 0;
}

int venteEnCours(ListeVente *liste){
  return !(liste->nbElement == 1);
}
