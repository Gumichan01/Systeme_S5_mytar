

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
	// TODO améliorer la fonction usage() pour que ce soit plus clair pour l'utilisateur

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
	// @note tester flag_s
	int i;
	int archive;

#ifdef DEBUG
	printf("DEBUG : Ouverture du fichier %s\n", archive_file);
#endif

	if(!flag_c)
	{
		fprintf(stderr,"%s, création de l'archive non permise, option '-c' non detectée ",argv[0]);
		return -1;
	}


	// On ouvre l'archive
	if((archive = open(archive_file, O_WRONLY | O_TRUNC | O_CREAT, 0660)) == -1 )
	{
		warn("Problème à la création de l'archive %s ",archive_file);
		return -1;
	}

#ifdef DEBUG
	printf("DEBUG : Fichier %s ouvert, OK\n", archive_file);
#endif

	// On met dans l'archive tous les fichiers
	for(i = param; i < argc ; i++)
	{

#ifdef DEBUG
	printf("DEBUG : Lecture du fichier %s\n", argv[i]);
#endif

		archiver(archive,argv[i]);
	}

	close(archive);

	return 0;
}


void archiver(int archive, char *filename)				// archiver un fichier  
{
	// TODO mettre le checksum puis prendre en compte l'option '-s' : void archiver(int archive, char *filename)
	// @note tester flag_s
	int j;
	off_t debut;

	struct stat s;
	int fdInput; 	// fd en lecture

	Entete info;

	char buf[BUFSIZE];
	int lus;


	if(lstat(filename,&s) == -1)
	{
		fprintf(stderr,"%s : fichier inexistant \n", filename);
	}
	else
	{
		// On récupère les informations du fichier puis on les mets dans l'entete
		info.path_length = strlen(filename) +1;
		info.file_length = s.st_size;
		info.mode = s.st_mode;
		info.m_time = s.st_mtime;
		info.a_time = s.st_atime;

		// On ecrit dans l'archive
		debut = lseek(archive,0, SEEK_CUR);	// On garde la position du début

		write(archive,&info.path_length,sizeof(size_t));	// ecrire la taille
		write(archive,&info.file_length,sizeof(off_t));		// longueur du contenu
		write(archive,&info.mode,sizeof(mode_t));	// ecrire le mode
		write(archive,&info.m_time,sizeof(time_t)); 
		write(archive,&info.a_time,sizeof(time_t));
		//write(archive,&info.checksum,sizeof(&info.checksum));

		write(archive, filename, info.path_length);	// ecrire le nom du fichier

		// Si c'est un lien, on met juste le nom du fichier pointé par le lien
		if(S_ISLNK(info.mode) > 0)
		{
			lus = readlink(filename,buf,BUFSIZE);

			if(lus == -1)
			{
				warn(" %s : ", filename);
			}
			else
			{
				buf[lus] = '\0';
				write(archive,buf, strlen(buf));
			}

		}
		else if(S_ISDIR(info.mode) > 0) 
		{
			// On archive tous les fichiers qui sont dans ce repertoire
			archiver_rep(archive,filename);
		}
		else	// Sinon on lit son contenu
		{
			fdInput = open(filename,O_RDONLY);

			if(fdInput == -1)
			{
				fprintf(stderr," %s : erreur à l'ouverture ", filename);
				warn("Erreur à l'ouverture de %s ",filename);
			}
			else
			{
				for(j = 0 ; j <= info.file_length/BUFSIZE; j++)
				{
					lus = read(fdInput,buf, BUFSIZE);
					write(archive,buf,lus);

					if(lus != BUFSIZE && info.file_length != (BUFSIZE*j) + lus)
					{
						warn("Taille de %s invalide ", filename);
						lseek(archive, debut, SEEK_SET);
						break;
					}

				}

				close(fdInput);
			}
		}
	}

}


void archiver_rep(int archive, char *rep)				// archiver un répertoire
{
	struct dirent *sd = NULL;
	DIR *dir = NULL;

  	char nomFichier[MAX_FILE];

	dir = opendir(rep);

	if(dir == NULL)
	{
		warn("Erreur à l'ouverture du repertoire %s", rep);
	}
	else
	{
		while((sd = readdir(dir)) != NULL )
		{

			if(strcmp(sd->d_name, ".") == 0 || strcmp(sd->d_name, "..") == 0)
			{
				continue;
			}

			strcpy(nomFichier, rep);

			if(nomFichier[strlen(nomFichier)-1] != '/')
				strcat(nomFichier, "/");

			strcat(nomFichier, sd->d_name);

			archiver(archive, nomFichier);
		}

		closedir(dir);
	}
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














