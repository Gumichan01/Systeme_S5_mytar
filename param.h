

/**
*
*	@file param.h
*
*	@brief Fichier bibliothèque param.h
*
*	Il gère les paramètres du programme
*
*	@author Luxon JEAN-PIERRE, Kahina RAHANI
*	Licence 3 Informatique
*
*/




#ifndef PARAM_INCLUDED_H
#define PARAM_INCLUDED_H

#include <string.h>

#define MAX_FILE 256	/* Taille max du nom du fichier */

typedef struct Option{

	int flag_h; 	/* flag personnalisé pour l'affichage de l'aide */

	int flag_c;	/* flag création archive */
	int flag_a;	/* flag ajout d'un fichier dans l'archive */
	int flag_d;	/* flag suppression d'un fichier dans l'archive */
	int flag_l;	/* flag liste des fichiers qui sont dans l'archive */
	int flag_x;	/* flag extraction de l'archive */
	int flag_k;	/* flag ne pas remplacer les fichiers existants à l'extraction */
	int flag_s;	/* flag liens symboliques à prendre en compte à l'archivage */
	int flag_C;	/* flag mettre le répertoire mis en paramètre avec -C comme racine */
	int flag_v; /* flag verification intégrité de l'archive */

    /* Extensions */

    int flag_m; /* Pour l'affichage des fichiers avec le md5 si renseigné */
    int flag_z; /* Pour la compression */

	int flag_f; /* flag indiquant le chemin vers un fichier */

}Option;


/*Mets tous les champs à zero*/
void init(Option *sp);


/* Fait une vérification des paramètres du programme
   renvoie un nombre de paramètre >= 0 si tout s'est bien passé, -1 sinon */
int check_param(int argc, char **argv, Option *sp);

/* Stocke le nom du fichier archive dans buf et renvoie l'adresse de buf */
char * getArchive(char *buf,int argc,char **argv);

/* Renvoie la position du premier fichier à archiver */
int getFirstPath(int argc, char **argv);

/*  On récupère le repertoire qui sera la racine
    de l'arborescence dans larchive cible */
char * getRepRoot(char *buf,int argc,char **argv);

#endif /* PARAM_INCLUDED_H */












