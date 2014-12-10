

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

#include "mytar_posix_lib.h"

#include "lock_lib.h"
#include "param.h"

#ifndef CHECKSUM_SIZE
#define CHECKSUM_SIZE 32	/* Constante definissant la taille du checksum */
#endif

#ifndef BUFSIZE
#define BUFSIZE 1024		/* Constante definissant la taille du checksum */
#endif

#define USR_W 0100	/* Droit d'écriture pour le propriétaire */
#define GRP_W 0010	/* Droit d'écriture pour le groupe */
#define OTH_W 0001	/* Droit d'écriture pour les autres */

#define USR_RX 0500
#define GRP_RX 0050
#define OTH_RX 0005


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
									/* usage */
int creer_archive(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp);		/* creation de l'archive */

void archiver(int archive, char *filename, Parametres *sp);		/* archiver un fichier */
void archiver_rep(int archive, char *rep, Parametres *sp);		/* archiver un répertoire */

/* TODO revoir l'extraction complete de l'archive*/
int extraire_archive(char *archive_file, Parametres *sp);		/* extraction de l'archive */
/* TODO faire l'extraction d'un certain bombre de fichiers de l'archive */

int ajouter_fichier(char *archive_file, int firstPath, int argc, char **argv, Parametres *sp);		/*Ajout du fichier dans l'archive*/
int supprimer_fichiers(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp);	/*Suppression de*/

int liste_fichiers(char *archive_file, Parametres *sp);

void ecrire_fichier_sauvegarde(int fdArchive,int fdFichier, Entete *info,char *filename, char *buf, int bufsize);

char * md5sum(const char *filename, char *checksum);	/* Fonction qui calcul le md5 du ficheir pris en paramètre*/

/* Supprimer les ".." || le '/' de début de chaine */
/*char suprimerParasites(char * newF, char oldF);*/

#endif /* MYTAR_INCLUDED_H */


















