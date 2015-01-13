

/**
*
*	@file main.c
*
*	@brief Fichier main, il constitue le squelette du programme
*
*	@author Luxon JEAN-PIERRE, Kahina RAHANI
*	Licence 3 Informatique
*
*/

#include "mytar.h"




int main(int argc, char **argv)
{
	int nb_options;
	char archive_file[MAX_FILE];
	char archive_gz[MAX_FILE];  /* Archive après décompression */
	int param = 0;              /* on met param à 0 mais ça peut changer */
	int retval, len;            /*valeur de contôle et longeur d'un path*/
	Option sp;

	init(&sp);	/* On met tous les champs à zero */

	if(argc < 2 )
	{
		fprintf(stderr," %s : Manque d'arguments\n %s -h  pour savoir comment utiliser le programme\n",basename(argv[0]),basename(argv[0]) );
		return EXIT_FAILURE;
	}

	if( ( nb_options = check_param(argc,argv,&sp)) == -1)
	{
		fprintf(stderr," %s : Problème avec les options du programme\n %s -h  pour savoir comment utiliser le programme\n",basename(argv[0]),basename(argv[0]));
		return EXIT_FAILURE;
	}

#ifdef DEBUG
	printf("DEBUG : Il y a %d paramètres actifs\n", nb_options);
#endif

	/*Aucune option ? option -h actif ?*/
	if(nb_options== 0 || sp.flag_h)
	{	/* On n'a aucun arametre ou l'option -h est active */
		usage(basename(argv[0]));
	}


#ifdef DEBUG
	printf("DEBUG : sp.flag_f : %d \n", sp.flag_f);
#endif

	/* Si le sp.flag -f n'est pas présent, arrêt du programme */
	if(!sp.flag_f)
	{
		fprintf(stderr," %s : Erreur fatal, option -f (obligatoire) non présente\n %s -h pour savoir comment utiliser le programme\n",basename(argv[0]),basename(argv[0]));
		return EXIT_FAILURE;
	}

    /*  Récupération des informations
        (nom de l'archive, repertoire racine si '-C' actif) */
	getArchive(archive_file,argc,argv);
	param = getFirstPath(argc,argv);

    /* Le fichier a-til une sous-chaine ".mtr" ? */
    if(strstr(archive_file,".mtr") == NULL)
    {
        fprintf(stderr,"%s : je refuse de traiter un fichier non valide. '%s' n'est pas un fichier '.mtr' \n",basename(argv[0]), archive_file);
        return EXIT_FAILURE;
    }

    len = strlen(archive_file);

    if(strstr(archive_file,".gz") != NULL)
    {
        /* On verifie qu'on a bien un ficheir <nom>.mtr.gz */
        if(len < 7 || archive_file[len - 7] != '.')
        {
            fprintf(stderr,"%s : Bien essayé pour le \".gz\" en pleine chaine, mais je ne suis pas dupe \n",basename(argv[0]));
            return EXIT_FAILURE;
        }
    }
    else
    {
        /* A-t-on un fichier <nom>.mtr ?*/
        if(len < 4 || archive_file[len - 4] != '.')
        {
            fprintf(stderr,"%s : Bien essayé pour le \".mtr\" en pleine chaine, mais je ne suis pas dupe \n",basename(argv[0]));
            return EXIT_FAILURE;
        }
    }


#ifdef DEBUG
	printf("DEBUG : Archive : %s ; param : %d ; \n", archive_file, param);
#endif


	if(sp.flag_c)	/* Si le sp.flag -c existe -> on crée l'archive */
	{

#ifdef DEBUG
		printf("DEBUG : sp.flag_c actif -> création archive \n");
#endif

        /* Si '-z' est renseigné ET l'archive à la sous-chaine ".gz" */
        if(sp.flag_z && (strstr(archive_file,".gz") != NULL))
        {
            strcpy(archive_gz,archive_file);    /* on copie le nom dans archive_gz*/
            memset(archive_file, 0,strlen(archive_file));   /*On met des 0 */
            strncpy(archive_file,archive_gz,strlen(archive_gz) - 3);    /* on met le nom sans le ".gz" */
        }
        else if(strstr(archive_file,".gz") != NULL)
        {
            /* On a un fichier .gz alors que la compression n'est pas demandée */
            fprintf(stderr,"%s: fichier %s invalide pour l'archivage simple \n",basename(argv[0]), archive_file);
            return EXIT_FAILURE;
        }

        /* Dans tous les cas on archive dans un fichier .mtr */
		if(creer_archive(archive_file,param,argc, argv,&sp) == -1)
		{
			fprintf(stderr,"%s: impossible de créer l'archive \n",basename(argv[0]));
			return EXIT_FAILURE;
		}

		if(sp.flag_z)
		{
		    if(compresser(archive_file) == -1)
		    {
		        unlink(archive_file);
		        return EXIT_FAILURE;
		    }
		}

	}
	else if(sp.flag_x)
	{
#ifdef DEBUG
		printf("DEBUG : sp.flag_x actif -> extraction archive \n");
#endif
        /* Si on a un fichier compresé, on le décompresse d'abord*/
        if(sp.flag_z && (strstr(archive_file,".gz") != NULL))
        {
            memset(archive_gz,0,MAX_FILE);
            sprintf(archive_gz,".%d-%s",getpid(),archive_file);

            /*  On copie le contenu du fichier cible dans archive_gz
                Car on ne doit pas modifier le fichier existant */

            if(copy(archive_gz,archive_file) == -1)
                    return EXIT_FAILURE;

            strncpy(archive_file,archive_gz,strlen(archive_gz) - 3);

            if(decompresser(archive_gz) == -1)
            {
                    fprintf(stderr,"%s: impossible de décompresser l'archive, fichier invalide ! \n",basename(argv[0]));
                    unlink(archive_gz);
                    return EXIT_FAILURE;
            }

            retval = extraire_archive(archive_file,param,argc,argv,&sp);

            unlink(archive_gz);
            unlink(archive_file);
        }
        else
            retval = extraire_archive(archive_file,param,argc,argv,&sp);

        if(retval == -1)
        {
            fprintf(stderr,"%s : impossible d'extraire l'archive, fichier invalide ! \n",basename(argv[0]));
            return EXIT_FAILURE;
        }

	}
	else if(sp.flag_a)
	{
#ifdef DEBUG
		printf("DEBUG : sp.flag_a actif -> ajout fichier dans archive \n");
#endif

		if(ajouter_fichier(archive_file,param,argc,argv,&sp) == -1)
		{
			fprintf(stderr,"%s: Problème lors de l'ajout des fichiers dans l'archive %s \n",basename(argv[0]), archive_file);
			return EXIT_FAILURE;
		}

	}
	else if(sp.flag_d)
	{
#ifdef DEBUG
		printf("DEBUG : sp.flag_d actif -> suppression fichier dans archive \n");
#endif

		if(supprimer_fichiers(archive_file,param,argc,argv,&sp) == -1)
		{
			fprintf(stderr,"%s: Problème lors de la suppression des fichiers de l'archive %s \n",basename(argv[0]),archive_file);
			return EXIT_FAILURE;
		}

	}
	else if(sp.flag_l)
	{
#ifdef DEBUG
		printf("DEBUG : sp.flag_l actif -> liste des fichiers dans archive \n");
#endif

        if(sp.flag_z && (strstr(archive_file,".gz") != NULL))
        {
            memset(archive_gz,0,MAX_FILE);
            sprintf(archive_gz,".%d-%s",getpid(),archive_file);

            /*  On copie le contenu du fichier cible dans archive_gz
                Car on ne doit pas modifier le fichier existant */

            if(copy(archive_gz,archive_file) == -1)
                    return EXIT_FAILURE;

            strncpy(archive_file,archive_gz,strlen(archive_gz) - 3);

            if(decompresser(archive_gz) == -1)
            {
                    fprintf(stderr,"%s: impossible de décompresser l'archive, fichier invalide ! \n",basename(argv[0]));
                    unlink(archive_gz);
                    return EXIT_FAILURE;
            }

            retval = liste_fichiers(archive_file, &sp,argc,argv);

            unlink(archive_gz);
            unlink(archive_file);

        }
        else
            retval = liste_fichiers(archive_file, &sp,argc,argv);

		if(retval == -1)
		{
			fprintf(stderr,"%s: Problème lors de l'affichage des fichiers situés dans l'archive %s \n",basename(argv[0]),archive_file);
			return EXIT_FAILURE;
		}
	}
	else
	{
        printf("C'est bien beau de me donner '%s', mais je ne sais pas quoi faire avec !!\n", archive_file);
        printf("RYFM -> %s -h\n",basename(argv[0]));
	}

	return EXIT_SUCCESS;
}







