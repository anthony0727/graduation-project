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
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <signal.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
 
#ifndef LOCK_EX
#define	USE_LOCKF
#include <unistd.h>
extern int errno;
#endif
 
/*
 
   This module initializes the shared data structures that are
   stored in virtual memory
 
*/
 
#include	<wiss.h>
#include	<page.h>
#include 	<sm.h>
 
#ifndef ultrix
#define	sm_count	shm_nattch
#endif
 
extern	char	*shmAlloc();
extern  char    *getenv();
extern	void	*shmat();
 
SMDESC		*smPtr;   /* pointer to shared memory segment */
int		procNum;  /* index of this process in smPtr->users[] table */
static int 	shmid;    /* unique id of shared memory segment */
int 		keyFd;    /* file descriptor for keyfile */
 
int num_maxbufs=512;
#define	FREEHEAPSPACE 20*1024*1024  /* amount of free shared memory space
	left after allocating space for the buffer pool and buffer table.
	This is a total guess and perhaps may prove to be too small */
 
static
LockFile(fd)
	int fd;
{
	int e;
#ifdef USE_LOCKF
	lseek(fd, 0L, 0);
	e = lockf(fd,F_LOCK,0);
	if (e < 0) {
		printf("error return %d,errno=%d from lockf(lock)\n", e, errno);
		printf("\tFD = %d, keyFd = %d\n", fd, keyFd);
	}
#else
    	e = flock(fd,LOCK_EX);
    	if (e < 0) printf("error return %d from flock\n",e);
#endif
}
 
static
UnlockFile(fd)
	int fd;
{
	int e;
#ifdef USE_LOCKF
	lseek(fd, 0L, 0);
	e = lockf(fd,F_ULOCK,0);
	if (e < 0) {
		printf("error return %d,errno=%d from lockf(unlock)\n", e, errno);
		printf("\tFD = %d, keyFd = %d\n", fd, keyFd);
	}
#else
    	e = flock(fd,LOCK_UN);
    	if (e < 0) printf("error return %d from flock\n",e);
#endif
}
 
wiss_init()
{
    sm_init();
    am_init();
}
 
wiss_final()
{
	int pid, i, e;
        struct shmid_ds shmbuf;
	int	lastProc;
 
	am_final();
	pid = getpid();
	i = 0;
	/* first lock the key file in exclusive mode */
	/* in order to make sure that that if another process starts */
	/*  at the same time this process is flushing things to disk */
	/*  that the processing starting will block.  */
 
	LockFile(keyFd);
 
	/* look for the process in the users table */
	while ((smPtr->users[i].pid != pid) && (i < MAXPROCS)) i++;
	if (i == MAXPROCS)
	{
	    printf("pid %d NOT in users table. error!!!\n",pid);
	    exit(1);
	}
	else
	{
	    smPtr->userCnt--;
	    smPtr->users[i].pid = -1;  /* mark entry free */
	    /* perhaps should add a commit/abort transaction call here ??? */
	    /* if the transaction is still active */
	    smPtr->users[i].transId = -1; /* no active trans */
	}
 
        /* get status of shared memory segment */
        shmctl (shmid, IPC_STAT, &shmbuf);
 
        printf("number of attached processes at wiss_final = %d\n",
		shmbuf.sm_count);
 
	/* remove the process's semaphore */
		//james 161010
		semctl(smPtr->users[procNum].sem.semid, 0, IPC_RMID, 0);
		for (i = 0; i < MAXTHREADS; i++)	{
			semctl(smPtr->users[procNum].sems[i].semid, 0, IPC_RMID, 0);
		}
		//james 161010
 
        /* check the number of attached processes */
        /* if this process is the last, flush the shared memory */
 
	if (shmbuf.sm_count  == 1)
        {
	    /* current process is the last one, flush shared structures */
	    /* to disk and then delete shared memory segment */
 
	    sm_final();
	    /* detach this process */
	    if (shmdt(smPtr) < 0)
		perror("shmdt detach error in wiss_final\n");
 
	    /* finally delete the actual shared memory segment */;
	    if (shmctl (shmid, IPC_RMID, 0) != 0)
		perror("shmctl remove error in wiss_final\n");
	    lastProc = TRUE;
	}
	else  /* not last user, detach the process from the memory segment */
	{
		if (shmdt(smPtr) < 0)
			perror("shmdt detach error in wiss_final\n");
		lastProc = FALSE;
	}
/*
	if (lastProc) unlink(SHM_KEYFILE);
*/
 
    	/* unlock the keyfile so other processes may start */
	UnlockFile(keyFd);
 
	close(keyFd);
	return(0);
}
 
