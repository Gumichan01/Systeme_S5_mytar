

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

// Bibliothèque standard
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "mytar_posix_lib.h"

#include "lock_lib.h"
#include "param.h"

#ifndef CHECKSUM_SIZE
#define CHECKSUM_SIZE 32	// Constante definissant la taille du checksum 
#endif


typedef struct Entete{

	size_t path_length;		// longueur du fichier avec '\0';
	off_t file_length;		// longueur du contenu
	mode_t mode;			// type et droits
	time_t m_time;			// date dernière modification du fichier
	time_t a_time;			// date dernier accès au fichier
	char checksum[CHECKSUM_SIZE];	// checksum du fichier

}Entete;


void usage(char *prog);					// usage
int creer_archive(char *archive_file, int param,int argc, char **argv);		// creation de l'archive

int archiver(int archive, char *filename);		// archiver un fichier  
int archiver_rep(int archive, char *rep);		// archiver un répertoire

int extraire_archive(char *archive_file);		// extraction de l'archive

int ajouter_fichier(char *archive_file, char *filename);
int supprimer_fichier(char *archive_file, char *filename);

int verifier_archive(char *archive_file);


#endif // MYTAR_INCLUDED_H


















