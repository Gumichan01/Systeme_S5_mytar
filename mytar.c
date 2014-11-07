

/**
*
*	@file mytar.h
*
*	@brief Fichier mytar.c, il contient les définitions de fonctions
*
*	@author Luxon JEAN-PIERRE, Kahina RAHANI
*	Licence 3 Informatique
*
*/

// @note : en dehors de la vérification, toutes les fonctions seront 
// des reprises des fonctions du TP 6 de système

#include "mytar.h"



// usage
void usage(char * prog)
{
	// TODO améliorer la fonction usage() pour que ce soit plus clair pour l'tutilisateur

	if(prog != NULL)
	{
		fprintf(stderr,"usage : %s [-c|a|d|l|x|k|s|C <rep>|v] -f <archive> <noms_fichiers> \n", prog);
		//fprintf(stderr,"\t%s [-xf] <archive> \n", prog);
	}
	exit(EXIT_FAILURE);
}

// archiveè=_file : fichier archive; param : position du premier fichier path
int creer_archive(char *archive_file, int param,int argc, char **argv)	// creation de l'archive
{
	// TODO création archive :  int creer_archive(char *archive_file, int param,int argc, char **argv)
	// @note tester flag_c et flag_s
	return 0;
}


int archiver(int archive, char *filename)				// archiver un fichier  
{
	// TODO ajouter un fichier dans l'archive : int archiver(int archive, char *filename)
	// @note tester flag_s
	return 0;
}


int archiver_rep(int archive, char *rep)				// archiver un répertoire
{
	// TODO ajouter un répertoire : int archiver_rep(int archive, char *rep)
	return 0;
}



int extraire_archive(char *archive_file)				// extraction de l'archive
{
	// TODO extraction archive : int extraire_archive(char *archive_file)
	// @note tester flag_k
	return 0;
}

int ajouter_fichier(char *archive_file, char *filename)
{
	// TODO ajout d'un fichier dans l'archive : int ajouter_fichier(char *archive_file, char *filename)
	// @note tester flag_a
	return 0;
}


int supprimer_fichier(char *archive_file, char *filename)
{
	// TODO suppression d'un fichier dans l'archive : int supprimer_fichier(char *archive_file, char *filename)
	// @note tester flag_d
	return 0;
}


int verifier_archive(char *archive_file)
{
	// TODO verifie l'integrité de l'archive : int verifier_archive(int archive)
	// @note tester flag_v
	return 0;
}