sm_init()
{
 
    register int	i;		/* for going along VolDev */
    int		NumBuffers;
    key_t 	smKey;  /* key for use with shared memory calls */
    int 	oum;
    struct shmid_ds shmbuf;
    key_t	semKey; /* key for getting a semaphore */
    int		e;
    int		hashSize;
    int		shmflg;
	size_t	heapsize, totalSMsize;
 
/*
    oum = umask(0);
*/
    if ((keyFd=open(SHM_KEYFILE, O_CREAT|O_WRONLY, 0777)) < 0)
    {
	printf("shmsbrk: cannot access key file");
	exit(1);
    }
    else
    {
	/* write a few bytes of garbage into the file */
	write(keyFd, "abcd",4);
    }
/*
    umask(oum);
*/
 
    /* the first process in locks the file in exclusive mode in case
       two processes start simultaneously. This will insure that
       only one process executes sm_init simultaneously */
 
    LockFile(keyFd);
 
    /* next generate a key for the shared-memory file */
    smKey = ftok(SHM_KEYFILE, ' ');
    if (smKey == -1)
    {
	printf("ftok returned NULL\n");
	UnlockFile(keyFd);
	exit(1);
    }
 
    /* then generate a unique semKey for this process */
    semKey = smKey + getpid();
 
    /* calculate size of shared memory segment to get */
    NumBuffers = num_maxbufs;  /* number of buffers in buffer pool */
 
    /* calculate total heapsize (in bytes) */
    hashSize = (NumBuffers*3)-1;
 
	heapsize = (size_t)NumBuffers*((size_t)PAGESIZE + sizeof(BUFTAB))
		+ (size_t)hashSize*(2 * sizeof(HASHBUCKET) + sizeof(LATCH))
		+ (size_t)FREEHEAPSPACE;
 
    totalSMsize = heapsize + sizeof(SMDESC);
 
/*
    printf("size of SMDESC = %d\n", sizeof(SMDESC));
    printf("size of heap = %d\n", heapsize);
    printf("total size of shared segment requested = %d\n", totalSMsize);
*/
 
    /* get the shared memory segment */
    shmid = shmget(smKey, totalSMsize, IPC_CREAT | 0777);
    if (shmid < 0)
    {
	printf("shmget of shared memory block of size %lureturned error %d\n",
		totalSMsize, shmid);
	UnlockFile(keyFd);
	exit(1);
    }
 
    /* finally attach the shared memory segment to the process */
 
#ifdef hpux
    /* no specific address */
    smPtr = (SMDESC *) shmat(shmid, (char *)0, 0);
#else
/*
    Use the following code on sparcs and decstations to specify a hard address
    This is necessary because SUNOS and Ultrix don't guarantee that all
    processes attached to the same shared memory segments actually
    will get the same address.  There seems to be some sensitivity
    to the address actually selected.  For example, 0xf0000000 worked
    fine on a Sun IPC but not on a Sparc10.   0xd0000000 seems
    to work on both machines.
*/
 
    shmflg = 0 | SHM_RND;
    smPtr = (SMDESC *) shmat(shmid, (char *)0, shmflg);
    if (((int) smPtr) == -1)
    {
        perror("shmat returned error\n");
        UnlockFile(keyFd);
        exit(1);
    }
#endif
 
 
/*
    printf("shmid returned was %d\n",shmid);
    printf("value of smPtr = %x, addr of heap = %x\n",smPtr, smPtr->heap);
*/
 
 
    /* get status of shared memory segment */
    shmctl (shmid, IPC_STAT, &shmbuf);
#ifdef TRACE
    printf("number of attached processes = %d\n",shmbuf.sm_count);
#endif TRACE
 
    /* check the number of attached processes */
    /* if this process is the first, initialize the shared memory */
    if (shmbuf.sm_count  == 1)
    {
        /* second, initialize the heap in shared memory for shmAlloc to use */
        smPtr->search_ptr = NULL;
        smPtr->heap_top = NULL;
        /* heap is allocated in terms of heapword units */
        smPtr->heapSize = heapsize/(sizeof(union heapWord));
 
        /* next initialize all the shared-memory data structures relating */
        /* to the buffer pool */
 
        smPtr->clock_hand = -1;
        smPtr->hash_size = hashSize;
        smPtr->bf_num_free = NumBuffers;
        smPtr->bf_num_bufs = NumBuffers;
 
        /* initialize the transaction counter */
        smPtr->transCounter = 1;
 
        /* now malloc the buffer pool and the buffer
	   table out of shared memory */
 
        smPtr->bufferpool = (PAGE *) shmAlloc ((size_t)NumBuffers * (size_t)PAGESIZE);
        if (smPtr->bufferpool == NULL)
        {
 	    printf("malloc of buffer pool with %d pages failed !!! \n",
		NumBuffers);
	    UnlockFile(keyFd);
	    exit(1);
        }
        smPtr->buftable =(BUFTAB *) shmAlloc ((size_t)NumBuffers * sizeof(BUFTAB));
        if (smPtr->buftable == NULL)
        {
	    printf("malloc of buffer table with %d entries failed !!! \n",
		NumBuffers);
	    UnlockFile(keyFd);
	    exit(1);
        }
/*
        printf("addr of bufpool = %x, addr of buftab=%x\n",smPtr->bufferpool,
		smPtr->buftable);
*/
 
        for(i = 0; i < NumBuffers; i++)
        {
	    smPtr->buftable[i].Bvalid = FALSE;
	    smPtr->buftable[i].Bdirty = FALSE;
	    smPtr->buftable[i].busy = FALSE;
	    InitLatch(&smPtr->buftable[i].latch);
        }
        /* initialize the level 0 data structures in shared memory */
        io_init();
        /* initialize the level 1 data structures in shared memory */
	bf_init();
        /* initialize the level 2 data structures in shared memory */
        st_init();
        /* initialize the lock manager */
        lm_initialize_resources();
	/* initialize the user table */
	smPtr->userCnt = 1;
	smPtr->users[0].pid = getpid();
	smPtr->users[0].transId = -1; /* no active trans */
	procNum = 0;   /* this process got the first slot */
	/* mark all others free */
	for (i=1;i<MAXPROCS;i++) smPtr->users[i].pid = -1;
    }
    else
    {
	/* another process was already active so no need to initialize */
	/* the shared data structures */
	/* look for a free spot */
	i = 0;
	while ((smPtr->users[i].pid != -1) && (i < MAXPROCS)) i++;
	if (i == MAXPROCS)
	{
	    printf("TOO MANY SIMULTANEOUS USERS. MAXPROCS = %d\n",MAXPROCS);
	    UnlockFile(keyFd);
	    exit(1);
	}
	else
	{
	    (smPtr->userCnt)++;
	    smPtr->users[i].pid = getpid();
	    smPtr->users[i].transId = -1; /* no active trans */
	    procNum = i;   /* this process got the ith slot */
	}
    }
    /* now initialize the process's semaphore */
	//james 161010
	InitSem(&smPtr->users[procNum].sem.semid, 0, semKey + MAXTHREADS);
	atomic_flag_clear(&smPtr->users[procNum].sem.sync);
	for (i = 0; i < MAXTHREADS; i++)	{
		InitSem(&smPtr->users[procNum].sems[i].semid, 0, semKey + i);
		atomic_flag_clear(&smPtr->users[procNum].sems[i].sync);
	}
	//james 161010
/*
    printf("proc %d got semid = %d\n",procNum,
	smPtr->users[procNum].sem.semid);
*/
    printf("shmid returned was %d\n",shmid);
    printf("value of smPtr = %x, addr of heap = %x\n",smPtr, smPtr->heap);
    printf("value of &smPtr->hashBucket[0] = %x\n",&smPtr->hashBucket[0]);
    printf("address of smPtr = %x\n",&smPtr);
 
    /* unlock the keyfile so other processes may start */
    UnlockFile(keyFd);
}
 
