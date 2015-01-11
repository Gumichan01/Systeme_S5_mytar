

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
	int param = 0; /* on met param à 0 mais ça peut changer */
	Option sp;

	init(&sp);	/* On met tous les champs à zero */

	if(argc < 2 )
	{
		fprintf(stderr," %s : Manque d'arguments\n %s -h  pour savoir comment utiliser le programme\n",argv[0],argv[0]);
		return EXIT_FAILURE;
	}

	if( ( nb_options = check_param(argc,argv,&sp)) == -1)
	{
		fprintf(stderr," %s : Problème avec les options du programme\n %s -h  pour savoir comment utiliser le programme\n",argv[0],argv[0]);
		return EXIT_FAILURE;
	}

#ifdef DEBUG
	printf("DEBUG : Il y a %d paramètres actifs\n", nb_options);
#endif

	/*Aucune option ? option -h actif ?*/
	if(nb_options== 0 || sp.flag_h)
	{	/* On n'a aucun arametre ou l'option -h est active */
		usage(argv[0]);
	}


#ifdef DEBUG
	printf("DEBUG : sp.flag_f : %d \n", sp.flag_f);
#endif

	/* Si le sp.flag -f n'est pas présent, arrêt du programme */
	if(!sp.flag_f)
	{
		fprintf(stderr," %s : Erreur fatal, option -f (obligatoire) non présente\n %s -h pour savoir comment utiliser le programme\n",argv[0],argv[0]);
		return EXIT_FAILURE;
	}

    /*  Récupération des informations
        (nom de l'archive, repertoire racine si '-C' actif) */
	getArchive(archive_file,argc,argv);
	param = getFirstPath(argc,argv);


    if(strstr(archive_file,".mtr") == NULL)
    {
        fprintf(stderr,"%s : je refuse de traiter un fichier non valide. '%s' n'est pas un fichier '.mtr' \n",argv[0], archive_file);
        return EXIT_FAILURE;
    }


#ifdef DEBUG
	printf("DEBUG : Archive : %s ; param : %d ; \n", archive_file, param);
#endif


	if(sp.flag_c)	/* Si le sp.flag -c existe -> on crée l'archive */
	{

#ifdef DEBUG
		printf("DEBUG : sp.flag_c actif -> création archive \n");
#endif

		if(creer_archive(archive_file,param,argc, argv,&sp) == -1)
		{
			fprintf(stderr,"%s : impossible de créer l'archive \n",argv[0]);
			return EXIT_FAILURE;
		}

	}
	else if(sp.flag_x)
	{
#ifdef DEBUG
		printf("DEBUG : sp.flag_x actif -> extraction archive \n");
#endif

		if(extraire_archive(archive_file,param,argc,argv,&sp) == -1)
		{
			fprintf(stderr,"%s : impossible d'extraire l'archive, le fichier est peut-être invalide ! \n",argv[0]);
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
			fprintf(stderr,"%s : Problème lors de l'ajout des fichiers dans l'archive %s \n",argv[0], archive_file);
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
			fprintf(stderr,"%s : Problème lors de la suppression des fichiers de l'archive %s \n",argv[0],archive_file);
			return EXIT_FAILURE;
		}

	}
	else if(sp.flag_l)
	{
#ifdef DEBUG
		printf("DEBUG : sp.flag_l actif -> liste des fichiers dans archive \n");
#endif

		if(liste_fichiers(archive_file, &sp,argc,argv) == -1)
		{
			fprintf(stderr,"%s : Problème lors de l'affichage des fichiers situés dans l'archive %s \n",argv[0],archive_file);
			return EXIT_FAILURE;
		}
	}
	else
	{
        printf("C'est bien beau de me donner '%s', mais je ne sais pas quoi faire avec !!\n", archive_file);
        printf("RYFM -> %s -h\n",argv[0]);
	}

	return EXIT_SUCCESS;
}







