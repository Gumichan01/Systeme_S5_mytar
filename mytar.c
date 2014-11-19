

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

/* @note : en dehors de la vérification, de l'ajout et de la suppression de l'archive, toutes les fonctions seront 
   des reprises des fonctions du TP 6 de système */

#include "mytar.h"



/* usage */
void usage(char * prog)
{
	/* TODO améliorer la fonction usage() pour que ce soit plus clair pour l'utilisateur : void usage(char * prog) */
	/* TODO par exemple lister tous les usages possibles du programme : void usage(char * prog) */

	if(prog != NULL)
	{
		fprintf(stderr,"usage : %s [-c|a|d|l|x|k|s|C <rep>|v] -f <archive> <noms_fichiers> \n", prog);
		/*fprintf(stderr,"\t%s [-xf] <archive> \n", prog); */
	}
	exit(EXIT_FAILURE);
}

/* creation de l'archive */
/* archive_file : fichier archive; param : position du premier fichier path */
int creer_archive(const char *archive_file, int param,int argc, char **argv, Parametres *sp)
{
	int i;
	int archive;

#ifdef DEBUG
	printf("DEBUG : Ouverture du fichier %s\n", archive_file);
#endif

	if(sp == NULL) return -1;	/* La structure paramètre est NULL -> On arrete tout*/


	if(!sp->flag_c)
	{
		fprintf(stderr,"%s, création de l'archive non permise, option '-c' non detectée ",argv[0]);
		return -1;
	}


	/* On ouvre l'archive */
	if((archive = open(archive_file, O_WRONLY | O_TRUNC | O_CREAT, 0660)) == -1 )
	{
		warn("Problème à la création de l'archive %s ",archive_file);
		return -1;
	}

#ifdef DEBUG
	printf("DEBUG : Fichier %s ouvert, OK\n", archive_file);
#endif

	/* On met dans l'archive tous les fichiers */
	for(i = param; i < argc ; i++)
	{

#ifdef DEBUG
	printf("DEBUG : Lecture du fichier %s\n", argv[i]);
#endif

		archiver(archive,argv[i],sp);
	}

	close(archive);

	return 0;
}


