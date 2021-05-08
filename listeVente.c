#include "listeVente.h"

ListeVente *initialisation()
{
    ListeVente *liste = malloc(sizeof(*liste));
    Vente *vente = malloc(sizeof(*element));

    if (liste == NULL || vente == NULL)
    {
        exit(EXIT_FAILURE);
    }

    vente->vente->type_requete = -1;
    vente->vente->id = 0;
    vente->vente->description = "";
    vente->vente->prix = 0;
    vente->suivant = NULL;
    liste->premier = vente;

    return liste;
}

void insertion(ListeVente *liste, struct requete_vente vente)
{
    /* Création du nouvel élément */
    Vente *nouveau = malloc(sizeof(*nouveau));
    if (liste == NULL || nouveau == NULL)
    {
        exit(EXIT_FAILURE);
    }
    nouveau->vente->type_requete = vente.type_requete;
    nouveau->vente->id = vente.id;
    nouveau->vente->description = vente.description;
    nouveau->vente->prix = vente.prix;

    /* Insertion de l'élément au début de la liste */
    nouveau->suivant = liste->premier;
    liste->premier = nouveau;
}

void suppression(ListeVente *liste)
{
    if (liste == NULL)
    {
        exit(EXIT_FAILURE);
    }

    if (liste->premier != NULL)
    {
        Element *aSupprimer = liste->premier;
        liste->premier = liste->premier->suivant;
        free(aSupprimer);
    }
}

int venteEnCours(ListeVente *liste, struct requete_vente reqVente){
  return liste->premier == reqVente 0 ? 0 : 1;
}
