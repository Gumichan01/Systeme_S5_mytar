

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

/* @note 1 : en dehors de la vérification, de l'ajout et de la suppression de l'archive, toutes les fonctions seront 
   des reprises des fonctions du TP 6 de système */

/* @note 2 : Toutes les fonctions doivent utiliser les verrous sur le fichier archive*/

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


/* Ecrit l'entete du fichier */
void ecrireEntete(int archive, Entete *info, char *filename)
{
	if(info != NULL)
	{ 
		write(archive,&info->path_length,sizeof(size_t));	/* ecrire la taille */
		write(archive,&info->file_length,sizeof(off_t));		/* longueur du contenu */
		write(archive,&info->mode,sizeof(mode_t));		/* ecrire le mode */
		write(archive,&info->m_time,sizeof(time_t)); 
		write(archive,&info->a_time,sizeof(time_t));
		write(archive,&info->checksum,CHECKSUM_SIZE);

		write(archive, filename, info->path_length);	/* ecrire le nom du fichier */
	}
}



/* creation de l'archive */
/* archive_file : fichier archive; firstPath : position du premier fichier path */
int creer_archive(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp)
{
	/* @note tester sp->flag_s */
	int i;
	int archive;

#ifdef DEBUG
	printf("DEBUG : Ouverture du fichier %s\n", archive_file);
#endif

	if(sp == NULL) return -1;


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
	for(i = firstPath; i < argc ; i++)
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
	off_t debut;

	struct stat s;
	int fdInput; 	/* fd en lecture */

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
		info.path_length = strlen(filename);
		info.file_length = s.st_size;
		info.mode = s.st_mode;
		info.m_time = s.st_mtime;
		info.a_time = s.st_atime;
		memset(info.checksum,0,CHECKSUM_SIZE);

		/* On ecrit dans l'archive */
		debut = lseek(archive,0, SEEK_CUR);	/* On garde la position du début */


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
					/*buf[lus] = '\0';*/
					ecrireEntete(archive,&info,filename);
					write(archive,buf,lus);
				}
			}
			else
			{	/* L'option -s n'est pas active, mince, on revient */
				printf("INFO : %s est un lien symbolique, il sera donc ignoré \n", filename);
				lseek(archive, debut, SEEK_SET);
			}

		}
		else if(S_ISDIR(info.mode) > 0) /* Le cas si c'est un répertoire */
		{
			/* A-t-on les droits de parcours (pour l'accès aux fichiers) et de modification (pour l'extraction) */
			if((info.mode & 0500) != 0500 || (info.mode & 0050) != 0050 || (info.mode & 0005) != 0005)
			{
				fprintf(stderr,"ATTENTION : %s - Droit non valides sur le fichier \n",filename);
				lseek(archive, debut, SEEK_SET);
			}
			else
			{	/* On archive tous les fichiers qui sont dans ce repertoire */
				ecrireEntete(archive,&info,filename);
				archiver_rep(archive,filename,sp);
			}
		}
		else if(S_ISREG(info.mode) > 0)	/* Le cas d'un fichier régulier , on lit son contenu */
		{
			fdInput = open(filename,O_RDONLY);

			if(fdInput == -1)
			{
				warn("Erreur à l'ouverture de %s ",filename);
				lseek(archive, debut, SEEK_SET);
			}
			else
			{
				if(sp->flag_v)
				{
					/* On récupère le md5 du fichier à archiver */
					if( md5sum(filename, info.checksum) == NULL)
					{
						fprintf(stderr,"Echec lors de l'obtention du md5 du fichier %s \n", filename);
						memset(info.checksum,0,CHECKSUM_SIZE);
					}
				}

				ecrireEntete(archive,&info,filename);

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
		else /* le cas si le fichier est un tube ou socket ...etc... */
		{
			fprintf(stderr,"Format fichier non valide! \n");
			lseek(archive, debut, SEEK_SET);
			
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
int extraire_archive(char *archive_file, Parametres *sp)
{
	/* TODO mettre verrou sur l'archive : int extraire_archive(char *archive_file) */
	/* TODO Faire le '-v' en extraction -> Verifier l'integrité du fichier décompressé vis-à-vis de ce qui a été indiqué dans le md5 dans l'archive voir l-421 */
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

	if(!sp->flag_x)
	{
		fprintf(stderr,"ERREUR : extraction de l'archive non permise, option '-x' non detectée ");
		return -1;
	}

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
		read(archive,&info.checksum,CHECKSUM_SIZE);
		read(archive, filename, info.path_length);	/* lire le nom du fichier */

		/*info.checksum[sizeof(&info.checksum)] = '\0';*/
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


			if(sp->flag_k)	/* Si l'option -k est active */
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
				printf("ATTENTION : %s déjà existant\n",filename);
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

		/* @note .1 : on récupère le checksum quand '-v' est renseigné && si le checksum est rempli de 0, ne pas faire la comparaison || comparer (utiliser strcmp())*/
		/* @note .2 : Doit-on afficher quelque chose si la comparaison n'est pas bonne ?  A voir*/

	}

	close(archive);

#ifdef DEBUG
	printf("DEBUG : Fin de l'extraction \n");
#endif

	return 0;
}


/* option "-a"*/
int ajouter_fichier(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp)
{
	/*ajout d'un fichier dans l'archive : int ajouter_fichier(char *archive_file, char *filename) */
	/* @note tester sp->flag_a */
	int fdArchive;
	int i;


	if(sp == NULL)	return -1;

	if(!sp->flag_a)
	{
		fprintf(stderr,"ERREUR : ajout dans l'archive non permise, option '-a' non detectée ");
		return -1;
	}

	fdArchive = open(archive_file,O_WRONLY| O_APPEND);

	if(fdArchive == -1)
	{

		warn("erreur à l'ouverture du fichier archivé %s ",archive_file);
		return -1;
	}

	lseek(fdArchive,0,SEEK_CUR);	/* On se met à -1 car le dernier octet du fichier à pour pour hexadecimal  0x0A*/
	
	for(i = firstPath; (i< argc && strcmp(argv[i],"-f")); i++)
	{
		archiver(fdArchive, argv[i], sp);
	}

	close(fdArchive);

	return 0;

}


/* option "-d"*/
int supprimer_fichiers(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp)
{

	/* TODO La suppression d'un répertoire + test si un fichier archivé n'est pas valide */
	/* suppression d'un fichier dans l'archive : int supprimer_fichier(char *archive_file, char *filename) */
	

	int fdArchive, fdFichier;
	int taille_archive;
	int i,j, lus;

	char tmpFichier[] = "/tmp/tampon_F";

	char buf[BUFSIZE];
	char filename[BUFSIZE];

	struct stat s;
	Entete info;


	if(sp == NULL) return -1;

	if(!sp->flag_d)
	{
		fprintf(stderr,"ERREUR: supression dans l'archive non permise, option '-d' non detectée ");
		return -1;
	}

	if(stat(archive_file, &s) == -1)
	{
		warn("%s - impossible d'obtenir les informations sur le fichier archive ",archive_file);
		return -1;
	}

	taille_archive = s.st_size;

	fdArchive = open(archive_file, O_RDONLY);

	if(fdArchive == -1)
	{

		warn("erreur à l'ouverture du fichier archivé %s ",archive_file);
		return -1;
	}

	fdFichier = open(tmpFichier,O_WRONLY|O_TRUNC|O_CREAT,0660);

	if(fdFichier == -1)
	{
		close(fdArchive);
		return -1;
	}


	/* Tant qu'on n'est pas à la fin de l'archive */
	while(lseek(fdArchive,0,SEEK_CUR) != taille_archive)
	{
		/* On lit l'entete */

		read(fdArchive,&info.path_length,sizeof(info.path_length));
		read(fdArchive,&info.file_length,sizeof(off_t));
		read(fdArchive,&info.mode,sizeof(mode_t));
		read(fdArchive,&info.m_time,sizeof(time_t));
		read(fdArchive,&info.a_time,sizeof(time_t));
		read(fdArchive,&info.checksum,CHECKSUM_SIZE);
		read(fdArchive, filename, info.path_length);	/* lire le nom du fichier */

		filename[info.path_length] = '\0';


		for(i = firstPath; (i< argc && strcmp(argv[i],"-f")); i++)
		{
			
			if(strcmp(argv[i],filename)==0)
			{
				
				/* Si le fichier est un lien symbolique */
				if(S_ISLNK(info.mode) && !sp->flag_s)
				{
					/* L'option '-s' n'est pas actif -> le lien doit être ignoré dans la suppression (pas de suppression) */
					ecrire_fichier_sauvegarde(fdArchive,fdFichier, &info,filename, buf, BUFSIZE);
					continue;
				}

				lseek(fdArchive,info.file_length,SEEK_CUR);
				
			}
			else
			{
				ecrire_fichier_sauvegarde(fdArchive,fdFichier, &info,filename, buf, BUFSIZE);
			}
		

		}
		

	}

	close(fdFichier);
	close(fdArchive);


	if(stat(tmpFichier, &s) == -1)
	{
		unlink(tmpFichier);	/* On veut être sûr que le fichier sera bien supprimé, même s'il est possible qu'il soit déjà supprimé */
		return -1;
	}

	taille_archive = s.st_size;

	fdArchive = open(archive_file, O_WRONLY|O_TRUNC);
	
	if(fdArchive == -1)
	{
		return -1;
	}

	fdFichier = open(tmpFichier,O_RDONLY);

	if(fdFichier == -1)
	{
		close(fdArchive);
		return -1;
	}


	for(j = 0 ; j < taille_archive/BUFSIZE; j++)
	{
		lus = read(fdFichier,buf, BUFSIZE);
		write(fdArchive,buf,lus);
	}

	lus = read(fdFichier,buf, taille_archive % BUFSIZE);
	write(fdArchive,buf,lus);

	close(fdFichier);
	close(fdArchive);

	/*unlink(tmpFichier);*/

	return 0;
}

/* Ecrire  le contenu d'un fichier dans un autre (utilisé dans supprimer_fichiers)*/
void ecrire_fichier_sauvegarde(int fdArchive,int fdFichier, Entete *info,char *filename, char *buf, int bufsize)
{
	int j, lus;

	if(info == NULL || filename == NULL || buf == NULL) return;

	write(fdFichier,&info->path_length,sizeof(info->path_length));
	write(fdFichier,&info->file_length,sizeof(off_t));
	write(fdFichier,&info->mode,sizeof(mode_t));
	write(fdFichier,&info->m_time,sizeof(time_t));
	write(fdFichier,&info->a_time,sizeof(time_t));
	write(fdFichier,info->checksum,CHECKSUM_SIZE);	/* @bug sur cette ecriture qui corrompait le fichier (Reparé)*/
	write(fdFichier,filename, info->path_length);


	for(j = 0 ; j < info->file_length/bufsize; j++)
	{
		lus = read(fdArchive,buf, bufsize);
		write(fdFichier,buf,lus);
	}

	lus = read(fdArchive,buf, info->file_length % bufsize);
	write(fdFichier,buf,lus);

}


char * md5sum(const char *filename, char *checksum)
{
	pid_t p;
	int status;
	int lus = -1;

	int tube[2];
	char cmd[] = "md5sum";


	if(filename == NULL || checksum == NULL)
		return NULL;	/* Les paramètres ne sont pas valides,on ne va pas plus loin, on renvoie NULL */

	if(pipe(tube) == -1)
		return NULL;	/* Echec de la création du tube, on renvoie NULL */

	p = fork();

	if(p == 0)
	{

		close(tube[0]);		/* Fermeture du lecteur */
		dup2(tube[1],STDOUT_FILENO);

		execlp(cmd,cmd,filename, NULL);

		close(tube[1]);
		perror("Erreur lors de l'execution de la commande d'obtention du md5 ");

		exit(-1);

	}
	else if(p > 0)
	{

		close(tube[1]);		/* Fermeture de l'ecrivain */

		wait(&status);

		if(WIFEXITED(status) && (WEXITSTATUS(status) == 0))
		{
			/* Tout s'est bien passé */
			lus = read(tube[0],checksum,CHECKSUM_SIZE);
		}
		else
		{
			/* Il y a eu un problème */
			fprintf(stderr, "md5sum - Echec lors de l'obtention du md5 \n");
			return NULL;
		}


		if(lus < CHECKSUM_SIZE)
		{
			/* On lit moins que ce qui était attendu, ou bien la lecture à echoué, ce n'est pas normal à ce stade*/
			fprintf(stderr,"ERREUR: problème lors de la récupération de la checksum \n");
			return NULL;
		}
		else
			return checksum;	/* Tout va bien, la fonction a fait son travai*/

	}
	else
	{
		fprintf(stderr,"ERREUR: checksum, execution de la commande impossible\n");
		return NULL;	/* La création du fork a echoué, on ne peut rien faire, retourne NULL*/
	}

}

























