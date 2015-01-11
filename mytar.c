

/**
*
*	@file mytar.c
*
*	@brief Fichier mytar.c, il contient les définitions de fonctions
*
*	@author Luxon JEAN-PIERRE, Kahina RAHANI
*	Licence 3 Informatique
*
*/


#include "mytar.h"


/* usage */
void usage(char * prog)
{

	if(prog != NULL)
	{
		printf("Utilisation : \n");
		printf("%s -c -f <nom_archive>.mtr <path1>...<pathn> : archiver les fichiers path dans le fichier archive .mtr\n",prog);
		printf("%s -a <path1>...<pathn> -f <nom_archive>.mtr <path1>...<pathn> : ajouter un fichier à l'archive\n",prog);
		printf("%s -d <path1>...<pathn> -f <nom_archive>.mtr <path1>...<pathn> : supprimer un/les fichier(s) path de l'archive\n",prog);
		printf("%s -x <path1>...<pathn> -f <nom_archive>.mtr <path1>...<pathn> : extraire un/les fichier(s) path de l'archive\n",prog);
		printf("%s -l <path1>...<pathn> -f <nom_archive>.mtr <path1>...<pathn> : lister les fichiers archivés \n",prog);

		printf("\nRemarque 1 : pour les options '-x' et '-l', si les path ne sont pas renseignés, tous les fichiers sont traités \n");
		printf("\nOptions auxiliaires : \n");

		printf(" -k : à l'extraction, n'écrase pas les fichiers existants ayant le même nom\n");
		printf(" -s : prendre en compte les liens symboliques\n");
		printf(" -C <rep> : à la création, défitint rep comme racine des fichiers archivés. A l'extraction, tous les fichiers sont extraits dans <rep>\n");
		printf(" -v : à la création et l'ajout, calcule le md5sum du fichier à archiver. A l'extraction, compare md5 du fichier extrait à la valeur renseignée\n");

	}
	exit(EXIT_FAILURE);
}


/* Ecrit l'entete du fichier */
int ecrireEntete(int archive, Entete *info, char *filename)
{
	if(info != NULL)
	{
	    if( write(archive,&info->path_length,sizeof(size_t)) != sizeof(size_t)
            || write(archive,&info->file_length,sizeof(off_t)) != sizeof(size_t)
            || write(archive,&info->mode,sizeof(mode_t)) != sizeof(mode_t)
            || write(archive,&info->m_time,sizeof(time_t)) != sizeof(time_t)
            || write(archive,&info->a_time,sizeof(time_t)) != sizeof(time_t)
            || write(archive,&info->checksum,CHECKSUM_SIZE) != CHECKSUM_SIZE
            || write(archive, filename, info->path_length) != info->path_length )
        {
            return -1;
        }

        return 0;
	}

	return -1;
}



/* creation de l'archive */
/* archive_file : fichier archive; firstPath : position du premier fichier path */
int creer_archive(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp)
{
	int i, err = 0;
	int archive;
	char root[MAX_PATH];

	getRepRoot(root,argc,argv);

#ifdef DEBUG
	printf("DEBUG : Ouverture du fichier %s\n", archive_file);
#endif

	if(sp == NULL) return -1;


	if(!sp->flag_c)
	{
		fprintf(stderr,"%s : création de l'archive non permise, option '-c' non detectée ",argv[0]);
		return -1;
	}


	if(firstPath == -1)
	{
	    /* L'utilisateur n'a pas fourni de fichier, Il n'y a rien à faire */
        fprintf(stderr," %s : ERREUR: Aucun fichier n'a été renseigné \n",argv[0]);
        return -1;
	}

	/* On ouvre l'archive */
	if((archive = open(archive_file, O_WRONLY | O_TRUNC | O_CREAT, USR_RW)) == -1 )
	{
		warn("Problème à la création de l'archive %s ",archive_file);
		return -1;
	}

#ifdef DEBUG
	printf("DEBUG : Fichier %s ouvert, Verrouillage\n", archive_file);
#endif

    if(lockfile(archive) == -1)
    {
        warn("[%d] Attention : Je n'ai pas pu mettre le verrou sur le fichier %s ",getpid(),archive_file);
        close(archive);
        return -1;
    }

	/* On met dans l'archive tous les fichiers */
	for(i = firstPath; i < argc ; i++)
	{

#ifdef DEBUG
	printf("DEBUG : Lecture du fichier %s\n", argv[i]);
#endif

		err += archiver(archive,argv[i],root,sp);
	}

#ifdef DEBUG
	printf("DEBUG : Déverrouillage du fichier %s\n", archive_file);
#endif

    if(unlockfile(archive) == -1)
    {
        warn("[%d] Attention : Je n'ai pas pu deverrouiller le fichier %s ",getpid(),archive_file);
    }

	close(archive);

	if(err)
	{
        unlink(archive_file);
        return -1;
	}

	return 0;
}


