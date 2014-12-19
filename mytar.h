

/**
*
*	@file mytar.h
*
*	@brief Fichier bibliothèque mytar.h,
*		il contient toutes les bibliothèques utilisables
*
*	@author Luxon JEAN-PIERRE, Kahina RAHANI
*	Licence 3 Informatique
*
*/


#ifndef MYTAR_INCLUDED_H
#define MYTAR_INCLUDED_H

#define _POSIX_C_SOURCE 200112L

/* Bibliothèque standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <time.h>

#include "mytar_posix_lib.h"

#include "lock_lib.h"
#include "param.h"

#ifndef CHECKSUM_SIZE
#define CHECKSUM_SIZE 32	/* Constante definissant la taille du checksum */
#endif

#ifndef BUFSIZE
#define BUFSIZE 1024		/* Constante definissant la taille du buffer */
#endif

#ifndef MAX_PATH
#define MAX_PATH 256		/* Constante definissant la longueur max du nom du fichier */
#endif

#ifndef CHAMPSMAX
#define CHAMPSMAX 32		/* Constante definissant la longueur max du nom du fichier */
#endif

#define USR_RX 0500         /* Droit lecture + parcours(répertoire) ou execution(fichier normal) propriétaire */
#define GRP_RX 0050         /* Droit lecture + parcours(répertoire) ou execution(fichier normal) groupe */
#define OTH_RX 0005         /* Droit lecture + parcours(répertoire) ou execution(fichier normal) autres */


#define USR_RW 0600          /* Droit d'écriture sur un fichier */
#define USR_RWX 0700

#define DEBUG			/* Constante pour le debogage */


typedef struct Entete{

	size_t path_length;		/* longueur du fichier avec '\0' */
	off_t file_length;		/* longueur du contenu */
	mode_t mode;			/* type et droits */
	time_t m_time;			/* date dernière modification du fichier */
	time_t a_time;			/* date dernier accès au fichier */
	char checksum[CHECKSUM_SIZE];	/* checksum du fichier */

}Entete;


void usage(char *prog);

void ecrireEntete(int archive, Entete *info, char *filename);

int creer_archive(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp);		/* creation de l'archive */

void archiver(int archive, char *filename, Parametres *sp);		/* archiver un fichier */
void archiver_rep(int archive, char *rep, Parametres *sp);		/* archiver un répertoire */

/* TODO revoir l'extraction complete de l'archive*/
int extraire_archive(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp);		/* extraction de l'archive */
/* TODO faire l'extraction d'un certain bombre de fichiers de l'archive */

int ajouter_fichier(char *archive_file, int firstPath, int argc, char **argv, Parametres *sp);		/*Ajout du fichier dans l'archive*/
int supprimer_fichiers(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp);	/*Suppression de*/


int liste_fichiers(char *archive_file, Parametres *sp);


/* Içi, la liste de toutes les fonctions annexes */


void ecrire_fichier_sauvegarde(int fdArchive,int fdFichier, Entete *info,char *filename, char *buf, int bufsize);

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

/* Rempli le champs d'information à partir des informations du fichier*/
char *remplirChamps(const Entete *info, char *champs);

#endif /* MYTAR_INCLUDED_H */


















