

/**
*	@file lock_lib.c
*
*	@brief Une implementation "haut niveau" du système de verrou
*
*/


#include "lock_lib.h"


/*
	Type de verrouillage

	F_RDLCK : verrou en lecture (plusieurs verrous peuvent cohabiter)
	f_WRLCK : verrou en ecriture (pas d'autre verrou possible)

*/

/* On pose un verrou (par defaut, verrou exclusif d'écriture sur tout le fichier) */
int lockfile(int fd)
{
	struct flock fl;

	fl.l_type = F_WRLCK;	/* exclusif */
	fl.l_start = 0;		    /* debut du verrou (içi, au début du fichier) */
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;		    /* Si l_len == 0 -> englobe tout le fichier */

	return fcntl(fd, F_SETLK, &fl);
}


int unlockfile(int fd)
{
	struct flock fl;

	fl.l_type = F_UNLCK;	/* déverouillage */
	fl.l_start = 0;		    /* debut du verrou (içi, au début du fichier) */
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;		    /* Si l_len == 0 -> englobe tout le fichier */

	return fcntl(fd, F_SETLK, &fl);
}



/* On pose un verrou selon le souhait de l'utilisateur */
int lock_reg(int fd, int cmd, short l_type, int offset, short whence, short l_len)
{
	struct flock fl;

	fl.l_type = l_type;	    /* L_RDLCK, L_RWLCK ou UNLCK */
	fl.l_start = offset;	/* debut du verrou (position relative à whence) */
	fl.l_whence = whence;	/* SEEK_SET, SEEK_CUR , SEEK_END */
	fl.l_len = l_len;	    /* Si l_len == 0 -> englobe tout le fichier */

	return fcntl(fd, cmd, &fl);
}









