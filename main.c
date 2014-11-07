

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


//#define DEBUG



int main(int argc, char **argv)
{	
	int nb_param;
	char archive_file[MAX_FILE];
	int param = 0, i; // on met param à 0 mais ça peut changer 


	if(argc < 2 )
	{
		fprintf(stderr,"%s : Manque d'arguments, faire %s -h  pour savoir comment utiliser le programme\n",argv[0],argv[0]);
		return EXIT_FAILURE;
	}

	if( ( nb_param = check_param(argc,argv)) == -1)
	{
		fprintf(stderr," %s : Problème avec les options du programme, faire %s -h  pour savoir comment utiliser le programme\n",argv[0],argv[0]);
		return EXIT_FAILURE;
	}

#ifdef DEBUG
	printf("DEBUG : Il y a %d paramètres actifs\n", nb_param);
#endif

	getArchive(archive_file,argc,argv);
	param = getFirstPath(argc,argv);


	if(flag_h)	//Si le flag -h est présent -> appel de usage
	{
		usage(argv[0]);	
	}

	// Si le flag -f n'est pas présent, arrêt du programme
	if(!flag_f)
	{
		fprintf(stderr," %s : Erreur fatal, option -f (obligatoire) non présente, faire %s -h  pour savoir comment utiliser le programme\n",argv[0],argv[0]);
		return EXIT_FAILURE;
	}

	if(flag_c)	// Si le flag -c existe -> on crée l'archive
	{

#ifdef DEBUG
		printf("DEBUG : flag_c actif -> création archive \n");
#endif

		if(creer_archive(archive_file,param,argc, argv) == -1)
		{
			fprintf(stderr,"%s : impossible de créer l'archive \n",argv[0]);
			return EXIT_FAILURE;
		}

	}
	else if(flag_x)
	{
#ifdef DEBUG
		printf("DEBUG : flag_x actif -> extraction archive \n");
#endif

		if(extraire_archive(archive_file) == -1)	// à modifier !!!!!
		{
			fprintf(stderr,"%s : impossible d'extraire l'archive \n",argv[0]);
			return EXIT_FAILURE;
		}


	}
	else if(flag_a)
	{
#ifdef DEBUG
		printf("DEBUG : flag_a actif -> ajout fichier dans archive \n");
#endif

		for(i = param; i < argc; i++)
		{
			if(ajouter_fichier(archive_file,argv[i]) == -1)	// à modifier !!!!!
			{
				fprintf(stderr,"%s : impossible d'ajouter le fichier %s dans l'archive %s \n",argv[0], argv[i], archive_file);
				//return EXIT_FAILURE;
			}
		}
	}
	else if(flag_d)
	{
#ifdef DEBUG
		printf("DEBUG : flag_d actif -> suppression fichier dans archive \n");
#endif

		for(i = param; i < argc; i++)
		{
			if(supprimer_fichier(archive_file,argv[i]) == -1)	// à modifier !!!!!
			{
				fprintf(stderr,"%s : impossible de supprimer le fichier %s de l'archive %s \n",argv[0],argv[i],archive_file);
				//return EXIT_FAILURE;
			}
		}

	}
	else if(flag_v)
	{
#ifdef DEBUG
		printf("DEBUG : flag_v actif -> verification archive \n");
#endif

		if(verifier_archive(archive_file) == -1)	// à modifier !!!!!
		{
			fprintf(stderr,"%s : problème lors de la vérification de l'archive \n",argv[0]);
			return EXIT_FAILURE;
		}


	}

	return EXIT_SUCCESS;
}