/* archiver un fichier */
int archiver(int archive, char *filename, char *root, Parametres *sp)
{

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
            return -1;
        }

        if(sp->flag_C)
        {
            if(strncmp(root,newF,strlen(root)))
            {
                if(catRoot(root,newF) == NULL)
                {
                    fprintf(stderr,"Impossible de mettre le fichier %s dans le repertoire cible dans l'archive",filename);
                }
            }
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
					if(ecrireEntete(archive,&info,newF) == -1)
					{
#ifdef DEBUG
	fprintf(stderr,"DEBUG : ERREUR : ecrireEntete dans archiver lien \n");
#endif
                        return -1;
					}

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
#ifdef DEBUG
	printf("DEBUG : INFO :  mode  %s : %o \n",filename,info.mode);
#endif
			/* A-t-on les droits de parcours (pour l'accès aux fichiers) et de modification (pour l'extraction) */
			if((info.mode & USR_RX) != USR_RX && (info.mode & GRP_RX) != GRP_RX && (info.mode & OTH_RX) != OTH_RX)
			{
				fprintf(stderr,"ATTENTION : %s - Droit non valides sur le fichier \n",filename);
				/*lseek(archive, debut, SEEK_SET);  Le lseek me parait optionnel */
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


                if(ecrireEntete(archive,&info,newF) == -1)
                {
#ifdef DEBUG
	fprintf(stderr,"DEBUG : ERREUR : ecrireEntete dans archiver rep \n");
#endif
                    return -1;
                }

				if(archiver_rep(archive,filename,root,sp) == -1)
				{
#ifdef DEBUG
	fprintf(stderr,"DEBUG : ERREUR : archivage de %s \n",filename);
#endif
                    return -1;
				}
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


                if(ecrireEntete(archive,&info,newF) == -1)
                {
#ifdef DEBUG
	fprintf(stderr,"DEBUG : ERREUR : ecrireEntete dans archiver fichier \n");
#endif
                    close(fdInput);
                    return -1;
                }

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
			fprintf(stderr,"ATTENTION : Format du fichier %s non valide ! \n",filename);
			lseek(archive, debut, SEEK_SET);
		}
	}

    return 0;
}


/* archiver un répertoire */
/* On suppose que la structure est bien passée en parametre */
int archiver_rep(int archive, char *rep, char *root, Parametres *sp)
{
	struct dirent *sd = NULL;
	DIR *dir = NULL;
	int err = 0;

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

			if(archiver(archive, nomFichier, root,sp) == -1)
			{
                err = -1;
			}
		}

		closedir(dir);
	}

	return err;
}


