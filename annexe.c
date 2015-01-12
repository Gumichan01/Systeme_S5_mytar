

/**
*
*	@file annexe.c
*
*	@brief Fichier annexe.c
*
*	@author Luxon JEAN-PIERRE, Kahina RAHANI
*	Licence 3 Informatique
*
*/

/* Bibliothèque standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <time.h>

#include "annexe.h"
#include "mytar_posix_lib.h"



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

        close(tube[0]);

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
    strictement inférieur à la longueur d'un checksum
    renvoie -1.
*/
int checksumRenseigne(char * checksum)
{
    int i = 0;

    if(checksum == NULL)
        return -1; /* Le checksum n'est pas conforme, problème ! */

    /* Tant qu'on est pas à la fin et qu'on a rien de défini */
    while(i < CHECKSUM_SIZE && checksum[i] == 0)
    {
        i++;
    }

    /*  Si on est pas arrivé au bout, on considère que le checksum est défini */
    if(i != CHECKSUM_SIZE)
        return 1;

    return 0;   /* Si on arrive là, alors cela signifie que le checksum n'est pas défini */

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
    on a un fichier qui n'est pas dans une arborescence
    Attention : ne s'applique pas sur un répertoire */
char *getArborescence(char *filename, char *newA)
{
    int i;

    memset(newA,0,MAX_PATH);

    i = strlen(filename) - 1;

    while(i >= 0 && filename[i] != '/')
    {
        i--;
    }


    if(i == -1)
        return NULL;
    else
        strncpy(newA,filename,i+1);

    return newA;
}



/*  Concatène le chemin newF avec le chemin root.
    Cette fonction est utilisée lorsque l'option '-C' est active */
char *catRoot(char *rootRep,char *newF)
{
    char root[MAX_PATH];
    char tmp_fichier[MAX_PATH];

    /* rootRep ou newF est NULL ? -> on quitte */
    if( rootRep == NULL || newF == NULL )
        return NULL;

    if( enleverSlashEtPoints(rootRep,root) == NULL )
        return NULL;    /* Il y a eu un problème -> on quitte */

    if(root[strlen(root) - 1] != '/')
        root[strlen(root)] = '/';

    memset(tmp_fichier,0,MAX_PATH);
    strcpy(tmp_fichier,root);
    strcat(tmp_fichier,newF);

    memset(newF,0,MAX_PATH);
    strcpy(newF,tmp_fichier);

    return newF;
}




int compresser(char *archive_file)
{
	pid_t p;
	int status;
	char cmd[] = "gzip";

	if(archive_file == NULL)
		return -1;	/* Les paramètres ne sont pas valides, on ne va pas plus loin, on renvoie NULL */

	p = fork();

	if(p == 0)
	{

		execlp(cmd,cmd,archive_file, NULL);

		perror("Erreur lors de l'execution de gzip ");

		exit(-1);

	}
	else if(p > 0)
	{

		wait(&status);

		if(WIFEXITED(status))
		{
			/* Tout s'est bien passé */
			return WEXITSTATUS(status);
		}
		else
            return -1;

	}
	else
	{
		fprintf(stderr,"ERREUR : gzip, execution de la commande impossible\n");
		return -1;	/* La création du fork a echoué, on ne peut rien faire, retourne -1*/
	}

}


int decompresser(char *compressed_file)
{
	pid_t p;
	int status;
	char cmd[] = "gzip";

	if(compressed_file == NULL)
		return -1;	/* Les paramètres ne sont pas valides, on ne va pas plus loin, on renvoie NULL */

	p = fork();

	if(p == 0)
	{

		execlp(cmd,cmd,"-d",compressed_file, NULL);

		perror("Erreur lors de l'execution de gzip ");

		exit(-1);

	}
	else if(p > 0)
	{

		wait(&status);

		if(WIFEXITED(status))
		{
			/* Tout s'est bien passé */
			return WEXITSTATUS(status);
		}
		else
            return -1;

	}
	else
	{
		fprintf(stderr,"ERREUR : gzip, execution de la commande impossible\n");
		return -1;	/* La création du fork a echoué, on ne peut rien faire, retourne -1*/
	}
}

/* Copie un ficheir dans un autre */
int copy(char *dest, char *src)
{
	pid_t p;
	int status;
	char cmd[] = "cp";

	if(dest == NULL || src == NULL)
		return -1;	/* Les paramètres ne sont pas valides, on ne va pas plus loin, on renvoie NULL */

	p = fork();

	if(p == 0)
	{

		execlp(cmd,cmd,src,dest, NULL);

		perror("Erreur lors de l'execution de cp ");

		exit(-1);

	}
	else if(p > 0)
	{

		wait(&status);

		if(WIFEXITED(status))
		{
			/* Tout s'est bien passé */
			return WEXITSTATUS(status);
		}
		else
            return -1;

	}
	else
	{
		fprintf(stderr,"ERREUR : cp : execution de la commande impossible\n");
		return -1;	/* La création du fork a echoué, on ne peut rien faire, retourne -1*/
	}


}














