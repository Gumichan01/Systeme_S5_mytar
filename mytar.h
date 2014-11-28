

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




#define DEBUG			/* Constante pour le debogage */


typedef struct Entete{

	size_t path_length;		/* longueur du fichier avec '\0' */
	off_t file_length;		/* longueur du contenu */
	mode_t mode;			/* type et droits */
	time_t m_time;			/* date dernière modification du fichier */
	time_t a_time;			/* date dernier accès au fichier */
	char checksum[CHECKSUM_SIZE +1];	/* checksum du fichier */

}Entete;


void usage(char *prog);	

void ecrireEntete(int archive, Entete *info, char *filename);
									/* usage */
int creer_archive(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp);		/* creation de l'archive */

void archiver(int archive, char *filename, Parametres *sp);		/* archiver un fichier */
void archiver_rep(int archive, char *rep, Parametres *sp);		/* archiver un répertoire */

int extraire_archive(char *archive_file, Parametres *sp);		/* extraction de l'archive */

int ajouter_fichier(char *archive_file, int firstPath, int argc, char **argv, Parametres *sp);
int supprimer_fichiers(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp);

void ecrire_fichier_sauvegarde(int fdArchive,int fdFichier, Entete *info,char *filename, char *buf, int bufsize);


#endif /* MYTAR_INCLUDED_H */


















