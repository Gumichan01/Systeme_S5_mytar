

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
#include "annexe.h"
#include "param.h"


#ifndef BUFSIZE
#define BUFSIZE 2048		/* Constante definissant la taille du buffer */
#endif


#ifndef CHAMPSMAX
#define CHAMPSMAX 32		/* Constante definissant la longueur des buffer de remplirChamps */
#endif

#define USR_RX 0500         /* Droit lecture + parcours(répertoire) ou execution(fichier normal) propriétaire */
#define GRP_RX 0050         /* Droit lecture + parcours(répertoire) ou execution(fichier normal) groupe */
#define OTH_RX 0005         /* Droit lecture + parcours(répertoire) ou execution(fichier normal) autres */


#define USR_RW 0600          /* Droit d'écriture sur un fichier */
#define USR_RWX 0700         /* Droit de lecture et d'écriture */

/*#define DEBUG*/			/* Constante pour le debogage */


typedef struct Entete{

	size_t path_length;		        /* longueur du fichier avec '\0' */
	off_t file_length;		        /* longueur du contenu */
	mode_t mode;			        /* type et droits */
	time_t m_time;			        /* date dernière modification du fichier */
	time_t a_time;			        /* date dernier accès au fichier */
	char checksum[CHECKSUM_SIZE];	/* checksum du fichier */

}Entete;


void usage(char *prog);

int ecrireEntete(int archive, Entete *info, char *filename);

/* (option '-c') */
int creer_archive(char *archive_file, int firstPath,int argc, char **argv, Option *sp);

int archiver(int archive, char *filename,char *root, Option *sp);   /* Archiver un fichier */
int archiver_rep(int archive, char *rep, char *root, Option *sp);   /* Archiver une arborescence */

/* Fait l'extraction de l'archive (option '-x') */
int extraire_archive(char *archive_file, int firstPath,int argc, char **argv, Option *sp);

/* Ajoute un ou plusieurs fichiers dans l'archive (option '-a') */
int ajouter_fichier(char *archive_file, int firstPath, int argc, char **argv, Option *sp);

/* Supprime les fichiers à supprimer (option '-d') */
int supprimer_fichiers(char *archive_file, int firstPath,int argc, char **argv, Option *sp);

/* Affiche la liste des fichiers (option '-l') */
int liste_fichiers(char *archive_file, Option *sp, int argc, char **argv);

/* Ecrire le contenu d'un fichier dans un autre */
int ecrire_fichier_sauvegarde(int fdArchive,int fdFichier, Entete *info,char *filename, char *buf, int bufsize);

/* Rempli le champs d'information à partir des informations du fichier */
char *remplirChamps(const Entete *info, char *champs);


#endif /* MYTAR_INCLUDED_H */


















