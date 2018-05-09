
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


#include        <wiss.h>
#include        <page.h>
#include        <sm.h>
#include        <lockquiz.h>
#include 	<LM_macro.h>
#include 	<LM_error.h>
#include 	<locktables.h>

extern SMDESC   *smPtr; /* ptr to common data structures in shared memory */
extern  int     procNum; /* index of the process in the smPtr->users table */

  
extern int check_deadlock();

char LM_locknames[MAXLOCKTYPES][4] = {
  "NL", "IS", "IX", "S", "SIX", "X"
  };

short LM_leaftype[MAXLOCKTYPES]= {
  FALSE, FALSE, FALSE, TRUE, FALSE, TRUE
  };

extern    struct    graph_bucket    *find_trans ();
extern    struct    wait_node       *addwaitbucket ();
extern    struct    wait_node       *addwaitbucket_u ();
extern              short           addowner ();
extern    LOCKTYPE  lock_strength ();

int	lmPageLocks;


/*
  extern    filetable_s               filetable[];
  extern    files_s                   files[];
  */

printlock (lockone)
     LOCKTYPE lockone;
{
  /*
   * ADD check for bounds of lockone XXX
   */
  printf("%s.\n",LM_locknames[lockone]);
}


/*
 *  findpage (page_id)
 *  Given a page_id, it hashes it and finds the node corresponding to it
 *  in the resource hash table.  If a node is found (ie. result != NULL),
 *  a latch is left set on the node.  Otherwise, a latch is left set on
 *  the lock table entry 
 */
struct    node    *findpage (page_id)
     PID     page_id;
{
  register struct  node    *finger;
  struct  node    *result;
  int	index;
  
#ifdef LMTRACE
  printf ("Calling findpage with page (%d, %d).\n",page_id.Pvolid,page_id.Ppage);
#endif


  /* begin by getting a latch on the proper locktable entry */
  /* we don't release this latch until we find the node we want */
  /* and we don't release it at all if the node is missing */
  /* we don't latch crab down this chain because someone */
  /* may very well be holding a latch on one of the entries */
  /* and it is not necessary to actually block unless nodes are */
  /* actually being added or deleted to the chain.  Anybody doing this */
  /* is required to obtain and hold the lock on the lock table entry */

  index = hash_page(page_id);
  SetLatch(&smPtr->locktable[index].concurrent, procNum, NULL);
  finger = smPtr->locktable[index].lockptr;
  result = NULL;
  while ((result == NULL) && (finger != NULL))
  {
    	if ((finger -> owner == PAGER) && 
           (finger->owner_id.page_id.Ppage == page_id.Ppage) &&
	   (finger -> owner_id.page_id.Pvolid == page_id.Pvolid))
    	{
	  	result = finger;
	}
	else finger = finger -> flink;
  }
  if (result != NULL) 
  {
       SetLatch(&result->concurrent, procNum, NULL);
       ReleaseLatch(&smPtr->locktable[index].concurrent, procNum);
  }
  return (result);
}


/*
 *  findfile (file_id)
 *  Given a file_id, it hashes it and finds the node corresponding to it
 *  in the resource hash table.  If a node is found (ie. result != NULL),
 *  a latch is left set on the node.  Otherwise, a latch is left set on
 *  the lock table entry 
 */
struct    node    *findfile (file_id)
     FID file_id;
{
  register    struct  node    *finger;
  struct  node    *result;
  int	index;
  
#ifdef LMTRACE
  printf ("Calling findfile with file (%d, %d).\n",file_id.Fvolid,file_id.Ffilenum);
#endif

  /* begin by getting a latch on the proper locktable entry */
  /* we don't release this latch until we find the node we want */
  /* and we don't release it at all if the node is missing */
  /* we don't latch crab down this chain because someone */
  /* may very well be holding a latch on one of the entries */
  /* and it is not necessary to actually block unless nodes are */
  /* actually being added or deleted to the chain.  Anybody doing this */
  /* is required to obtain and hold the lock on the lock table entry */

  index = hash_file(file_id);
  SetLatch(&smPtr->locktable[index].concurrent, procNum, NULL);
  finger = smPtr->locktable[index].lockptr;
  result = NULL;
  while ((result == NULL) && (finger != NULL))
  {
	if ((finger -> owner == FILER) 
	   && (finger -> owner_id.file_id.Fvolid == file_id.Fvolid)
	   && (finger -> owner_id.file_id.Ffilenum == file_id.Ffilenum))
    	{
	  	result = finger;
	}
  	else finger = finger -> flink;
  }
  if (result != NULL) 
  {
      SetLatch(&result->concurrent, procNum, NULL);
      ReleaseLatch(&smPtr->locktable[index].concurrent, procNum);
  }
  return(result);
}


