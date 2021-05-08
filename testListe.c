#include "listeVente.h"
#include "requete.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

/*test: testListeVente.c listeVente.o
      $(CC) $(CFLAGS) -o $@ $^*/

int main(){
    struct requete_vente req;
    ListeVente* liste = NULL;
    int enCours;
    printf("Bienvenu dans le test\n");
    req.type_requete=REQUETE_VENTE;
    req.id=getpid();
    req.description[0] = 'e';
    req.prix=10;
    printf("before +\n");
    insertion(liste, req);
    printf("before -\n");
    suppression(liste);
    printf("after + et -\n");
    enCours = venteEnCours(liste);
    printf("%d\n", enCours);

    printf("OK\n");
    return 0;
}