/* extraction de l'archive */
int extraire_archive(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp)
{

	struct stat s,tmp;
	Entete info;

	int fdOutput; 	/* fd en ecriture sur le fichier désarchivé */
	int archive;	/* fd en lecture sur l'archive */
	int i,j, taille_archive;
	int extraire = 0;

	char buf[BUFSIZE];
	char filename[MAX_PATH];
    char root[MAX_PATH];
	char arbo[MAX_PATH];    /*Stocke l'arborescence dans laquelle appartient le fichier cible*/
	char checksum[CHECKSUM_SIZE];
	int lus;

	int err = 0;


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

    getRepRoot(root,argc,argv);

	taille_archive = s.st_size;

	if((archive = open(archive_file, O_RDONLY)) == -1)
	{
		warn("Impossible d'ouvrir %s ",archive_file);
		return -1;
	}

    if(read_lock(archive,0,SEEK_SET,0) == -1)
    {
        warn("[%d] Attention : Je n'ai pas pu mettre le verrou sur le fichier %s à extraire ",getpid(),archive_file);
        close(archive);
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

        if(sp->flag_C)
        {
            if(strncmp(root,filename,strlen(root)))
            {
                if(catRoot(root,filename) == NULL)
                {
                    fprintf(stderr,"Impossible de mettre %s dans l'arborescence %s",filename, root);
                }
            }
        }

        /* Y a-t-il des fichiers spécifiques à extraire ? */
        if(firstPath != -1)
        {
            /* On ne veut extraire que les fichiers en paramètre */
            for(i = firstPath; i < argc; i++)
            {
                if(!strncmp(filename,argv[i],strlen(argv[i])))
                {
                    extraire = 1;
                    break;
                }
            }

            /* Le fichier que je m'apprête à extraire est-il dans les fichiers désirés */
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

            /* On crée l'arborescence */
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
			warn("erreur à l'ouverture du fichier archivé %s ",filename);
			lseek(archive,info.file_length,SEEK_CUR);
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
               printf("%s: INFO : Le checksum de %s n'est pas renseigné \n",argv[0],filename);
            else
                fprintf(stderr,"%s: %s : Probleme interne à checksumRenseigne() ou paramètres invalides\n",argv[0],filename);
        }

	}

    if(unlockfile(archive) == -1)
    {
        warn("[%d] Attention : Je n'ai pas pu deverrouiller le fichier %s ",getpid(),archive_file);
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

	int fdArchive;
	int i, err = 0;


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


    if(lockfile(fdArchive) == -1)
    {
        warn("[%d] Attention : Je n'ai pas pu mettre le verrou sur le fichier %s ",getpid(),archive_file);
        close(fdArchive);
        return -1;
    }

	lseek(fdArchive,0,SEEK_END);

	for(i = firstPath; (i< argc && argv[i][0] != '-' ); i++)
	{
		err += archiver(fdArchive, argv[i],NULL, sp);
	}

    if(unlockfile(fdArchive) == -1)
    {
        warn("[%d] Attention : Je n'ai pas pu deverrouiller le fichier %s ",getpid(),archive_file);
    }


	close(fdArchive);

	if(err)
    {
        return -1;
    }

	return 0;

}


/* option "-d"*/
int supprimer_fichiers(char *archive_file, int firstPath,int argc, char **argv, Parametres *sp)
{

	int fdArchive, fdFichier;
	off_t taille_archive;
	int i,j, lus;
	int err = 0;

	char tmpFichier[32];

	char buf[BUFSIZE];
	char filename[BUFSIZE];

	struct stat s;
	Entete info;

	int supprimer = 0;

    sprintf(tmpFichier,"/tmp/.tampon-%d.kl.mtr",(int)getpid());


	if(sp == NULL) return -1;

	if(!sp->flag_d)
	{
		fprintf(stderr," %s:ERREUR: supression dans l'archive non permise, option '-d' non detectée \n",argv[0]);
		return -1;
	}

	if(firstPath == -1)
	{
	    /* L'utilisateur n'a pas fourni de fichier, Il n'y a rien à faire */
        fprintf(stderr," %s:ERREUR: Aucun fichier n'a été renseigné \n",argv[0]);
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

	fdFichier = open(tmpFichier,O_WRONLY|O_TRUNC|O_CREAT,USR_RW);

	if(fdFichier == -1)
	{
		close(fdArchive);
		return -1;
	}

    if(read_lock(fdArchive,0,SEEK_SET,0) == -1)
    {
        warn("[%d] Attention : Je n'ai pas pu mettre le verrou sur le fichier %s à extraire ",getpid(),archive_file);
        close(fdFichier);
        close(fdArchive);
        return -1;
    }


    if(lockfile(fdFichier) == -1)
    {
        warn("[%d] Attention : Je n'ai pas pu mettre le verrou sur le fichier %s ",getpid(),archive_file);
        close(fdFichier);
        unlockfile(fdArchive);
        close(fdArchive);
        return -1;
    }


	/* Tant qu'on n'est pas à la fin de l'archive */
	while(lseek(fdArchive,0,SEEK_CUR) != taille_archive)
	{
	    supprimer = 0;

		/* On lit l'entete */

        if( read(fdArchive,&info.path_length,sizeof(info.path_length)) != sizeof(info.path_length)
            || read(fdArchive,&info.file_length,sizeof(off_t)) != sizeof(off_t)
            || read(fdArchive,&info.mode,sizeof(mode_t)) != sizeof(mode_t)
            || read(fdArchive,&info.m_time,sizeof(time_t)) != sizeof(time_t)
            || read(fdArchive,&info.a_time,sizeof(time_t)) != sizeof(time_t)
            || read(fdArchive,&info.checksum,CHECKSUM_SIZE) != CHECKSUM_SIZE
            || read(fdArchive, filename, info.path_length) != info.path_length )
        {
            err = -1;
            break;
        }

		filename[info.path_length] = '\0';

        /* On ne veut supprimer que les fichiers en paramètre */
        for(i = firstPath; (i< argc && strcmp(argv[i],"-f")); i++)
        {
            if(!strncmp(filename,argv[i],strlen(argv[i])))
            {   /* On a deux fichiers egaux ou un fichier qui est dans l'arborescence argv[i] */
                supprimer = 1;

                break;
            }

        }

        if(supprimer)
        {
#ifdef DEBUG
	printf("DEBUG : Suppression de %s \n",filename);
#endif

            if(S_ISLNK(info.mode))
            {
#ifdef DEBUG
	printf("DEBUG : %s est un lien symbolique \n",filename);
#endif

                /* C'est un lien */
                if(!sp->flag_s)
                {
#ifdef DEBUG
	printf("DEBUG : '-s' absent, pas de suppression \n");
#endif
                    /* L'option '-s' n'est pas actif -> le lien doit être ignoré
                        dans la suppression (pas de suppression) */
                    if(ecrire_fichier_sauvegarde(fdArchive,fdFichier, &info,filename, buf, BUFSIZE) == -1)
                    {
                        err =-1;
                        break;
                    }

                    continue;
                }
                else
                {
#ifdef DEBUG
	printf("DEBUG : %s peut effectivement être supprimé \n",filename);
#endif
                    /* On doit supprimer le lien */
                    lseek(fdArchive,info.file_length,SEEK_CUR);
                }

            }
            else
            {

#ifdef DEBUG
	printf("DEBUG : %s a été supprimé \n",filename);
#endif
                lseek(fdArchive,info.file_length,SEEK_CUR);
            }
        }
        else
        {
            if(ecrire_fichier_sauvegarde(fdArchive,fdFichier, &info,filename, buf, BUFSIZE))
            {
                err = -1;
                break;
            }
        }
	}

    unlockfile(fdFichier);
    unlockfile(fdArchive);

	close(fdFichier);
	close(fdArchive);

    if(err)
    {
#ifdef DEBUG
	printf("DEBUG : ERREUR :  problème lors de la sauvegarde des fichiers \n");
#endif
        unlink(tmpFichier);
        return -1;
    }

	if(stat(tmpFichier, &s) == -1)
	{
        /* On a eu un problème quelque part, cela ne doit pas arriver */
		unlink(tmpFichier);	/* On veut être sûr que le fichier sera bien supprimé, même s'il est possible qu'il soit déjà supprimé */
		return -1;
	}

	taille_archive = s.st_size;

	fdArchive = open(archive_file, O_WRONLY|O_TRUNC);

	if(fdArchive == -1)
	{
		return -1;
	}

    if(lockfile(fdArchive) == -1)
    {
        warn("[%d] Attention : Je n'ai pas pu mettre le verrou sur le fichier %s ",getpid(),archive_file);
        unlockfile(fdArchive);
        close(fdArchive);
        return -1;
    }


	fdFichier = open(tmpFichier,O_RDONLY);

	if(fdFichier == -1)
	{
		close(fdArchive);
		return -1;
	}

    if(read_lock(fdFichier,0,SEEK_SET,0) == -1)
    {
        warn("[%d] Attention : Je n'ai pas pu mettre le verrou sur le fichier %s ",getpid(),archive_file);
        close(fdFichier);
        unlockfile(fdArchive);
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

	unlockfile(fdFichier);
	unlockfile(fdArchive);

	close(fdFichier);
	close(fdArchive);

	unlink(tmpFichier); /* On supprime le fichier tampon */

	return 0;
}


/*  option "-l" */
int liste_fichiers(char *archive_file, Parametres *sp, int argc, char **argv)
{

    Entete info;
    struct stat s;

	int archive, err = 0;
    off_t taille_archive;

    char filename[MAX_PATH];
    char buf[BUFSIZE];
    char champs[BUFSIZE];

    int lister, i;
    int firstPath = getFirstPath(argc,argv);

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

	archive = open(archive_file,O_RDONLY);

	if(archive == -1)
	{
		warn("erreur à l'ouverture du fichier archivé %s ",archive_file);
		return -1;
	}

    if(read_lock(archive,0,SEEK_SET,0) == -1)
    {
        warn("[%d] Attention : Je n'ai pas pu mettre le verrou sur le fichier %s à lire",getpid(),archive_file);
        close(archive);
        return -1;
    }

    while(lseek(archive,0,SEEK_CUR) != taille_archive)
	{
		/* On lit l'entete */

		lister = 0;

		if( read(archive,&info.path_length,sizeof(info.path_length)) != sizeof(info.path_length)
            || read(archive,&info.file_length,sizeof(off_t)) != sizeof(off_t)
            || read(archive,&info.mode,sizeof(mode_t)) != sizeof(mode_t)
            || read(archive,&info.m_time,sizeof(time_t)) != sizeof(time_t)
            || read(archive,&info.a_time,sizeof(time_t)) != sizeof(time_t)
            || read(archive,&info.checksum,CHECKSUM_SIZE) != CHECKSUM_SIZE
            || read(archive,filename, info.path_length) != info.path_length )
        {
            err = -1;
            break;
        }

		info.checksum[CHECKSUM_SIZE] = '\0';
		filename[info.path_length] = '\0';

        if(firstPath != -1)
        {
            /* On ne veut extraire que les fichiers en paramètre */
            for(i = firstPath; i < argc; i++)
            {
                if(!strncmp(filename,argv[i],strlen(argv[i])))
                {
                    lister = 1;
                    break;
                }
            }

            /* Le fichier que je m'apprête à extraire est-il dans les fichiers désirés */
            if(lister == 0)
            {   /* Ce n'est pas un fichier que j'ai demandé, on passe à la suite */

                if(!S_ISDIR(info.mode))
                    lseek(archive,info.file_length,SEEK_CUR);

                continue;
            }
        }

        if(remplirChamps(&info,champs) == NULL)
        {
            fprintf(stderr,
                    "ERREUR : problème lors de la mise en page de l'affichages des informations sur %s \n",
                    filename);
        }
        else
        {
            if(S_ISLNK(info.mode) && sp->flag_s)
            {
                if(read(archive,buf,info.file_length) == info.file_length )
                {
                    buf[info.file_length] = '\0';
                    strcat(champs,filename);
                    strcat(champs, " -> ");
                    strcat(champs,buf);
                    printf("%s \n",champs);
                    continue;
                }

            }
            else
            {
                strcat(champs,filename);
                printf("%s \n",champs);
                lseek(archive,info.file_length,SEEK_CUR);
            }
        }

	}

    unlockfile(archive);

    close(archive);

    if(err == -1)
    {
        return -1;
    }

	return 0;
}


/* Ecrire le contenu d'un fichier dans un autre (utilisée dans supprimer_fichiers)*/
int ecrire_fichier_sauvegarde(int fdArchive,int fdFichier, Entete *info,char *filename, char *buf, int bufsize)
{
	int j, lus;
	int err = 0;

	if(info == NULL || filename == NULL || buf == NULL) return -1;


    if(ecrireEntete(fdFichier,info,filename) == -1)
    {
#ifdef DEBUG
        printf("ERREUR : ecriture entete fichier %s \n", filename);
#endif
        return -1;
    }


	for(j = 0 ; j < info->file_length/bufsize; j++)
	{
		lus = read(fdArchive,buf, bufsize);

        if(lus > 0)
            write(fdFichier,buf,lus);
        else
        {
#ifdef DEBUG
            printf("ERREUR : lecture contenu fichier %s \n", filename);
#endif
            err = -1;
            break;
        }
	}

    if(err == 0)
    {
        lus = read(fdArchive,buf, info->file_length % bufsize);
        write(fdFichier,buf,lus);
	}

	return 0;
}


char *remplirChamps(const Entete *info, char *champs)
{
    char buf_time[CHAMPSMAX];
    char tmp_str[CHAMPSMAX];
    int i;

    if(info == NULL || champs == NULL)
        return NULL;

    memset(champs,0,BUFSIZE);

    sprintf(tmp_str,"%o",info->mode);


    if(S_ISDIR(info->mode))
    {
        champs[0] = 'd';
    }
    else if(S_ISLNK(info->mode))
    {
        champs[0] = 's';
    }
    else if(S_ISREG(info->mode))
    {
        champs[0] = '-';
    }
    else
    {
        fprintf(stderr,"Format invalide\n");
        return NULL;
    }

    for(i = strlen(tmp_str) - 3; i < strlen(tmp_str);i++)
    {
        switch(tmp_str[i])
        {
            case '1' : strcat(champs,"--x");
                        break;

            case '2' : strcat(champs,"-w-");
                        break;

            case '3' : strcat(champs,"-wx");
                        break;

            case '4' : strcat(champs,"r--");
                        break;

            case '5' : strcat(champs,"r-x");
                        break;

            case '6' : strcat(champs,"rw-");
                        break;

            case '7' : strcat(champs,"rwx");
                        break;

            default :  strcat(champs,"---");
                        break;

        }

    }

    strcat(champs," ");   /*On concatène avec ' ' */

    /* On met la taille du fichier */
    sprintf(tmp_str,"%d",(int)info->file_length);

    for(i = strlen(tmp_str); i < 9;i++)
    {
        strcat(champs," ");
    }
    strcat(champs,tmp_str);

    strcat(champs," ");

    /* On met la date */
    strftime(buf_time, 20, "%b. %d %H:%M ", localtime(&info->m_time));
    strcat(champs,buf_time);


    return champs;
}




