LOCKTYPE getlockmode(file_id,trans_id)
     FID file_id;
     int trans_id;
{
  struct      node      *resource;
  register    struct    wait_node     *temp;
  int		fileIndex;

  
#ifdef LMTRACE
  printf ("-%d-Entering getlockmode for file (%d, %d) with trans %d.\n",
	  trans_id, file_id.Fvolid,file_id.Ffilenum,trans_id);
#endif
  
  fileIndex  = hash_file(file_id);
  resource = findfile(file_id);
  if (resource == NULL)
  {
#ifdef LMTRACE
      printf ("-%d-Resource has not been acquired by anybody, to upgrade lock request.\n",trans_id);
#endif
        ReleaseLatch(&smPtr->locktable[fileIndex].concurrent, procNum);
        return(l_NL);
  }
  if (resource -> trans_id == trans_id)
  {
        ReleaseLatch(&resource->concurrent, procNum);
    	return(resource->serving);
  }
  else {
    for (temp = resource -> o_list; 
	 ((temp != NULL) && (temp -> trans_id != trans_id)); 
	 temp = temp -> flink);
    if (temp == NULL) {
      
#ifdef LMTRACE
      printf ("-%d-ERROR (inside getlockmode)\n", trans_id);
      printf ("-%d-The given trans_id is not in the list of the owners.\n"
	      ,trans_id);
      printf ("-%d-The current owners of the resource are : \n",trans_id);
      printf ("-%d-\tThe main owner is %d.\n",
	      trans_id,resource -> trans_id);
      printf ("-%d-\tThe rest of the owners are : ", trans_id);
      for (temp = resource -> o_list; 
	   ((temp != NULL) && (temp -> trans_id != trans_id)); 
	   temp = temp -> flink) 
	printf(" %d ", temp -> trans_id);
      printf("\n");
#endif LMTRACE
      
      ReleaseLatch(&resource->concurrent, procNum);
      return(l_NL);
    }
    ReleaseLatch(&resource->concurrent, procNum);
    return (temp->serving);
  }
}

/* In order to reduce the cost of determining whether a transaction
has a particular file or page locked,  the locks that have
been acquired by a transaction are organized in two hash
tables fidHashTbl and pidHashTbl - which hang off the graph_bucket
for the transaction.  Since these are private hash tables they
are not in shared memory and do not need to be latched before
being accessed */


fidHashDelete(fid, htPtr, lockqptr)
    FID 		fid;
    struct fidLocalHash	*htPtr;
    struct lockq	*lockqptr;
{
    register int	i;
    register struct lockq *forePtr, *backPtr;

    i = HASHLOCALFID(fid);
    if (htPtr->fidChain[i] == lockqptr)
    {
	htPtr->fidChain[i] = lockqptr->collision;
    }
    else
    {
	backPtr = htPtr->fidChain[i];
	forePtr = backPtr->collision;
	while ((forePtr != lockqptr) && (forePtr != NULL))
	{
	    backPtr = forePtr;
	    forePtr = forePtr->collision;
	}
	if (forePtr == NULL)
	    printf("error in fidHashDelete, fid %d.%d not found\n",
		fid.Fvolid, fid.Ffilenum);
	else
	{
	    backPtr->collision = forePtr->collision;
	}
    }
}

pidHashDelete(pid, htPtr, lockqptr)
    PID 		pid;
    struct pidLocalHash *htPtr;
    struct lockq	*lockqptr;
{
    register int	i;
    register struct lockq *forePtr, *backPtr;

    i = HASHLOCALPID(pid);
    if (htPtr->pidChain[i] == lockqptr)
    {
	htPtr->pidChain[i] = lockqptr->collision;
	lockqptr->collision = NULL;
    }
    else
    {
	backPtr = htPtr->pidChain[i];
	forePtr = backPtr->collision;
	while ((forePtr != lockqptr) && (forePtr != NULL))
	{
	    backPtr = forePtr;
	    forePtr = forePtr->collision;
	}
	if (forePtr == NULL)
	    printf("error in pidHashDelete, pid %d.%d not found\n",
		pid.Pvolid, pid.Ppage);
	else
	{
	    backPtr->collision = forePtr->collision;
	    lockqptr->collision = NULL;
	}
    }
}



struct    lockq    *is_file_acquired (trans, file_id)
     struct    graph_bucket    *trans;
     FID       file_id;
{
  register  struct    lockq    *locks, *result;
  struct fidLocalHash	*htPtr;
  int		i;
  int     	done;
  
#ifdef LMTRACE
  printf ("-%d-Calling is_file_acquired with fileid (%d, %d) trans %d.\n",
	  trans->trans_id, file_id.Fvolid,file_id.Ffilenum,trans->trans_id);
#endif
  
  htPtr = trans->fidHashTbl;
  i = HASHLOCALFID(file_id);

  done = FALSE;
  result = NULL;
  for (locks = htPtr->fidChain[i]; (locks != NULL) && (done != TRUE); 
       locks = locks->collision)
    {
      if (locks->resptr->owner == FILER
	  && locks->resptr->owner_id.file_id.Fvolid == file_id.Fvolid
	  && locks->resptr->owner_id.file_id.Ffilenum == file_id.Ffilenum)
	{
          result = locks;
          done = TRUE; 
	}
    }
  return (result);
}


