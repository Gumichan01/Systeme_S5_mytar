

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
	int nb_param;
	char archive_file[MAX_FILE];
	int param = 0; /* on met param à 0 mais ça peut changer */
	Parametres sp;

	init(&sp);	/* On met tous les champs à zero */

	if(argc < 2 )
	{
		fprintf(stderr,"%s : Manque d'arguments, faire %s -h  pour savoir comment utiliser le programme\n",argv[0],argv[0]);
		return EXIT_FAILURE;
	}

	if( ( nb_param = check_param(argc,argv,&sp)) == -1)
	{
		fprintf(stderr," %s : Problème avec les options du programme, faire %s -h  pour savoir comment utiliser le programme\n",argv[0],argv[0]);
		return EXIT_FAILURE;
	}

#ifdef DEBUG
	printf("DEBUG : Il y a %d paramètres actifs\n", nb_param);
#endif

	/*Aucun parametre ? option -h actif ?*/
	if(nb_param == 0 || sp.flag_h)
	{	/* On n'a aucun arametre ou l'option -h est active */
		usage(argv[0]);
	}


#ifdef DEBUG
	printf("DEBUG : sp.flag_f : %d \n", sp.flag_f);
#endif

	/* Si le sp.flag -f n'est pas présent, arrêt du programme */
	if(!sp.flag_f)
	{
		fprintf(stderr," %s : Erreur fatal, option -f (obligatoire) non présente, faire %s -h  pour savoir comment utiliser le programme\n",argv[0],argv[0]);
		return EXIT_FAILURE;
	}

	getArchive(archive_file,argc,argv);
	param = getFirstPath(argc,argv);


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

		if(extraire_archive(archive_file,&sp) == -1)
		{
			fprintf(stderr,"%s : impossible d'extraire l'archive \n",argv[0]);
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

		if(liste_fichiers(archive_file, &sp) == -1)
		{
			fprintf(stderr,"%s : Problème lors de l'affichage des fichiers situés dans l'archive %s \n",argv[0],archive_file);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}







