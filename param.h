

/**
*
*	@file param.h
*
*	@brief Fichier bibliothèque param.h
*
*	Il gère les paramètre du programme
*
*	@author Luxon JEAN-PIERRE, Kahina RAHANI
*	Licence 3 Informatique
*
*/

#ifndef PARAM_INCLUDED_H
#define PARAM_INCLUDED_H

#define MAX_FILE 256	// Taille maxe du nom du fichier

static int flag_h = 0; // flag personnalisé pour l'affichage de l'aide

static int flag_c = 0;	// flag création archive
static int flag_a = 0;	// flag ajout d'un fichier dans l'archive
static int flag_d = 0;	// flag suppression d'un fichier dans l'archive
static int flag_l = 0;	// flag liste des fichiers qui sont dans l'archive
static int flag_x = 0;	// flag extraction de l'archive
static int flag_k = 0;	// flag ne pas remplacer les fichiers existants à l'extraction
static int flag_s = 0;	// flag liens symboliques à prendre en compte à l'archivage
static int flag_C = 0;	// flag mettre le répertoire mis en paramètre avec -C comme racine
static int flag_v = 0;	// flag verification intégrité de l'archive

static int flag_f = 0;	// flag indiquant le chemin vers un fichier

//static char archive_file[MAX_FILE];

#define OPTION_NON_RECONNU -1
#define OPTION_F -2

//#define NO_CREAT_OPTION -3


// Fait une vérification des paramètres du programme
// renvoie un nombre de paramètre >=0 si tout s'est bien passé, -1 sinon
int check_param(int argc, char **argv);

// Stocke le nom du fichier archive dans buf et renvoie l'adresse de buf
char * getArchive(char *buf,int argc,char **argv);

// Renvoie la position du premier fichier sur lequel faire un traitement
int getFirstPath(int argc, char **argv);

#endif // PARAM_INCLUDED_H












