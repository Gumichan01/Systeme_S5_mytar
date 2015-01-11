

/**
*
*	@file annexe.h
*
*	@brief Fichier bibliothèque annexe.h
*
*	Il contient les définitions des fonctions auxiliaires
*   utilisées par mytar
*
*	@author Luxon JEAN-PIERRE, Kahina RAHANI
*	Licence 3 Informatique
*
*/

#ifndef ANNEXE_INCLUDED_H
#define ANNEXE_INCLUDED_H

#ifndef CHECKSUM_SIZE
#define CHECKSUM_SIZE 32	/* Constante definissant la taille du checksum */
#endif

#ifndef MAX_PATH
#define MAX_PATH 256		/* Constante definissant la longueur max du nom du fichier */
#endif



/* Fonction qui calcul le md5 du ficheir pris en paramètre*/
char * md5sum(const char *filename, char *checksum);

/* Verifie si le md5 d'un fichier est renseigné*/
int checksumRenseigne(char * checksum);

/* Supprimer les "../" , "./" || le '/' de début de chaine */
char *enleverSlashEtPoints(char *oldchaine, char *newchaine);

/* Créer une arborescence à partir du chemin passé en paramètre*/
int mkdirP(char *arborescence);

/*  Permet d'obtenir l'arborescence à laquelle appartient
    un fichier tel que ce fichier ne soit pas un répertoire */
char *getArborescence(char *filename, char *newA);


char *catRoot(char *rootRep,char *newF);



#endif
