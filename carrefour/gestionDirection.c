#ifndef _GESTIONDIRECTION_C_
#define _GESTIONDIRECTION_C_

#include "gestionDirection.h"

/* Le nombre maximale de voiture qui peuvent arriver dans le carrfour
   par itération. */
#define NOMBRE_ARRIVEE_VOITURE_MAX 10
/* Le nombre maximale de bus qui peuvent arriver dans le carrfour
   par itération. */
#define NOMBRE_ARRIVEE_BUS_MAX 3
/* La probabilité qu'une voiture arrive dans une direction donnée. */
#define PROBA_ARRIVEE_VOITURE 0.75
/* La probabilité qu'un bus arrive dans une directions donnée. */
#define PROBA_ARRIVEE_BUS     0.25
/* La probabilité qu'un véhicule puisse sortir de sa voie de sortie. */
#define PROBA_SORTIE_VEHICULE 0.5
/* La capacité des voies d'entrée. */
#define CAPACITE_VOIE_ENTREE  20
/* La capacité des voies de sortie. */
#define CAPACITE_VOIE_SORTIE  20

/* Retourne un nouveau véhicule avec la direction de départ et la
   direction d'arrivée donnée en argument. */
static vehicule nouveauVehicule(direction directionDepart,
			 direction directionArrivee) {
  vehicule resultat;

  resultat.directionDepart = directionDepart;
  resultat.directionArrivee = directionArrivee;

  return resultat;
}

/* Initialise les voies d'entrée à vide. */
static fileVehicules **initialiserVoiesEntree(const int nombreDeVoies) {
  int i;
  fileVehicules **voiesEntree = NULL;

  voiesEntree = malloc(nombreDeVoies * sizeof(fileVehicules *));

  if (voiesEntree == NULL) {
    fprintf(stderr,
	    "gestionDirection.c : initialiserVoiesEntree : erreur d'initialisation.\n");
  }

  for (i = 0; i < nombreDeVoies; i++) {
    voiesEntree[i] = fileVide(CAPACITE_VOIE_ENTREE);
  }

  return voiesEntree;
}

/* Libère les ressources allouées au voies d'entrée. */
static void libererVoiesEntree(fileVehicules ** voiesEntree, int nombreDeVoies) {
  int i;

  for (i = 0; i < nombreDeVoies; i++) {
    freeFileVehicules(voiesEntree[i]);
  }
  free(voiesEntree);
}

/* Retourne l'état du feu contenu dans la mémoire memoire. */
static feu lireEtatFeux(int memoire) {
  feu *etat = NULL;
  feu resultat;

  etat = shmat(memoire, NULL, 0);
  resultat = *etat;
  shmdt(etat);

  return resultat;
}

/* Attend le passage au vert du feu dans la direction dir, avec son état contenu 
   dans memoireEtatFeux et protégé par semaphoreEtatFeux. */
static void attendrePassageAuVert(direction dir, int memoireEtatFeux,
				  int semaphoreEtatFeux, int semaphoreChangementFeux) {
  int vert = 0;
  feu etatAttendu;

  if (dir == OUEST || dir == EST) {
    etatAttendu = VERT_EST_OUEST;
  } else {
    etatAttendu = VERT_NORD_SUD;
  }

  while (1) {
    P(semaphoreChangementFeux);
    P(semaphoreEtatFeux);
    vert = lireEtatFeux(memoireEtatFeux) == etatAttendu;
    if (vert) {
      V(semaphoreEtatFeux);
      break;
    } else {
      V(semaphoreEtatFeux);
    }
  }
}

static int directionsToTableau(direction directionsPossibles, direction *buffer) {
  int taille = 0;

  if ((NORD | directionsPossibles) == directionsPossibles) {
    buffer[taille] = NORD;
    (taille)++;
  }
  if ((SUD | directionsPossibles) == directionsPossibles) {
    buffer[taille] = SUD;
    (taille)++;
  }
  if ((EST | directionsPossibles) == directionsPossibles) {
    buffer[taille] = EST;
    (taille)++;
  }
  if ((OUEST | directionsPossibles) == directionsPossibles) {
    buffer[taille] = OUEST;
    (taille)++;
  }

  return taille;
}

/* Retourne une direction aléatoire parmi les directions en paramètre. */
static direction directionAleatoire(direction directionsPossibles) {
  int numero;
  direction directions[4];
  int nombreDirs;
  direction resultat;
  
  nombreDirs = directionsToTableau(directionsPossibles, directions);
  numero = rand() % nombreDirs;
  resultat = directions[numero];

  return resultat;
}

