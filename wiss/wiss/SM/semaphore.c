
/********************************************************/
/*                                                      */
/*               WiSS Storage System                    */
/*          Version SystemV-4.0, September 1990	        */
/*                                                      */
/*              COPYRIGHT (C) 1990                      */
/*                David J. DeWitt 		        */
/*               Madison, WI U.S.A.                     */
/*                                                      */
/*	         ALL RIGHTS RESERVED                    */
/*                                                      */
/********************************************************/


#
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <signal.h>
#include <assert.h>
#include <stdio.h>


union semun {
    int val;
    struct semid_ds *buf;
    ushort	*array;  /* was array[] but mips compiler choked */
};
#ifdef ultrix
union semun {
    int val;
    struct semid_ds *buf;
    ushort	*array;  /* was array[] but mips compiler choked */
};
#endif

/*  Function Header 
 *
 * Name         : InitSem
 * Purpose      : Creates a semaphore and sets its initial value to 0 or 1.
 * Arguments    : sem_ptr   - address where semid is to be written
 *                start_val - initial value
 * Return value : void
 * Side effects : *sem_ptr is initialized to contain a semid
 * Comments     : Access right is given irrespective of uid.
 *
 */

InitSem(sem_ptr, start_val, key)
/*----------------------------*/
int    	*sem_ptr;
int	start_val;
key_t	key;
{
  static union semun param ;	
  static int ret_code; 	/* diagnosis - returned by semctl */

  /* Create semaphore and write its id at sem_ptr */
  *sem_ptr = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL) ;
  if (*sem_ptr < 0) printf("semget returned error %d in InitSem\n",*sem_ptr);
  
   /* now initialize the semaphore */
   param.val = start_val ;
   ret_code = semctl(*sem_ptr, 0, SETVAL, param) ;
  if (ret_code < 0) 
	printf("semctl returned error %d in InitSem\n",ret_code);
}

/*  Function Header 
 *
 * Name         : RmSem
 * Purpose      : Removes a semaphore .
 * Arguments    : sem_ptr   - address of the semid to remove
 * Return value : none
 * Side effects : 
 * Comments     : 
 *
 */

RmSem(sem_ptr)
/*----------*/
int    	*sem_ptr;
{
  static int ret_code; 	/* diagnosis returned by semctl */
  ret_code = semctl(*sem_ptr, 0, IPC_RMID, 0);
  if (ret_code < 0) printf("semctl returned %d in RmSem\n",ret_code);
}

/*  Function Header 
 *
 * Name         : WaitSem
 * Purpose      : P operation on a semaphore
 * Arguments    : sem_ptr - pointer to the id of the semaphore
 * Return value : void
 * Side effects : standard P semantics
 * Comments     :
 *
 */

struct sembuf p_op = { 
  0,   /* semaphore index */
  -1,  /* P is a decrementation */
  0
} ;


WaitSem(sem_ptr)
int    *sem_ptr;
{
    int e;
    e = semop(*sem_ptr, &p_op, 1);
    if (e < 0) printf("semop() returned error %d in WaitSem\n",e);
}



/*  Function Header 
 *
 * Name         : SendSem
 * Purpose      : V operation on a semaphore
 * Arguments    : sem_ptr - pointer to the id of the semaphore
 * Return value : void
 * Side effects : standard V semantics
 * Comments     :
 *
 */

struct sembuf v_op = { 
  0,   		/* semaphore index */
  1, 		/* V is an incrementation */
  0
  } ;


SendSem(sem_ptr)
int    *sem_ptr;
{
    int e;
    e = semop(*sem_ptr, &v_op, 1);
    if (e < 0) printf("semop() returned error %d in SendSem\n",e);
}


