

/**
*	@file lock_lib.h
*
*	@brief Une implementation "haut niveau" du système de verrou
*
*	@author Luxon JEAN-PIERRE, Kahina RAHANI
*	Licence 3 Informatique
*
*/

#include <fcntl.h>

#ifndef LOCK_LIB_H
#define LOCK_LIB_H

/*  Fonction lockfile met le verrou sur
    tout le fichier avec les parametres par defaut */
int lockfile(int fd);

/*  Fonction qui déverrouilles le fichier */
int unlockfile(int fd);

/* Fonction lock_reg met le verrou sur une partie du fichier */
int lock_reg(int fd, int cmd, short l_type, int offset, short whence, short l_len);

/* On demande la pose non-bloquante d'un verrou de lecture (verrous partagés) */
#define read_lock(fd,offset,whence,len) \
	lock_reg((fd),F_SETLK,F_RDLCK,(offset),(whence),(len))

/* On demande la pose bloquante cette fois-ci du verrou de lecture (w/W : wait) */
#define readw_lock(fd,offset,whence,len) \
	lock_reg((fd),F_SETLKW,F_RDLCK,(offset),(whence),(len))

/* On demande la pose non-bloquante d'un verrou d'ecriture (verrou exclusif) */
#define write_lock(fd,offset,whence,len) \
	lock_reg((fd),F_SETLK,F_WRLCK,(offset),(whence),(len))


/* On demande la pose bloquante cette fois-ci d'un verrou d'ecriture (verrou exclusif) */
#define writew_lock(fd,offset,whence,len) \
	lock_reg((fd),F_SETLKW,F_WRLCK,(offset),(whence),(len))

/* On retire le verrou */
#define un_lock(fd,offset,whence,len) \
	lock_reg((fd),F_SETLKW,F_UNLCK,(offset),(whence),(len))


#endif /* LOCK_LIB_H */





















