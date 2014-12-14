

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


	if(firstPath == -1)
	{
	    /* L'utilisateur n'a pas fourni de fichier, Il n'y a rien à faire */
        fprintf(stderr," %s:ERREUR: Aucun ficheir n'a été renseigné \n",argv[0]);
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
	char newF[MAX_PATH];
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
        if(enleverSlashEtPoints(filename,newF) == NULL)
        {
            fprintf(stderr,"ERREUR : problème interne lié au fichier suivant : %s \n", filename);
            return;
        }

#ifdef DEBUG
	printf("DEBUG : %s -> %s \n",filename,newF);
#endif

		/* On récupère les informations du fichier puis on les mets dans l'entete */
		info.path_length = strlen(newF);
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
					ecrireEntete(archive,&info,newF);
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
			if((info.mode & USR_RX) != USR_RX || (info.mode & GRP_RX) != GRP_RX || (info.mode & OTH_RX) != OTH_RX)
			{
				fprintf(stderr,"ATTENTION : %s - Droit non valides sur le fichier \n",filename);
				lseek(archive, debut, SEEK_SET); /* TODO Le lseek me parait optionnel */
			}
			else
			{	/* On archive tous les fichiers qui sont dans ce repertoire */
			    info.file_length = 0;

                /*  Dans le cas où on a un dossier, on s'assure d'avoir le '/'
                    quoiqu'il arrive. C'est une politique qu'on a choisit pour
                    notre programme */
			    if(newF[info.path_length -1] != '/' && info.path_length < MAX_PATH)
			    {
                    strcat(newF, "/");
                    info.path_length++;
			    }

				ecrireEntete(archive,&info,newF);
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

				ecrireEntete(archive,&info,newF);

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
int extraire_archive(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp)
{
	/* TODO mettre verrou sur l'archive + création arborescence relative à un fichier dans un dossier : int extraire_archive(char *archive_file) */
	/* TODO Faire le '-v' en extraction -> Verifier l'integrité du fichier décompressé vis-à-vis de ce qui a été indiqué dans le md5 dans l'archive voir l-421 */
	struct stat s,tmp;
	Entete info;

	int fdOutput; 	/* fd en ecriture sur le fichier désarchivé */
	int archive;	/* fd en lecture sur l'archive */
	int i,j, taille_archive;
	int extraire = 0;

	char buf[BUFSIZE];
	char filename[BUFSIZE];
	char arbo[MAX_PATH];    /*Stocke l'arborescence dans lequel appartient le fichier cible*/
	char checksum[CHECKSUM_SIZE];
	int lus;

    /*  Stocke les arborescence des fichiers à extraire
        utilisé notamment lorsqu'on veut extraire un repertoir et tous les fichier*/
	char *arborescences[MAX_PATH];
	int a = 0;
	int err = 0;


#ifdef DEBUG
	printf("DEBUG : Extraction de %s \n", archive_file);
#endif

	if(argv == NULL || sp == NULL) return -1;

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

        extraire = 0;

		read(archive,&info.path_length,sizeof(info.path_length));
		read(archive,&info.file_length,sizeof(off_t));
		read(archive,&info.mode,sizeof(mode_t));
		read(archive,&info.m_time,sizeof(time_t));
		read(archive,&info.a_time,sizeof(time_t));
		read(archive,&info.checksum,CHECKSUM_SIZE);
		read(archive,filename, info.path_length);	/* lire le nom du fichier */

		info.checksum[CHECKSUM_SIZE] = '\0';
		filename[info.path_length] = '\0';

        /* Y a-t-il des fichiers spécifiques à extraire ? */
        if(firstPath != -1)
        {
            // On ne veux extraire que les fichier en paramètre
            for(i = firstPath; i < argc; i++)
            {
                if(!strcmp(getArborescence(filename,arbo),argv[i]) || !strcmp(filename,argv[i]))
                {
                    extraire = 1;

                    /*  Coment savoir si ce qu'a mis l'utilisateur final
                        est un répertoire ou un fichier ?
                        Pour cela, on va encore définir une politique pars defaut
                        sur le type de fichier q'uon a en paramètre positionnel.
                        On part du principe que si l'utilisateur met un '/' final
                        on a un répertoire.
                        Dans le cas contraire, on a un fichier normal */
                    if(argv[i][strlen(argv[i])-1] == '/')
                    {
                        /*On considère le fichier comme un répertoire*/
                        if(S_ISDIR(info.mode) && !dansArborescence(argv[i], arborescences,a))
                        {
                            arborescences[a++] = argv[i];
                        }
                    }

                    break;
                }
                else
                {
                    /* Si le fichier appartient à une arborescence à extraire */
                    if(dansArborescence(getArborescence(argv[i], arbo), arborescences,a))
                    {
                        /* Il appartient à l'arborescence à extraire, on l'extrait donc */
                        extraire = 1;
                        break;
                    }
                }
            }

            /* Le fichier que je m'apprête à extraire est-il dans les fichiers désisé*/
            if(extraire == 0)
            {   /* Ce n'est pas un fichier que j'ai demandé, on passe à la suite */

                if(!S_ISDIR(info.mode))
                    lseek(archive,info.file_length,SEEK_CUR);

                continue;
            }
        }


#ifdef DEBUG
	printf("DEBUG : Extraction de %s hors de %s \n",filename, archive_file);
#endif

		/* Si c'est un lien, on met juste le nom du fichier pointé par le lien */
		if(S_ISLNK(info.mode) > 0)
		{
			lus = read(archive,buf,info.file_length);	/* on recupére le nom du fichier pointé par le lien */

			/* Cela a-t-il marché*/
			if(lus > 0)
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

			if(sp->flag_s)
			{
			    /* Ah, donc on le prend bien en compte */

                /* On test si le fichier est dans une arborescence */
                if(getArborescence(filename,arbo) != NULL)
                {
                    if(mkdirP(arbo) == -1)
                    {
                        fprintf(stderr,"Erreur lors de la création de l'arborescence %s\n",arbo);
                        lseek(archive,info.file_length,SEEK_CUR);
                        continue;
                    }
                }

				symlink(buf,filename);	/* On crée le lien filename sur buf */
			}
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

            // On crée l'arborescence
			if(mkdirP(filename) == -1)
			{
				warn("Erreur à la création de l'arborescence %s ", filename);
			}

            chmod(filename, info.mode);

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

        /* On test si le fichier est dans une arborescence */
        if(getArborescence(filename,arbo) != NULL)
        {
            if(mkdirP(arbo) == -1)
            {
                fprintf(stderr,"Erreur lors de la création de l'arborescence %s\n",arbo);
                lseek(archive,info.file_length,SEEK_CUR);
                continue;
            }
        }

		/* On ouvre le fichier à desarchiver en écriture */
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

        if(sp->flag_v)
        {
            err = checksumRenseigne(info.checksum);

            if(err == 1)
            {
                if(md5sum(filename,checksum) == NULL)
                {
                    fprintf(stderr,"%s : Probleme lors de l'obtention du md5 de %s",argv[0],filename);
                    continue;
                }

                if(!strncmp(checksum,info.checksum,CHECKSUM_SIZE))
                {
                    printf("%s: INFO : Le fichier %s est intègre \n",argv[0],filename);
                }
                else
                    printf("%s: ATTENTION : Le fichier %s n'est pas intègre \n",argv[0],filename);
            }
            else if(err == 0)
               printf("%s: INFO : Le fchecksum de %s n'est pas renseigné \n",argv[0],filename);
            else
                fprintf(stderr,"%s: %s : Probleme interne à checksumRenseigne() ou paramètres invalides\n",argv[0],filename);
        }

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


	if(firstPath == -1)
	{
	    /* L'utilisateur n'a pas fourni de fichier, Il n'y a rien à faire */
        fprintf(stderr," %s:ERREUR: Aucun ficheir n'a été renseigné \n",argv[0]);
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

    char *arborescences[MAX_PATH];
    char arbo[MAX_PATH];
	int a = 0;

	struct stat s;
	Entete info;

	int supprimer = 0;


	if(sp == NULL) return -1;

	if(!sp->flag_d)
	{
		fprintf(stderr," %s:ERREUR: supression dans l'archive non permise, option '-d' non detectée \n",argv[0]);
		return -1;
	}

	if(firstPath == -1)
	{
	    /* L'utilisateur n'a pas fourni de fichier, Il n'y a rien à faire */
        fprintf(stderr," %s:ERREUR: Aucun ficheir n'a été renseigné \n",argv[0]);
        return -1;
	}

	if(stat(archive_file, &s) == -1)
	{
		warn("%s - impossible d'obtenir les informations sur le fichier %s \n",argv[0],archive_file);
		return -1;
	}

	taille_archive = s.st_size;

	fdArchive = open(archive_file, O_RDONLY);

	if(fdArchive == -1)
	{

		warn("%s : erreur à l'ouverture du fichier archivé %s \n",argv[0],archive_file);
		return -1;
	}

	fdFichier = open(tmpFichier,O_WRONLY|O_TRUNC|O_CREAT,USR_W);

	if(fdFichier == -1)
	{
		close(fdArchive);
		return -1;
	}


	/* Tant qu'on n'est pas à la fin de l'archive */
	while(lseek(fdArchive,0,SEEK_CUR) != taille_archive)
	{
	    supprimer = 0;

		/* On lit l'entete */

		read(fdArchive,&info.path_length,sizeof(info.path_length));
		read(fdArchive,&info.file_length,sizeof(off_t));
		read(fdArchive,&info.mode,sizeof(mode_t));
		read(fdArchive,&info.m_time,sizeof(time_t));
		read(fdArchive,&info.a_time,sizeof(time_t));
		read(fdArchive,&info.checksum,CHECKSUM_SIZE);
		read(fdArchive, filename, info.path_length);	/* lire le nom du fichier */

		filename[info.path_length] = '\0';

        // On ne veux supprimer que les fichiers en paramètre
        for(i = firstPath; (i< argc && strcmp(argv[i],"-f")); i++)
        {
            if(!strcmp(getArborescence(filename,arbo),argv[i]) || !strcmp(filename,argv[i]))
            {
                supprimer = 1;

                /*  Coment savoir si ce qu'a mis l'utilisateur final
                    est un répertoire ou un fichier ?
                    Pour cela, on va encore définir une politique pars defaut
                    sur le type de fichier q'uon a en paramètre positionnel.
                    On part du principe que si l'utilisateur met un '/' final
                    on a un répertoire.
                    Dans le cas contraire, on a un fichier normal */
                if(argv[i][strlen(argv[i])-1] == '/')
                {
                    /*On considère le fichier comme un répertoire*/
                    if(S_ISDIR(info.mode) && !dansArborescence(argv[i], arborescences,a))
                    {
                        arborescences[a++] = argv[i];
                    }
                }

                break;
            }
            else
            {
                /* Si le fichier appartient à une arborescence à supprimer */
                if(dansArborescence(getArborescence(filename, arbo), arborescences,a))
                {
                    /* Il appartient à l'arborescence à supprimer, on l'extrait donc */
                    supprimer = 1;
                    break;
                }
            }
        }

        if(supprimer)
        {
            if(S_ISLNK(info.mode))
            {
                /* C'est un lien */
                if(!sp->flag_s)
                {
                    /* L'option '-s' n'est pas actif -> le lien doit être ignoré
                        dans la suppression (pas de suppression) */
                    ecrire_fichier_sauvegarde(fdArchive,fdFichier, &info,filename, buf, BUFSIZE);
                    continue;
                }
                else
                {
                    /* On doit supprimer le lien */
                    lseek(fdArchive,info.file_length,SEEK_CUR);
                }

            }

            lseek(fdArchive,info.file_length,SEEK_CUR);

            continue;
        }








		for(i = firstPath; (i< argc && strcmp(argv[i],"-f")); i++)
		{

			if(strcmp(argv[i],filename)==0)
			{

				/* Si le fichier est un lien symbolique */


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


/* option "-l" */
int liste_fichiers(char *archive_file, Parametres *sp){

    /* TODO liste_fichiers(char *archive_file, Parametres *sp) */

    char filename[BUFSIZE];
    char infoFichier[BUFSIZE*2];
    char tmp_char[10];
    /*char tmp_arch[] = "/tmp/.arch_kl";*/ /* Créer ce repertoire et y mettre une copie de l'archive*/
    /* @note : Il faudra penser à le supprimer quand on a tout fait*/

    Entete info;
    struct stat s;

	int fdArchive;
    int exit_val = 0;
    int taille_archive;

    if(stat(archive_file,&s) == -1)
    {
        perror("ERREUR ");
        return -1;
    }

    taille_archive = s.st_size;

	if(sp == NULL)	return -1;

	if(!sp->flag_l)
	{
		fprintf(stderr,"ERREUR : ajout dans l'archive non permise, option '-l' non detectée ");
		return -1;
	}

	fdArchive = open(archive_file,O_RDONLY);

	if(fdArchive == -1)
	{

		warn("erreur à l'ouverture du fichier archivé %s ",archive_file);
		return -1;
	}

    /* TODO mettre une copie de l'archive dans un repertoire tampon puis tout extraire dans ce même repertoire
        et faire un ls */

	close(fdArchive);

	return exit_val;
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

/* Calcule le md5 du fichier */
char * md5sum(const char *filename, char *checksum)
{
	pid_t p;
	int status;
	int lus = -1;

	int tube[2];
	char cmd[] = "md5sum";


	if(filename == NULL || checksum == NULL)
		return NULL;	/* Les paramètres ne sont pas valides, on ne va pas plus loin, on renvoie NULL */

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
			memset(checksum,0,CHECKSUM_SIZE);
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

/*
    Vérifies si le md5 d'un fichier est renseigné
    Si oui , la fonction retourne 1, 0 sinon.
    Si le checksum est NULL ou a une longueur
    strictement inférieur à la gueur d'un checksum
    renvoie -1.
*/
int checksumRenseigne(char * checksum)
{
    int i = 0;

    if(checksum == NULL || strlen(checksum) != CHECKSUM_SIZE)
        return -1; /* Le checksum n'est pas conforme, problème ! */

    /* Tant qu'on est pas à la fin et qu'on a rien de défini */
    while(i < CHECKSUM_SIZE && checksum[i] == 0)
    {
        i++;
    }

    /*  Si on est pas arrivé au bout, on considère que le checksum est définit*/
    if(i != CHECKSUM_SIZE)
        return 1;

    return 0;   /* Si on arrive là, alors cela signifie que le checksum n'est pas défini*/

}

/*  Enlève le '/' de début de chaine si le chemin est absolu
    ainsi que les "./" et "../" */
char *enleverSlashEtPoints(char *oldchaine, char *newchaine){

    int i = 0,j = 0;
    int cheminAbs = 1;      /* Variable indiquant qu'on a un chemin normal*/
    int dejaCopie = 0;      /* Variable indiquant qu'aucune copie n'a été faite */
    int debut = 0;              /* Debut d'une sous-chaine*/
    int apresDernierPoints = 0; /*La position après les "../" */


    if(oldchaine == NULL || newchaine == NULL)
        return NULL;


    memset(newchaine,0,MAX_PATH); /* On met tous les éléments à 0*/

    /* Le '/' est-il présent au début de la chaine ? */
    if(oldchaine[0] == '/')
        j += 1;


    for(i = j; i < strlen(oldchaine); i++)
    {
        if(oldchaine[i] != '.')
            continue;
        else
        {

            /* A-t-on "./" ? */
            if(oldchaine[i+1] == '/')
            {
                cheminAbs = 0;

                if(!dejaCopie)
                {   /* On n'a pas encore copié, on la fait donc */
                    strncpy(newchaine, oldchaine + j, i-j);
                    dejaCopie = 1;  /* On a copié */

                }
                else
                {   /* On a déjà copié, on concatène la suite avec ce qu'on avait déjà*/
                    strncat(newchaine, oldchaine + debut, i - debut);
                }

                debut = i + 2;  /* on mémorise le début de la suite de la chaine */
                i += 1;         /* On saute les caracères déjà testés*/

            }
            else if(oldchaine[i+1] == '.' && oldchaine[i+2] == '/')
            {
                cheminAbs = 0;

                /* On a "../" */
                debut = i + 3;  /* on mémorise le début de la suite de la chaine */
                apresDernierPoints = debut;
                i += 2;         /* On saute les caracères déjà testés*/

            }
            else
                continue;

        }
    }

        if(cheminAbs)
        {   /*  On a un chemin absolu, on copie toute la chaine tel quelle
                en prenant soin de ne pas avoir le '/' */
            strncpy(newchaine,(oldchaine + j),strlen(oldchaine) -j);
            newchaine[strlen(oldchaine) -j] = '\0';    /* pour être sûr d'avoir le '\0' */
        }
        else
        {
            /* On a un chemin relatif*/
            if(apresDernierPoints > 0)
            {   /* Il y avait la chaine "../" */
                strncpy(newchaine,oldchaine + apresDernierPoints,strlen(oldchaine) - apresDernierPoints);
                newchaine[strlen(oldchaine) - apresDernierPoints] = '\0';
            }
            else
            {   /* Il n'y avait pas la chaine "./" */
                strncat(newchaine,oldchaine + debut,strlen(oldchaine) - debut);
            }

        }


	return newchaine;
}


/* Créer une arborescence en utilisant la commane système "mkdir -p" */
int mkdirP(char *arborescence)
{
	pid_t p;
	int status;

	char cmd[] = "mkdir";


	if(arborescence == NULL)
		return -1;	/* Les paramètres ne sont pas valides, on ne va pas plus loin, on renvoie NULL */

	p = fork();

	if(p == 0)
	{

		execlp(cmd,cmd,"-p",arborescence, NULL);

		perror("Erreur lors de l'execution de la commande d'obtention du md5 ");

		exit(-1);

	}
	else if(p > 0)
	{

		wait(&status);

		if(WIFEXITED(status) && (WEXITSTATUS(status) == 0))
		{
			/* Tout s'est bien passé */
            return 0;
		}
		else
		{
			/* Il y a eu un problème */
			fprintf(stderr, "mkdir -p - Echec lors de la création de l'arborescence \n");
			return -1;
		}

	}
	else
	{
		fprintf(stderr,"ERREUR: cmkdir, execution de la commande impossible\n");
		return -1;	/* La création du fork a echoué, on ne peut rien faire, retourne NULL*/
	}

}


/*  Renvoie le 'pwd' du fichier mis en paramètre et
    stocke le resultat dans la 2ème chaine
    Si au moins une des deux chaines est NULL,
    alors le comportement est indéfini.
    En effet, la valeur NULL n'est retourné que si
    on a un fichier qui n'est pas dans une arborescence*/
char *getArborescence(char *filename, char *newA)
{
    int i;

    /*if(filename, newA)
        return NULL;*/

    memset(newA,0,MAX_PATH);

    i = strlen(filename) - 1;

    while(i >= 0 && filename[i] != '/')
    {
        i--;
    }


    if(i == -1)
        return NULL;
    else
    {
        strncpy(newA,filename,i+1);
    }

    return newA;
}

/*  Test si une arborescence est déjà dans un tableau
    d'arborescence. retourne 1 si oui , 0 sinon */
int dansArborescence(char *arbo, char *arborescences[], int lgr)
{
    int k = 0;

    if(arbo == NULL || arborescences == NULL )
    {
        return 0;
    }

    while(k < lgr)
    {
        if(!strcmp(arborescences[k], arbo))
            return 1;   /* On l'a trouvé*/

        k++;
    }

    return 0;
}












