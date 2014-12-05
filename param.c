

/**
*
*	@file param.c
*
*	@brief Fichier bibliothèque param.h
*
*	Il gère les paramètre du programme
*
*	@author Luxon JEAN-PIERRE, Kahina RAHANI
*	Licence 3 Informatique
*
*/


#include "param.h"

/* On met tous les champs à 0, c'est plus efficace que memset() */
void init(Parametres *sp)
{
	if(sp != NULL)
	{
		sp->flag_h = 0;
		sp->flag_c = 0;
		sp->flag_a = 0;
		sp->flag_d = 0;
		sp->flag_l = 0;
		sp->flag_x = 0;
		sp->flag_k = 0;
		sp->flag_s = 0;
		sp->flag_C = 0;
		sp->flag_v = 0;
		sp->flag_f = 0;
	}
}



/* Fait une vérification des paramètres du programme */
/* renvoie un nombre de paramètre >=0 si tout s'est bien passé, -1 sinon */
int check_param(int argc, char **argv, Parametres *sp)
{

	int compt = 0;
	int i;


	if(sp == NULL) return -1;	/* Structure invalide */


	for(i = 1; i<argc; i++)
	{
		if(!strcmp(argv[i],"-h"))
		{
			if(sp->flag_h)
			{
				
				return -1;
			}
			else
			{
				sp->flag_h = 1;
				compt += 1;
			}
		}
		
		if(!strcmp(argv[i],"-c"))
		{
			if(sp->flag_a || sp->flag_c || sp->flag_d || sp->flag_x )
			{
				
				return -1;
			}
			else
			{

				sp->flag_c = 1;
				compt += 1;
			}
		}
		if(!strcmp(argv[i],"-a"))
		{
			if(sp->flag_a || sp->flag_c || sp->flag_d || sp->flag_x )
			{
				
				return -1;
			}
			else
			{
				sp->flag_a = 1;
				compt += 1;
			}
		}

		if(!strcmp(argv[i],"-d"))
		{
			if(sp->flag_a || sp->flag_c || sp->flag_d || sp->flag_x )
			{
				
				return -1;
			}
			else
			{
				sp->flag_d = 1;
				compt += 1;
			}
		}

		if(!strcmp(argv[i],"-l"))
		{
			if(sp->flag_l)
			{
				
				return -1;
			}
			else
			{
				sp->flag_l = 1;
				compt += 1;
			}
		}

		if(!strcmp(argv[i],"-x"))
		{
			if(sp->flag_a || sp->flag_c || sp->flag_d || sp->flag_x )
			{
				
				return -1;
			}
			else
			{
				sp->flag_x = 1;
				compt += 1;
			}
		}

		if(!strcmp(argv[i],"-k"))
		{
			if(sp->flag_k)
			{
				
				return -1;
			}
			else
			{
				sp->flag_k = 1;
				compt += 1;
			}
		}

		if(!strcmp(argv[i],"-s"))
		{
			if(sp->flag_s)
			{
				
				return -1;
			}
			else
			{
				sp->flag_s = 1;
				compt += 1;
			}
		}

		if(!strcmp(argv[i],"-C"))
		{
			if(sp->flag_C)
			{
				
				return -1;
			}
			else
			{
				sp->flag_C = 1;
				compt += 1;
			}
		}

		if(!strcmp(argv[i],"-v"))
		{
			if(sp->flag_v)
			{
				return -1;
			}
			else
			{
				sp->flag_v = 1;
				compt += 1;
			}
		}

		if(!strcmp(argv[i],"-f"))
		{
			if(sp->flag_f)
			{
				
				return -1;
			}
			else
			{
				sp->flag_f = 1;
				compt += 1;
			}
		}

	}

	return compt;
}

/* Stocke le nom du fichier archive dans buf et renvoie l'adresse de buf */
char * getArchive(char *buf,int argc,char **argv)
{
		int i = 1;
		while((i<argc) && strcmp(argv[i],"-f")){
			i++;
		}

		if(i >= argc-1){
			return NULL;
		}


		if(buf != NULL)
			strcpy(buf,argv[i+1]);
		else
			return NULL;


	return buf;
}



int getFirstPath(int argc, char **argv)
{
	int i = 1;

	while(i<argc && strcmp(argv[i],"-f") && strcmp(argv[i],"-a") && strcmp(argv[i],"-d")){

		i++;
	}

	if(i == argc)
	{
		return -1;
	}

	/* Si on rencontre le flag -f */
	if(!strcmp(argv[i],"-f")){
		if(i>=argc - 2){
			return -1;
		}

		return i+2;
	}

	/* Si on rencontre le flag -a ou -d */
	if(!strcmp(argv[i],"-a") || !strcmp(argv[i],"-d")){
		return i+1;
		}

	return -1;
}