error_checking (trans_id, file_id, routine)
     int     trans_id;
     FID     file_id;
     short   *routine;
{
  struct    graph_bucket    *trans;
  
  if ((trans = find_trans (trans_id)) == NULL)
    {
      printf ("-%d-Trans is non existant in routine %s, cant lock.\n",
	      trans_id, routine);
      return;
    } 
  if (is_file_acquired (trans, file_id) == NULL)
    printf ("\n\t-%d- ERROR (proc error_checking), in routine %s, lock the FILE before accessing one of its pages.\n",trans_id, routine);
}


set_new_mode (trans, finger, mode)
     struct    graph_bucket    *trans;
     struct node *finger;
     LOCKTYPE mode;
{
  register  struct    lockq    *locks;
  
#ifdef LMTRACE
  printf ("-%d-calling set_new_mode with mode %d\n", trans->trans_id,mode);
#endif
  
  locks = trans -> locks_held; 
  while(locks != NULL) {
    if (locks ->resptr == finger)
      break;
    locks=locks->flink;
  }
  if(locks == NULL){
    printf("-%d-set_new_mode: Cannot find lock entry for xact. FATAL\n"
	   ,trans->trans_id);
    exit(1);
  }
  locks->request = mode;
}

struct    lockq    *is_page_acquired (trans, page_id)
     struct    graph_bucket    *trans;
     PID       page_id;
{
  register  struct    lockq    *locks, *result;
  struct pidLocalHash	*htPtr;
  int	         done;
  int		 i;
  
#ifdef LMTRACE
  printf ("-%d-calling is_page_acquried: page_id (%d,%d).\n",
	  trans->trans_id,page_id.Pvolid,page_id.Ppage);
#endif
  htPtr = trans->pidHashTbl;
  i = HASHLOCALPID(page_id);

  done = FALSE;
  result = NULL;
  for (locks = htPtr->pidChain[i]; (locks != NULL) && (done != TRUE); 
       locks = locks->collision)
    {
      if (locks ->resptr-> owner == PAGER
	  && locks ->resptr-> owner_id.page_id.Pvolid == page_id.Pvolid
	  && locks ->resptr-> owner_id.page_id.Ppage  == page_id.Ppage)
	{
	  result = locks;
	  done = TRUE; 
	}
    }
  
  return (result);
}




/*
 *  lock_file (trans_cell, file_id, mode)
 *  Given a file, it tries to acquire the desired lock mode on it. Fairness
 *  is granted by making a check on the queue of waiting transactions.  If any
 *  transaction is waiting for the file, even if the transactions request is
 *  compatible, it has to wait.  Before allowing a transaction to wait a check
 *  on deadlock is made.  If the waiting results in local deadlock then the
 *  request is turned down and the transaction is aborted.
 *  Note that if the requests are not compatible waiting results.
 */