/* archiver un fichier */
void archiver(int archive, char *filename, Parametres *sp)
{
	/* TODO poser verrou sur l'archive : void archiver(int archive, char *filename) */

	int j;
	int fdInput; 	/* fd en lecture */
	off_t debut;

	struct stat s;
	Entete info;

	char buf[BUFSIZE];
	int lus;

#ifdef DEBUG
	printf("DEBUG : Archivage de %s\n",filename);
#endif

	if(lstat(filename,&s) == -1)
	{
		fprintf(stderr,"%s : fichier inexistant \n", filename);
	}
	else
	{
		/* On récupère les informations du fichier puis on les mets dans l'entete */
		info.path_length = strlen(filename) +1;
		info.file_length = s.st_size;
		info.mode = s.st_mode;
		info.m_time = s.st_mtime;
		info.a_time = s.st_atime;
		memset(info.checksum,0,sizeof(info.checksum));	/* On met le checksum à 0 */


		if(sp->flag_v)
		{		/* Verification -> on doit mettre le md5 du fichier cible */
			if(md5sum(info.checksum, filename) == -1)
			{
				fprintf(stderr,"Erreur lors de l'obtention du md5 de %s ", filename);
				fflush(stderr);
			}

		}

		/* On ecrit dans l'archive */
		debut = lseek(archive,0, SEEK_CUR);	/* On garde la position du début */

		write(archive,&info.path_length,sizeof(size_t));	/* ecrire la taille */
		write(archive,&info.file_length,sizeof(off_t));		/* longueur du contenu */
		write(archive,&info.mode,sizeof(mode_t));	/* ecrire le mode */
		write(archive,&info.m_time,sizeof(time_t)); 
		write(archive,&info.a_time,sizeof(time_t));
		write(archive,&info.checksum,sizeof(&info.checksum));

		write(archive, filename, info.path_length);	/* ecrire le nom du fichier */

		/* Si c'est un lien, on met juste le nom du fichier pointé par le lien */
		if(S_ISLNK(info.mode) > 0)
		{
			if(sp->flag_s)	/* Si le flag est actif on traite les liens */
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
			else
			{	/* L'option -s n'est pas active, mince, on revient */
				lseek(archive, debut, SEEK_SET);
			}

		}
		else if(S_ISDIR(info.mode) > 0) 
		{
			/* A-t-on les droits de parcours (pour l'accès aux fichiers) et de modification (pour l'extraction) */
			if((info.mode & 0500) != 0500 || (info.mode & 0050) != 0050 || (info.mode & 0005) != 0005)
			{
				fprintf(stderr,"ATTENTION : %s - Droit non valides sur le fichier \n",filename);
				lseek(archive, debut, SEEK_SET);
			}
			else
			{	/* On archive tous les fichiers qui sont dans ce repertoire */
				archiver_rep(archive,filename,sp);
			}
		}
		else	/* Sinon on lit son contenu */
		{
			fdInput = open(filename,O_RDONLY);

			if(fdInput == -1)
			{
				warn("Erreur à l'ouverture de %s ",filename);
				lseek(archive, debut, SEEK_SET);
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


/* archiver un répertoire */
/* On suppose que la structure est bien passée en parametre */
void archiver_rep(int archive, char *rep, Parametres *sp)
{
	struct dirent *sd = NULL;
	DIR *dir = NULL;

  	char nomFichier[MAX_FILE];

#ifdef DEBUG
	printf("DEBUG : Archivage de l'arborescence de %s\n", rep);
#endif

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

			archiver(archive, nomFichier,sp);
		}

		closedir(dir);
	}
}


/* extraction de l'archive */
int extraire_archive(const char *archive_file, Parametres *sp)
{
	/* TODO mettre verrou sur l'archive : int extraire_archive(const char *archive_file) */
	struct stat s,tmp;
	Entete info;

	int fdOutput; 	/* fd en ecriture sur le fichier désarchivé */
	int archive;	/* fd en lecture sur l'archive */
	int j, taille_archive;

	char buf[BUFSIZE];
	char filename[BUFSIZE];
	int lus;

#ifdef DEBUG
	printf("DEBUG : Extraction de %s \n", archive_file);
#endif

	if(sp == NULL) return -1;

	if(stat(archive_file, &s) == -1)
	{
		warn("%s - impossible d'obtenir les informations sur le fichier archive ",archive_file);
		return -1;
	}


	taille_archive = s.st_size;

	if((archive = open(archive_file, O_RDONLY)) == -1)
	{
		warn("Impossible d'ouvrir %s ",archive_file);
		return -1;
	}


	/* Tant qu'on n'est pas à la fin du fichier */
	while(lseek(archive,0,SEEK_CUR) != taille_archive)
	{
		/* On lit l'entete */

		read(archive,&info.path_length,sizeof(info.path_length));
		read(archive,&info.file_length,sizeof(off_t));
		read(archive,&info.mode,sizeof(mode_t));
		read(archive,&info.m_time,sizeof(time_t));
		read(archive,&info.a_time,sizeof(time_t));
		read(archive,&info.checksum,sizeof(&info.checksum));
		read(archive, filename, info.path_length);	/* lire le nom du fichier */

		info.checksum[sizeof(&info.checksum)] = '\0';
		filename[info.path_length] = '\0';

#ifdef DEBUG
	printf("DEBUG : Extraction de %s hors de %s \n",filename, archive_file);
#endif

		/* Si c'est un lien, on met juste le nom du fichier pointé par le lien */
		if(S_ISLNK(info.mode) > 0)
		{
			lus = read(archive,buf,info.file_length);	/* on recupére le nom du fichier pointé par le lien */
			buf[info.file_length] = '\0';

			if(lus == -1)
			{
				warn(" Impossible de lire le contenu de %s : ", filename);
				continue;
			}

			if(sp->flag_k)	/* Si l'option -k existe */
			{
				if(lstat(filename,&tmp) != -1)
				{
					printf("ATTENTION : %s déjà existant\n",filename);
					continue;
				}
			}
		
			if(sp->flag_s)	/* Ah, donc on le prend bien en compte */
				symlink(buf,filename);	/* On crée le lien filename sur buf */

			continue;
		}

		if(S_ISDIR(info.mode) > 0)
		{
			if(sp->flag_k)
			{
				if(lstat(filename,&tmp) != -1)
				{
					printf("ATTENTION : %s déjà existant\n",filename);
					continue;
				}
			}

			if(mkdir(filename, info.mode) == -1)
			{
				warn("Erreur à la création du repertoire %s ", filename);
			}

			continue;
		}


		if(sp->flag_k)
		{
			if(lstat(filename,&tmp) != -1)
			{
				lseek(archive, info.file_length,SEEK_CUR);
				continue;	/* On passe au fichier suivant */
			}
		}

		/* On ouvre le fichier à desarchiver en écriture*/
		fdOutput = open(filename, O_WRONLY | O_TRUNC | O_CREAT, info.mode);

		if(fdOutput == -1)
		{
			/*close(archive); */
			warn("erreur à l'ouverture du fichier archivé %s ",filename);
			continue;
		}

		for(j = 0 ; j < info.file_length/BUFSIZE; j++)
		{
			lus = read(archive,buf, BUFSIZE);
			write(fdOutput,buf,lus);
		}

		lus = read(archive,buf, info.file_length % BUFSIZE);
		write(fdOutput,buf,lus);

		close(fdOutput);
	}

	close(archive);

#ifdef DEBUG
	printf("DEBUG : Fin de l'extraction \n");
#endif

	return 0;
}



int ajouter_fichier(const char *archive_file, char *filename, Parametres *sp)
{
	/* TODO ajout d'un fichier dans l'archive : int ajouter_fichier(const char *archive_file, char *filename) */
	/* @note tester sp->flag_a */
	return 0;
}


int supprimer_fichier(const char *archive_file, char *filename)
{
	/* TODO suppression d'un fichier dans l'archive : int supprimer_fichier(const char *archive_file, char *filename) */
	/* @note tester sp->flag_d */
	return 0;
}


int md5sum(char *checksum, char * filename)
{	/* TODO : corriger le bug lié au tableau arg*/
	pid_t p;	/* Le pid récupéré depuis fork() */
	int status;	/*Stocke le status du processus fils*/
	char *arg[] = {MD5SUM,filename, NULL}; /* filename n'es pas évaluable */
	int fd[2];	/* Les descripteurs de fichier en lecture/ecriture, pour le tube anonyme*/
	int lus;	/* Le nombre de caractères lus dans le tube*/

	if(filename == NULL) return -1;		/* fivhier non renseigné ? on retourne -1 */

#ifdef DEBUG
	printf("DEBUG : md5sum - creation du tube anonyme pour avori le md5 de %s \n",filename);
#endif

	if(pipe(fd) == -1)
	{				/* Création du tube anonyme echoue ? */
		warn(" pipe(fd) \n");
		return -1;
	}

	p = fork();			/* Création du nouveau processus */

	if(p > 0)	/* Père */
	{
		char buf[CHECKSUM_SIZE +1];	/* 32 caractères du chekcum +1 pour '\0' */

		/*printf("Je suis le père\n");
		fflush(stdout);*/

		close(fd[1]);	/* Le père ferme le tube en ecriture */
		wait(&status);	/* Il attend que son fils meure */

		if(!WIFEXITED(&status))		/*Le fils a-t-il pu faire la tâche demandée ?*/
		{	/*Non, donc il renvoie -1 */
			/*fprintf(stderr, "md5sum - Echec lors de l'obtention du md5 de %s \n", filename);
			fflush(stderr);	On force l'écriture */
			close(fd[0]);
			return -1;
		}
		
		/* Tout va bien, */
		lus = read(fd[0],buf,sizeof(buf));	/* On lit le checksum*/


		if(lus == sizeof(buf))
		{
			buf[lus] = '\0';
			strcpy(checksum,buf);
			/*printf("md5 : %s\n",buf);*/
	
			close(fd[0]);
			return 0;
		}
		else
		{
			close(fd[0]);
			fprintf(stderr,"erreur read : \n");
			fflush(stderr);
			return -1;
		}

	}
	else if(p == 0)	/* Fils */
	{
		/*printf("Je suis le fils\n");
		fflush(stdout);*/

		close(fd[0]);

		if(dup2(fd[1], STDOUT_FILENO) == -1)	
		{	/* Redirection stdout vers fd[1] impossible*/
			perror(" Problème dup2 \n");
			fflush(stderr);
			exit(EXIT_FAILURE);
		}

		execvp("md5sum", arg);
	
		/* Si on arrive là -> echec*/
		close(fd[1]);
		exit(EXIT_FAILURE);
	}
	else
	{
		warn("ERREUR fork - \n");
		close(fd[0]);
		close(fd[1]);
		return -1;
	}


	return 0;
}