/* Ajoute aléatoirement des véhicules dans les voies. */
static void ajouterVehicules(const direction dir, 
			     const direction *tableDirections,
			     int nombreVoiesVoiture, int nombreVoiesBus,
			     fileVehicules **voiesEntree) {
  int i;

  for (i = 0; i < NOMBRE_ARRIVEE_VOITURE_MAX; i++) {
    if (drand48() <= PROBA_ARRIVEE_VOITURE) {
      int voie = rand() % nombreVoiesVoiture;
      direction directionSortie = directionAleatoire(tableDirections[voie]);

      if (!estPleine(*voiesEntree[voie])) {
	
	ajouterVehiculeQueue(voiesEntree[voie], nouveauVehicule(dir, 
								directionSortie));
      }
    }
  }

  for (i = 0; i < NOMBRE_ARRIVEE_BUS_MAX; i++) {
    if (drand48() <= PROBA_ARRIVEE_BUS) {
      int voie = rand() % nombreVoiesBus + nombreVoiesVoiture;
      direction directionSortie = directionAleatoire(tableDirections[voie]);
      if (!estPleine(*voiesEntree[voie])) {
	ajouterVehiculeQueue(voiesEntree[voie], nouveauVehicule(dir, 
								directionSortie));
      }
    }
  }
}

static void envoyerRequeteBus(direction dir, fileVehicules **voiesEntree,
			      int nombreVoiesVoiture, int nombreVoiesBus, 
			      const int* fileRequetesBus) {
  int sontVides = 1;
  int i;
  requeteBus requete;

  if (fileRequetesBus == NULL) {
    return;
  }

  for (i = nombreVoiesVoiture; i < nombreVoiesVoiture + nombreVoiesBus; i++) {
    sontVides = sontVides && estVide(*voiesEntree[i]);
  }

  if (!sontVides) {
    requete.type = 42;
    requete.contenantLeBus = dir;
    msgsnd(*fileRequetesBus, &requete, sizeof(requeteBus), 0);
  }
}

/* Retourne l'état des voies de sortie contenu dans la mémoire memoire. */
static voiesSortie lireEtatVoiesSortie(int memoire) {
  voiesSortie *etat = NULL;
  voiesSortie resultat;

  etat = shmat(memoire, NULL, 0);
  resultat = *etat;
  shmdt(etat);

  return resultat;
}

int nombreVehiculesVoieSortie(direction dir, voiesSortie etatVoiesSortie) {
  if (dir == NORD) {
    return etatVoiesSortie.nombreVehiculesNord;
  } else if (dir == SUD) {
    return etatVoiesSortie.nombreVehiculesSud;
  } else if (dir == EST) {
    return etatVoiesSortie.nombreVehiculesEst;
  } else if (dir == OUEST) {
    return etatVoiesSortie.nombreVehiculesOuest;
  } else {
    fprintf(stderr, "gestionDirection.c : capaciteVoieSortie : direction invalide.");
    exit(EXIT_FAILURE);
  }
}

void incrementerVoieSortie(direction dir, int memoireVoiesSortie) {
  voiesSortie *etat = NULL;

  etat = shmat(memoireVoiesSortie, NULL, 0);
  if (dir == NORD) {
    etat->nombreVehiculesNord--;
  } else if (dir == SUD) {
    etat->nombreVehiculesSud--;
  } else if (dir == EST) {
    etat->nombreVehiculesEst--;
  } else if (dir == OUEST) {
    etat->nombreVehiculesOuest--;
  } else {
    fprintf(stderr, "gestionDirection.c : capaciteVoieSortie : direction invalide.");
    exit(EXIT_FAILURE);
  }
  shmdt(etat);
}

/* Fait passer le véhicule en tête de voie par le carrefour, si
   la voie n'est pas vide et que la voie de sortie demandée n'est
   pas pleine. */
int fairePasserVehicule(fileVehicules *voie,
			 const int memoireVoiesSortie,
			 const int semaphoreVoiesSortie) {
  voiesSortie etatVoiesSortie;
  vehicule vehicule;

  if (estVide(*voie)) {
    return 0;
  } else {
    vehicule = vehiculeTete(*voie);
    P(semaphoreVoiesSortie);
    etatVoiesSortie = lireEtatVoiesSortie(memoireVoiesSortie);
    if (nombreVehiculesVoieSortie(vehicule.directionArrivee, etatVoiesSortie) >=
	CAPACITE_VOIE_SORTIE) {
      V(semaphoreVoiesSortie);
      return 0;
    } else {
      retirerVehiculeTete(voie);
      incrementerVoieSortie(vehicule.directionArrivee, memoireVoiesSortie);
      V(semaphoreVoiesSortie);
      return 1;
    }
  }
}