lock_file (transId, file_id, mode, duration, cond)
     int transId;
     FID file_id;
     LOCKTYPE mode;
     DURATION duration;
     short cond;
{
  struct  		graph_bucket    *trans_cell;
  LOCKTYPE 		stronger;
  struct      		lockq        *lock;
  LOCKTYPE		oldmode, newmode;
  struct 		wait_node  *temp;
  int 			upgrade_wait;
  int 			tdbg;
  struct wait_node    	*waitCell;
  struct node    *finger;
  int         		starttime;
  short 		t_compat;
  int			fileIndex;	/* index of file in locktable */
  
  trans_cell = find_trans(transId);
  if (trans_cell == NULL) {
    printf 
      ("\nERROR (proc lock_file), Transaction cell is null, non existant.\n");
    return(lm_NULLTRANSCELL);
  }
  
  if (trans_cell -> res_wait != NULL) {
    printf
      ("\nERROR (proc lock_file), Transaction %d is supposed to be sleeping.\n",
       trans_cell->trans_id);
    return(lm_SLEEPING);
  }
  
#ifdef LMTRACE
  printf ("-%d- is requesting file (%d, %d) in mode ",
	  trans_cell->trans_id, file_id.Fvolid, file_id.Ffilenum);
  printlock(mode);
#endif LMTRACE

  if (trans_cell -> aborted == TRUE) {
    printf ("-%d-ERROR Aborted Transaction IGNORE.\n",trans_cell->trans_id);
    return(ABORTED);
  }
  /*
   * mode == NL? XXXXXX
   */
  if (mode == l_NL) 
    printf("-%d-lock_file: Lock made is NL\n",trans_cell->trans_id);
  
  /* This is for statistics which can be deleted */
  trans_cell -> no_filelocks ++; 
  fileIndex  = hash_file(file_id);

  /* If a lock node for the file exists, findfile() will leave
  a latch on the actual lock node.  Otherwise (ie. finger is NULL),
  a latch is left on the locktable entry itself */

  finger = findfile(file_id);
  if (finger == NULL)
  {
    CREATE_FILE_LOCKNODE_M(finger,trans_cell->trans_id,file_id,mode);

/*
    printf("lockfile after createfilenode finger=%x\n", finger);
*/
    InitLatch(&finger->concurrent);
    CREATE_FILE_LOCK_M(trans_cell,file_id,mode,finger);

    /* release latch as we are about to return */
    ReleaseLatch(&smPtr->locktable[fileIndex].concurrent, procNum);
  }
  else 
  {
    if ( (lock = is_file_acquired (trans_cell, file_id)) != NULL) 
    {
      stronger = LM_conv [lock->request][mode];
      if (stronger == ILLEGAL){
	
#ifdef LMTRACE
	printf ("-%d-lock_file: Illegal upgrade mode %d -> mode %d\n",
		trans_cell->trans_id, lock->request, mode);
#endif
	
        ReleaseLatch(&finger->concurrent, procNum);
	return(LM_ILLEGALCONV);
      }
      if (stronger == lock->request)
      {
	
#ifdef LMTRACE
	printf("-%d-lock_file: Already holding lock\n",
	       trans_cell->trans_id);
#endif LMTRACE
	
        ReleaseLatch(&finger->concurrent, procNum);
	return (GRANTED);
      }
      /*
       * Set mode to be the stronger mode
       */
      oldmode = lock->request;
      newmode = stronger;
    }
    
    /*
     * Perform Hierarchy Checks
     */
    if(lock){
      /*
       * Find out if upgrade has to wait
       *  ----------NOTE ---------------
       * The following code is optimized for the occurrence that it is 
       * sufficient to compare the mode of a new request with the
       * request held at the head at the queue. If this is not the
       * case in the particular lock modes being used, a comparison
       * has to be made with all held modes.
       * 
       */
      if (finger->o_list == NULL)
	upgrade_wait = FALSE;
      else if ((finger->no_locktypes == 1) && 
	       (LM_compat[newmode][oldmode] != ILLEGAL))
	upgrade_wait = FALSE;
      else if ((finger->no_locktypes == 1) &&
	       (LM_compat[newmode][oldmode] == ILLEGAL))
	upgrade_wait = TRUE;
      else if ((finger->trans_id != trans_cell-> trans_id) &&
	       (LM_compat[finger->serving][newmode] != ILLEGAL))
	upgrade_wait = FALSE;
      else if ((finger->trans_id != trans_cell-> trans_id) &&
	       (LM_compat[finger->serving][newmode] == ILLEGAL))
	upgrade_wait = TRUE;
      else if ((finger->trans_id == trans_cell->trans_id) &&
	       (LM_supr[oldmode][newmode] == oldmode))
	upgrade_wait = FALSE;
      else if ((finger->trans_id == trans_cell->trans_id) &&
	       (LM_supr[oldmode][newmode] == newmode)){
	/*
	 * Check compatibility with all the locktypes that are held 
	 * at this time on the resource
	 */
	upgrade_wait = FALSE;	
	for (temp=finger->o_list;temp!=NULL;temp= temp -> flink) 
	  if (LM_compat[temp -> serving][newmode]==ILLEGAL) {
	    upgrade_wait = TRUE;	
	    break;
	  }
      }
      else {
	printf("-%d-lock_file: Upgrading lock, unknown state, FATAL\n",
	       trans_cell->trans_id);
        ReleaseLatch(&finger->concurrent, procNum);
	trans_cell->aborted = TRUE;
	return(ABORTED);
      }
      
#ifdef LMTRACE
      printf("-%d-lock_file: upgrade_wait = %d\n",
	     trans_cell->trans_id, upgrade_wait);
#endif LMTRACE
      
    }
    /*
     *  ----------NOTE ---------------
     * The following code is optimized for the occurrence that it is 
     * sufficient to compare the mode of a new request with the
     * request held at the head at the queue. If this is not the
     * case in the particular lock modes being used, a comparison
     * has to be made with all held modes.
     */
    t_compat = LM_compat[mode][finger->serving];
    if ((!(lock) && ((finger->wait != NULL) || (t_compat == ILLEGAL))) 
	|| (lock && upgrade_wait))
    {
      switch (check_deadlock_u (trans_cell->trans_id, finger)) 
      {
	  case DEADLOCK :
	    if (cond)
	    {
                ReleaseLatch(&finger->concurrent, procNum);
	        return(COND_ABORTED);
	     }
	  
#ifdef LMTRACE
	    printf ("-%d-DEADLOCK occured with trans %d \n\n",
		  trans_cell->trans_id, finger -> trans_id);
#endif

	    trans_cell -> aborted = TRUE;
	    ReleaseLatch(&finger->concurrent, procNum); 
	    lm_abort_trans (trans_cell->trans_id); 
	    return (ABORTED);
	    break;
	case OK :
	    /* this is the case where the lock must really wait */
	    if (cond)
	    {
		/* lock request was conditional */
                ReleaseLatch(&finger->concurrent, procNum);
	        return(COND_WAIT);
	    }

	    if (lock)
	    {   
		/* Upgrade if lock exists already */
	        waitCell=addwaitbucket_u(finger,newmode,trans_cell->trans_id);
	        waitCell->upgrade = TRUE;
	    }
	    else 
	    {
	        waitCell = addwaitbucket (finger, mode, trans_cell->trans_id);
	        waitCell->upgrade = FALSE;
	    }
	  
	    waitCell->duration = duration;
	    trans_cell -> resource_wait = waitCell;
	    trans_cell -> res_wait = finger;
	  
#ifdef LMTRACE
	    printf ("-%d- Transaction is waiting for %d. 0x%x\n", 
		  trans_cell->trans_id, finger -> trans_id, &waitCell->monitor);
#endif LMTRACE
	  
	    /* The following stmt is for statistics gathering */
	    /* ------------> */
	    trans_cell -> no_blocks ++;

#ifdef LMTRACE
	    printf ("-%d- Transaction re-starting... \n",
		      trans_cell->trans_id);
#endif
	    /* cannot acquire the lock so we must block */
	    /* first release the latch on the page resource */
	    /* there is a race condition here as another transaction */
	    /* may unlock the resource before the WaitSem happens */
	    /* but the SendSem will remember and all will be ok */

	    waitCell->monitor = &smPtr->users[procNum].sem; 
		/* stick in addr of semaphore for the process */
		/* so that the process can be awakened later */

/*
    printf("lockfile, process %d about to go to sleep on its semaphore\n", 
		procNum);
*/

    	    ReleaseLatch(&finger->concurrent, procNum);
	    WaitSem (&waitCell->monitor->semid);
/*
    printf("lockfile, process %d wakes up from sleeping on its semaphore\n", 
	procNum);
*/
	    /* when the process wakes up, check to make sure it hasn't
	  	gotten blow away while asleep */
	    if (find_trans (trans_cell->trans_id) == NULL)
	    {
		  trans_cell -> res_wait = NULL;
		  return (ABORTED);
	    }
	    trans_cell->res_wait = NULL;
	    trans_cell->resource_wait = NULL;
	    break;

	case REACQUIRE :

#ifdef LMTRACE
	    printf("-%d-lock_file: deadlock ret REACQUIRE for upgrade\n", 
		trans_cell->trans_id);
#endif

	    break;
	default : 
/*
	    printf ("-%d-lock_file, check deadlock returned junk\n",
	    	trans_cell->trans_id);
*/
    	    ReleaseLatch(&finger->concurrent, procNum);
	    return (ABORTED); 
	}
	/* this leg of the if leaves no latches set */
    }
    else 
    {
      /* 
       * The lock can be granted. 
       */
      if (duration == INSTANT)
      {
    	  ReleaseLatch(&finger->concurrent, procNum);
	  return (GRANTED);
      }
      if(lock)
      {
	/*
	 * Change the request in both the transaction list of locks
	 * and the resource list of holders. Perform necessary
	 * adjustments to the grant Queue and store the strongest
	 * node at the head
	 */
	oldmode = finger->serving;
	lock->request = newmode;
	if(addowner_u(finger, newmode, trans_cell->trans_id) != OK){
	  printf ("-%d-lock_file: addowner_u failed. FATAL \n",
		  trans_cell->trans_id);
    	  ReleaseLatch(&finger->concurrent, procNum);
	  trans_cell->aborted = TRUE;
	  return(ABORTED);
	}
	if( (finger->wait != NULL) &&
	   (finger->serving != oldmode) &&
	   (LM_supr[finger->serving][oldmode] == oldmode))
	  walkfile(finger, finger->serving);
      }						
      else {
	CREATE_FILE_LOCK_M(trans_cell, file_id, mode,finger);
	ADDOWNER_M(finger, mode, trans_cell->trans_id);
      }
      ReleaseLatch(&finger->concurrent, procNum);
    }
  }
  return (GRANTED);
}


