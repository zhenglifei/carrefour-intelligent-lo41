/* Implémentation d'une file FIFO de vehicules, avec une capacite limitee. */
#ifndef _FILE_VEHICULES_H_
#define _FILE_VEHICULES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

/* Une file FIFO contenant des vehicules. */
typedef struct fileVehicules {
  vehicule *vehicules;
  size_t capacite;
  size_t taille;
} fileVehicules;

/* Retourne une file vide avec la capacite donnee en argument. */
fileVehicules *fileVide(size_t capacite);

/* Libere les ressources utilisees par la file passee en argument. */
void freeFileVehicules(fileVehicules * file);

/* Donne la capacite de la file. */
size_t capaciteFile(fileVehicules file);

/* Donne la taille actuelle de la file, c'est a dire son nombre de vehicules. */
size_t tailleFile(fileVehicules file);

/* Retourne vrai si la file est vide, faux sinon. */
int estVide(fileVehicules file);

/* Retourne vrai si la file est pleine, faux sinon. */
int estPleine(fileVehicules file);

/* Ajoute le vehicule aAjouter en queue de la file donnee en argument.
   Retourne faux si la file est pleine ou si l'argument file est NULL, vrai sinon. */
int ajouterVehiculeQueue(fileVehicules * file, vehicule aAjouter);

/* Donne sans le retirer le vehicule en tete de la file. 
   Précondition: la file n'est pas vide. Si cette condition n'est pas valide, 
                 la fonction donne un message d'erreur et lance exit(EXIT_FAILURE). */
vehicule vehiculeTete(fileVehicules file);

/* Donne et retire le vehicule en tete de la file. 
   Précondition: la file n'est pas vide ni NULL. Si cette condition n'est pas valide, 
                 la fonction donne un message d'erreur et lance exit(EXIT_FAILURE). */
vehicule retirerVehiculeTete(fileVehicules * file);

/* Donne une chaîne de caractère décrivant la direction dir. La chaîne est allouée avec
   malloc, et doit par conséquent être libérée avec free. */
char *directionToString(direction dir, char *resultat);

/* Donne une chaîne de caractères décrivant la file de véhicules file, bus indique si
   la file est une file réservée aux bus. La chaîne de caractère est allouée avec malloc,
   et doit par conséquent être libérée avec free. */
char *fileVehiculesToString(fileVehicules file, int bus, char *resultat);

/* Donne une chaîne de caractères représentant le véhicule v, bus indique s'il s'agit. 
   La chaîne de caractère est allouée avec malloc, et doit par conséquent être libérée avec
   free. */
char *vehiculeToString(vehicule v, int bus, char *resultat);

#endif