/* Fait passer les véhicules aléatoirement tant que le feu est vert. */
void fairePasserVehicules(const direction dir,
			  fileVehicules **voiesEntree, 
			  int nombreVoiesVoiture,
			  int nombreVoiesBus,
			  const int memoireVoiesSortie,
			  const int semaphoreVoiesSortie,
			  const int memoireEtatFeux,
			  const int semaphoreEtatFeux) {
  feu etatFeux;
  feu etatAttendu;
  char directionDepart[6];
  char directionArrivee[6];

  if (dir == OUEST || dir == EST) {
    etatAttendu = VERT_NORD_SUD;
  } else {
    etatAttendu = VERT_EST_OUEST;
  }

  while (1) {
    P(semaphoreEtatFeux);
    etatFeux = lireEtatFeux(memoireEtatFeux);
    if (etatFeux == etatAttendu) {
      int voie = rand() % (nombreVoiesVoiture + nombreVoiesBus);
      directionToString(vehiculeTete(*(voiesEntree[voie])).directionDepart, directionDepart);
      directionToString(vehiculeTete(*(voiesEntree[voie])).directionArrivee, directionArrivee);

      if (fairePasserVehicule(voiesEntree[voie], memoireVoiesSortie,
			      semaphoreVoiesSortie)) {
	printf("Passage du vehicule en tête de la voie numero %d de la direction %s dans la voie de sortie direction %s.\n", voie, directionDepart, directionArrivee);
      }
      V(semaphoreEtatFeux);
    } else {
      V(semaphoreEtatFeux);
      break;
    }
  }
}

static void afficherVoiesAvecMessage(char *message, direction dir, fileVehicules **voiesEntree,
				     int nombreVoiesVoiture, int nombreVoiesBus) {
  int i;
  char affichageVoie[2000];
  char direction[6];

  directionToString(dir, direction);
  printf("%s : etat des voies direction %s en entree :\n", message, direction);
  for (i = 0; i < nombreVoiesVoiture; i++) {
    fileVehiculesToString(*(voiesEntree[i]), 0, affichageVoie);
    printf("Voie numero %d : %s\n", i, affichageVoie);
  }

   for (i = nombreVoiesVoiture; i < nombreVoiesVoiture + nombreVoiesBus - 1; i++) {
    fileVehiculesToString(*(voiesEntree[i]), 1, affichageVoie);
    printf("Voie numero %d : %s\n", i, affichageVoie);
  }
}

/* Gère la direction dir, avec nombreVoiesVoiture voies de voitures et nombreVoiesBus
   voies de bus. Les directions de la voie de voiture numero i est donnée dans
   tableDirections[i], et les direction de la voie de bus numero j est donnée dans
   tableDirections[nombreVoiesVoiture + j], où i et j commencent à zero.        
*/
void gestionDirection(const direction dir, const int *fileRequetesBus,
		      const int memoireEtatFeu,
		      const int semaphoreEtatFeux,
		      const int nombreVoiesVoiture,
		      const direction * tableDirections,
		      const int nombreVoiesBus,
		      const int memoireVoiesSortie,
		      const int semaphoreVoiesSortie,
		      const int semaphoreChangementFeux) {
  fileVehicules **voiesEntree =
      initialiserVoiesEntree(nombreVoiesVoiture + nombreVoiesBus);

  srand(time(NULL));
  srand48(time(NULL));

  while (1) {
    ajouterVehicules(dir, tableDirections, nombreVoiesVoiture, nombreVoiesBus,
		     voiesEntree);
    afficherVoiesAvecMessage("Après ajout des véhicules", dir, voiesEntree, nombreVoiesVoiture,
			     nombreVoiesBus);
    envoyerRequeteBus(dir, voiesEntree, nombreVoiesVoiture, nombreVoiesBus, fileRequetesBus);
    afficherVoiesAvecMessage("Attente de passage au vert", dir, voiesEntree, nombreVoiesVoiture,
			     nombreVoiesBus);
    attendrePassageAuVert(dir, memoireEtatFeu, semaphoreEtatFeux, semaphoreChangementFeux);
    fairePasserVehicules(dir, voiesEntree, nombreVoiesVoiture,
			 nombreVoiesBus, memoireVoiesSortie, semaphoreVoiesSortie, 
			 memoireEtatFeu, semaphoreEtatFeux);
    afficherVoiesAvecMessage("Apres avoir fait passer les vehicules", dir, voiesEntree, 
			     nombreVoiesVoiture, nombreVoiesBus);
  }

  libererVoiesEntree(voiesEntree, nombreVoiesVoiture + nombreVoiesBus);
}

#endif