/*
 *           hierarchical locking: Ask for a lock on the page, and 
 *                ask for the corresponding lock on the file .
 *
 * Look first if there is a strong enough lock on the file belonging the
 * page, or on the page itself. 
 * If there is, the code GRANTED is returned immediatly.
 * If not, a lock on the file is asked. Three cases can happen:
 * 1.The lock on the file is GRANTED. 
 *   In this case we ask for the lock on the page.
 * 2.Asking for the lock on the file has ABORTED the transaction. Then  
 *   lock_page returns the code ABORTED.
 * 3.lock_file returns WAIT. In this case, the code WAIT_AND_REASK
 *   is returned. The transaction which asked for the lock has to wait 
 *   until the signal from the lockmanager to restart. MOREOVER, the
 *    transaction will have to restart by re-asking for the lock on the page. 
 *
 * Note: A page which is locked for the first time is linked to its file.
 */

lock_page (transId, file_id, page_id, mode, duration, cond)
     int transId;
     FID file_id;
     PID page_id;
     LOCKTYPE mode;
     DURATION duration;
     short cond;
{
  struct  graph_bucket    *trans_cell;
  struct  node            *finger, *file_finger;
  struct  wait_node	*waitCell;
  struct  lockq         *lock, *file_lock;
  LOCKTYPE		oldmode, newmode;
  struct  wait_node  	*temp;
  int 			upgrade_wait;
  LOCKTYPE 		stronger, tempmode;
  LOCKTYPE 		fileLockMode, newFileLockMode;
  int			needFileLock;
  int         		starttime;
  short 		t_compat;
  int			e;			/* returned error code */
  int			fileIndex;	/* index of file in locktable */
  int			pageIndex;	/* index of page in locktable */
  
  trans_cell = find_trans(transId);
  if (trans_cell == NULL){
    printf ("\nERROR (proc lock_page), trans_cell passed in is NULL.\n");
    printf ("\nERROR (proc lock_page), Activate trans before locking a page on it.\n");
    return(ABORTED);
  }
  
  if (trans_cell -> res_wait != NULL) {
    printf
      ("\nERROR (proc lock_page), Transaction %d is supposed to be sleeping.\n",
       trans_cell->trans_id);
    return(lm_SLEEPING);
  }
  
  if (mode == l_NL) 
    printf("lock_page: Lock made is NL\n");
  
#ifdef LMTRACE
  printf ("Transaction %d is requesting page (%d, %d) in mode ",trans_cell->trans_id, page_id.Pvolid, page_id.Ppage);
  printlock(mode);
#endif
lmPageLocks++;

  if (TESTPIDCLEAR(page_id)) {
    return(GRANTED);
  }
  
#ifdef LMTRACE
  if( !LM_leaftype[mode]){
    printf("lock_page: illegal mode asked for leaf type\n");
    return(LM_ILLEGALMODE);
  }
#endif
  
  if (trans_cell -> aborted == TRUE)
  {
    printf ("\nXACT %d was aborted but still running around, IGNORE.\n",
	trans_cell->trans_id);
    return(ABORTED);
  }
  
  /* This is for statistics and can be deleted */
  trans_cell->no_pagelocks++;

  /* begin by seeing if the transaction has a strong enough */
  /* lock on the file to avoid setting a page lock */
  if ((file_lock = is_file_acquired(trans_cell, file_id)) != NULL) 
  {
      fileLockMode = file_lock->request;
      switch(file_lock->request) {
      case l_X:
	return(GRANTED);
	break;
      case l_SIX:
      case l_S:
	if (mode == l_S) return(GRANTED);
	break;
      case l_IX:
      case l_IS:
	break;
      otherwise:
	printf("\nlock_page: wrong lock mode (mode = %d) \n", mode);
	return(LM_ILLEGALMODE);
      }
  }
  else
  {
    /* file_lock is null, since this version of the lock manager
      requires that a file lock be held before setting a page lock
      this is an error condition and we return 
    */

    printf("null file finger in lock page call with params\n");
    printf ("file (%d, %d), page (%d, %d), lock mode ", file_id.Fvolid, 
	file_id.Ffilenum, page_id.Pvolid, page_id.Ppage);
    printlock(mode);
    printf("\n");
    trans_cell->aborted = TRUE;
    return(ABORTED);
  }

  /* 
     If we didn't return we need to get a page lock as the file lock
     was not strong enough to avoid setting a page lock.  Begin
     by checking whether we already have a strong enough lock on the page
  */

  lock = NULL;
  if ((lock = is_page_acquired(trans_cell, page_id)) != NULL) 
  {
	/*  Transaction already has a lock on the page.  */
	/*  See if it is strong enough */

        stronger = LM_conv[lock->request][mode];
        if (stronger == ILLEGAL)
        {
	     printf ("\nlock_page: Illegal upgrade mode %d->mode %d\n",
			lock->request, mode);
	     return(LM_ILLEGALCONV);
        }
        if (stronger == lock->request) 
	{
	
#ifdef LMTRACE
		printf("\nlock_page: Already holding lock\n");
#endif
		return (GRANTED);
      	}
        /*
           * Set mode to be the stronger mode
         */
         oldmode = lock->request;
         newmode = stronger;
  }

   /* see if a strong enough lock exists on the file */
   switch(mode) 
   {
      case l_X: 
	if ((fileLockMode == l_IX) || (fileLockMode == l_SIX))
		needFileLock = FALSE;
	else {
	    needFileLock = TRUE;
	    newFileLockMode = l_IX;
	}
	break;
      case l_S: 
	if (fileLockMode == l_IS) needFileLock = FALSE;
	else {
	    needFileLock = TRUE;
	    newFileLockMode = l_IS;
	}
	break;
      default:
        printf ("\nlock_page: Illegal mode (mode = %d)\n", mode);
        return(LM_ILLEGALMODE);
  }
	
  if (needFileLock)
  {
      e = lock_file(transId, file_id, newFileLockMode, duration, cond);
      if (e != GRANTED) return(e); /* e == ABORTED or COND_WAIT or any error*/
  }

  /*
   *  So at this point we have a strong enough lock on file but either
   *  no lock on the page or one that is not sufficiently strong 
   *  We begin by setting the necessary latches 
   */

  /* 
   * Get a pointer to the file lock node itself.  If a lock node 
   * for the file exists, findfile() will leave a latch on the 
   * actual lock node.  Otherwise (ie. file_finger is NULL),
   * a latch is left on the locktable entry itself 
   */

  file_finger = findfile(file_id);
  if (file_finger == NULL) 
  {
	/* this should never happen since we checked above 
	that the transaction already had a lock on the file
	but we just double check to be sure */

	printf("null file finger in lock page call with params\n");
  	printf ("file (%d, %d), page (%d, %d), lock mode ",
		file_id.Fvolid, file_id.Ffilenum,
		page_id.Pvolid, page_id.Ppage);
  		printlock(mode);
	printf("\n");
        fileIndex = hash_file(file_id);
        ReleaseLatch(&smPtr->locktable[fileIndex].concurrent, procNum);
	trans_cell->aborted = TRUE;
    	return(ABORTED);
  }

  /* If a lock node for the page already exists, findpage() will leave
     a latch on the actual lock node.  Otherwise (ie. finger is NULL),
     a latch is left on the locktable entry itself 
  */
  pageIndex = hash_page(page_id);
  finger = findpage(page_id);
  if (finger == NULL) 
  {   
      /* There are no locks at all on this page */
      /* Create a "node" structure for the lock on this page */
      CREATE_PAGE_LOCKNODE_M(finger,trans_cell->trans_id, file_finger,page_id,mode);
      InitLatch(&finger->concurrent);

      /* next get a free "lockq" structure for the locking which gets */
      /* hung off the graphbucket structure associated with the transactions */
      /* and which points to the lock node acquired above */
      /* This call also inserts a pointer from the transaction's */
      /* pidHashTbl to the lockq structure so that future lock attempts */
      /* will happen fast */

      CREATE_PAGE_LOCK_M(trans_cell,page_id,mode,finger);

      /* release the two lock table latches as we are about to return */
      ReleaseLatch(&smPtr->locktable[pageIndex].concurrent, procNum);
      ReleaseLatch(&file_finger->concurrent, procNum);
  }
  else 
  {
      /* at this point we should be holding latches on the file and 
      page nodes */

      if (lock)
      {
          /*
          * Find out if upgrade has to wait
          */
      
         /*
          *  ----------NOTE ---------------
          * The following code is optimized for the occurrence that it is 
          * sufficient to compare the mode of a new request with the
          * request held at the head at the queue. If this is not the
          * case in the particular lock modes being used, a comparison
          * has to be made with all held modes.
          */
         if (finger->o_list == NULL) upgrade_wait = FALSE;
         else 
	    if ((finger->no_locktypes == 1) && 
	      (LM_compat[newmode][oldmode] != ILLEGAL)) 
			upgrade_wait = FALSE;
         else if ((finger->no_locktypes == 1) &&
	       (LM_compat[newmode][oldmode] == ILLEGAL)) 
			upgrade_wait = TRUE;
         else if ((finger->trans_id != trans_cell-> trans_id) &&
	       (LM_compat[finger->serving][newmode] != ILLEGAL))
	 		upgrade_wait = FALSE;
         else if ((finger->trans_id != trans_cell-> trans_id) &&
	       (LM_compat[finger->serving][newmode] == ILLEGAL))
			upgrade_wait = TRUE;
         else if ((finger->trans_id == trans_cell->trans_id) &&
	       (LM_supr[oldmode][newmode] == oldmode)) 
			upgrade_wait = FALSE;
         else if ((finger->trans_id == trans_cell->trans_id) &&
	       (LM_supr[oldmode][newmode] == newmode))
	 {
	    /*
	     * Check compatibility with all the locktypes that are held 
	     * at this time on the resource
	     */
	    upgrade_wait = FALSE;	
	    for (temp=finger->o_list;temp!=NULL;temp= temp -> flink) 
	    if (LM_compat[temp -> serving][newmode]==ILLEGAL) 
	    {
	        upgrade_wait = TRUE;	
	        break;
	    }
         }
         else 
	 {
	    printf("\nlock_file: Upgrading lock, unknown state, FATAL\n");
	    ReleaseLatch(&file_finger->concurrent, procNum);
    	    ReleaseLatch(&finger->concurrent, procNum);
	    trans_cell->aborted = TRUE;
	    return(ABORTED);
         }
      
#ifdef LMTRACE
      printf("\nlock_file: upgrade_wait = %d\n",upgrade_wait);
#endif
      
     }

    /*
     *  ----------NOTE ---------------
     * The following code is optimized for the occurrence that it is 
     * sufficient to compare the mode of a new request with the
     * request held at the head at the queue. If this is not the
     * case in the particular lock modes being used, a comparison
     * has to be made with all held modes.
     */
    t_compat = LM_compat[mode][finger->serving];
    if ((!(lock) && ((finger -> wait != NULL) || (t_compat == ILLEGAL))) ||
	(lock && upgrade_wait))
    {
      /* 
       * the lock cannot be granted immediately. put the acquierer 
       * in the wait-list if it doesn't create a deadlock 
       */
      switch (check_deadlock_u (trans_cell->trans_id, finger)) {
	case OK :
	  /* this is really the case the transaction has to wait!! */

	  if (cond)
	  {
	      ReleaseLatch(&file_finger->concurrent, procNum);
    	      ReleaseLatch(&finger->concurrent, procNum);
	      return(COND_WAIT);
	  }
	  
	  if (lock)
	  {   /* Upgrade if lock exists already */
	      waitCell=addwaitbucket_u(finger,newmode, trans_cell->trans_id);
	      waitCell->upgrade = TRUE;
	  }
	  else 
	  {
	      waitCell=addwaitbucket(finger,mode,trans_cell->trans_id);
	      waitCell->upgrade = FALSE;
	  }
	  waitCell->duration = duration;
	  trans_cell -> resource_wait = waitCell;
	  trans_cell -> res_wait = finger;
	  
	  /* The following stmt is for statistics gathering */
	  trans_cell -> no_blocks ++;
	  
#ifdef LMTRACE
	  printf ( "(!=NULL) %d is waiting for %d.\n",
		  trans_cell->trans_id, finger->trans_id);
#endif

	  /* cannot acquire the lock so we must block */
	  /* first release the latch on the page resource */
	  /* there is a race condition here as another transaction */
	  /* may unlock the resource before the WaitSem happens */
	  /* but the SendSem will remember and all will be ok */

	  waitCell->monitor = &smPtr->users[procNum].sem; 
		/* stick in addr of semaphore for the process */
		/* so that the process can be awakened later */

/*
	printf("lockpage, process %d about to go to sleep on its semaphore\n", 
	procNum);
*/

	  ReleaseLatch(&file_finger->concurrent, procNum);
    	  ReleaseLatch(&finger->concurrent, procNum);
	  WaitSem (&waitCell->monitor->semid);

#ifdef LMTRACE
	    printf ("-%d- Transaction re-starting... \n",
		      trans_cell->trans_id);
#endif
	  /* when the process wakes up, check to make sure it hasn't
	  gotten blow away while asleep */
	  if (find_trans (trans_cell->trans_id) == NULL){
		  trans_cell -> res_wait = NULL;
		  return (ABORTED);
	  }
	  trans_cell -> res_wait = NULL;
	  trans_cell -> resource_wait = NULL;
	  break;
	  
	case DEADLOCK :
	    if (cond)
	    {
    	        ReleaseLatch(&finger->concurrent, procNum);
		ReleaseLatch(&file_finger->concurrent, procNum);
	        return(COND_ABORTED);
	    }
	  
#ifdef LMTRACE
	  printf ("\n\tDEADLOCK occured between trans %d and %d.\n",
		  finger -> trans_id, trans_cell->trans_id);
#endif LMTRACE
	    trans_cell -> aborted = TRUE;
    	    ReleaseLatch(&finger->concurrent, procNum);
	    ReleaseLatch(&file_finger->concurrent, procNum);
	    lm_abort_trans (trans_cell->trans_id);
	    return (ABORTED);
	    break;
	  
	case REACQUIRE :
    	    ReleaseLatch(&finger->concurrent, procNum);
	    ReleaseLatch(&file_finger->concurrent, procNum);
	    return (GRANTED);
	    break;
	  
	default : 
	    printf ("\nERROR (proc lock_page) check deadlock returned junk.\n");
	    trans_cell -> aborted = TRUE;
    	    ReleaseLatch(&finger->concurrent, procNum);
	    ReleaseLatch(&file_finger->concurrent, procNum);
	    return (ABORTED);
	    break; 

	}
	/* this branch of the if statement terminates with 
	   no latches set */
    }
    else {
      if (duration == INSTANT)
      {
    	    ReleaseLatch(&finger->concurrent, procNum);
	    ReleaseLatch(&file_finger->concurrent, procNum);
	    return(GRANTED);
      }
      /*
       * If upgrades, do not get a new lock
       */
      
      if(lock){
	/*
	 * Change the request in both the transaction list of locks
	 * and the resource list of holders. Perform necessary
	 * adjustments to the grant Queue and store the strongest
	 * node at the head
	 */
	oldmode = finger->serving;
	lock->request = newmode;
	if(addowner_u(finger, newmode, trans_cell->trans_id) != OK)
	{
	      printf ("\nlock_page: addowner_u failed. FATAL \n");
    	      ReleaseLatch(&finger->concurrent, procNum);
	      ReleaseLatch(&file_finger->concurrent, procNum);
	      trans_cell -> aborted = TRUE;
	      return (ABORTED);
	}
	if( (finger->wait != NULL) &&
	   (finger->serving != oldmode) &&
	   (LM_supr[finger->serving][oldmode] == oldmode))
	  walkfile(finger, finger->serving);
      }						
      else {
	CREATE_PAGE_LOCK_M(trans_cell, page_id, mode,finger);
	ADDOWNER_M(finger, mode, trans_cell->trans_id);
      }
      ReleaseLatch(&finger->concurrent, procNum);
      ReleaseLatch(&file_finger->concurrent, procNum);
      /* this branch of the if also leaves no latches set */
    }
  }
  return (GRANTED);
}

/* routine to check whether there are any transactions waiting
to acquire a lock on a page.  Returns TRUE if a transaction is waiting,
otherwise, it returns FALSE */

check_for_waiters(page_id)
     PID page_id;
{
  register  struct  node            *finger;

  finger = findpage(page_id);
  if (finger != NULL)
  {
      if (finger->wait == NULL) return(FALSE);
      else return(TRUE);
  }
  else 
  {
    printf("\nError!!  check_for_waiters,  NO lock on page %d.%d\n",
	page_id.Pvolid, page_id.Ppage);
    return (TRUE);  /* seems safer than false */
  }
}

