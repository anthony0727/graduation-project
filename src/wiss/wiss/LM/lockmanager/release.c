
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

extern    struct node         *findfile(), *findpage();
extern    struct graph_bucket *find_trans ();
extern	  struct lockq	      *is_page_acquired();
extern	  struct lockq	      *is_file_acquired();


extern SMDESC   *smPtr; /* ptr to common data structures in shared memory */
extern  int     procNum; /* index of the process in the smPtr->users table */

LOCKTYPE lock_strength (lockone, locktwo)
     LOCKTYPE lockone, locktwo;
{
  
    return(LM_conv[lockone][locktwo]);
}


/*
 *  walkfile (mainnode, mode)
 *  As soon as a resource is released the list of waiting transactions
 *  is scanned and the waiting transactions waiting at the head of
 *  resource which are compatible are allowed in.  The walk is in FIFO
 *  order and stopped as soon as incompatibly is noticed or the list
 *  is exhausted.
 */
walkfile (mainnode, mode)
     struct node          *mainnode;
     LOCKTYPE mode;
{
  struct    wait_node    *temp, *perm;
  struct    graph_bucket *transone;
  int upgrade_wait;
  struct    wait_node    *tlist;
  short t_compat;
  struct graph_bucket *waitingTrans;
  
  mainnode -> serving = mode;
  
#ifdef LMTRACE
  printf ("Walking on the resource with trans_id %d\n",mainnode -> trans_id);
  if (mainnode -> wait == NULL)
    printf ("mainnode -> wait is null.\n");
#endif
  
  for (temp = mainnode -> wait; (temp != NULL); )
  {
      if ( (transone = find_trans(temp -> trans_id)) != NULL && 
	  transone -> aborted == FALSE ) {
	/*
	 * If this is an UPGRADE, walk thru the xact resource list and
	 * change the request type. Call addowner_u to properly
	 * change the resource entry for the xact and also set the 
	 * strongest mode at the head of the queue XXXXXXXXX
	 */ 
	
	if (temp->upgrade)
	{
	  /*
	   * Find out if upgrade can be done
	   */
	  if (mainnode->o_list == NULL) upgrade_wait = FALSE;
	  else 
	  if (mainnode->trans_id != temp->trans_id) 
	  {
	     if (LM_compat[mainnode->serving][temp->serving] != ILLEGAL)
	     {
	    	upgrade_wait = FALSE;
	     }
	     else
	     {
		 upgrade_wait = TRUE;
		 break;
	     }
	   } /*!=trans_id*/
	   else if (mainnode->trans_id == temp->trans_id)
	   {
	      upgrade_wait = FALSE;
	      for (tlist=mainnode->o_list;tlist!=NULL;tlist= tlist -> flink)
	      if (LM_compat[tlist -> serving][temp->serving]==ILLEGAL) 
	      {
		    upgrade_wait = TRUE;
		    break;
	      }
	      if(upgrade_wait) break;
	    }
	    else
	    {
	    	printf("\nwalkfile: Illegal state for upgrade FATAL\n");
	    	exit(1);
	    }
	  
	    /*
	     * If an instant duration is asked for, the wait node
	     * is released and the process is started up without recording 
	     * its state in the lock manager data structures
	     * The use of the goto infact just helps skipping to the end of the
	     * if then else block
	     */
	    if(temp->duration == INSTANT) goto freew_node;
	  
	    if(addowner_u(mainnode, temp->serving, temp->trans_id) != OK){
	        printf ("\nlock_page: addowner_u failed. FATAL \n");
	        exit(1);
	    }
	    set_new_mode(transone, mainnode, temp->serving);
	}
	else { /* not temp->upgrade */
	  /*
	   * ---------------NOTE-----------
	   * This code may not work for all lock types
	   */
	  t_compat = LM_compat[mainnode->serving][temp->serving];
	  if (t_compat)
	    break;
	  /*
	   * If an instant duration is asked for, the wait node
	   * is released and the process is started up without recording 
	   * its state in the lock manager data structures
	   * The use of the goto infact just helps skipping to the end of the
	   * if then else block
	   */
	  if(temp->duration == INSTANT) goto freew_node;
	  
	  ADDOWNER_M(mainnode,temp->serving,temp->trans_id);
	  
#ifdef LMTRACE
	  if (mainnode -> trans_id != temp -> trans_id) printf ("ERROR, in walkfile.\n");
#endif
	  
	  if (mainnode -> owner == FILER) {
	    /* maybe unsafe, need a latch on locktable[ix] entry?? - dewitt */
	    CREATE_FILE_LOCK_M(transone, mainnode->owner_id.file_id, 
		temp->serving, mainnode);
	  }
	  else 
	    /* maybe unsafe, needs a latch on locktable[ix] entry?? - dewitt */
	    CREATE_PAGE_LOCK_M(transone, mainnode -> owner_id.page_id, 
			       temp -> serving,mainnode);
	}
#ifdef LMTRACE
	printf ("Walking xact %d, 0x%x\n", transone->trans_id, &temp->monitor);
#endif
	
      freew_node:
	transone -> waiting_for = NULL;
	transone -> res_wait = NULL;
	transone -> resource_wait = NULL;
/*
	printf("\n-->(walkfile) transaction %d can restart!\n", 
		temp->trans_id);
*/
	SendSem (&temp->monitor->semid);
	mainnode -> wait = temp -> flink;
	transone -> waiting_for = NULL;
      } /* end if (... && transone -> aborted == FALSE ) */

      if (transone == NULL) {
/*
	printf("\n-->(walkfile) transaction %d can restart!\n", 
		temp->trans_id);
*/
	SendSem (&temp->monitor->semid);
      }
      perm = temp;
      if (temp -> flink != NULL)
	temp -> flink -> blink = temp -> blink;
      temp = temp -> flink;
      if (perm == mainnode->wait) 
	mainnode->wait = temp;
      if (mainnode -> wait == perm) 
	{
	  printf ("\nERROR, the transaction %d was aborted.\n",temp->trans_id);
	  mainnode -> wait = temp;
	}
/* below NEW DeWitt 11/91*/
      /* The following test was added to handle the case of the */
      /* abort of a transaction that is waiting for a lock upgrade */
      if (transone->resource_wait == perm) transone->resource_wait = NULL;
/* above NEW DeWitt 11/91*/

      FREE_WAIT_NODE_M(perm);

    } /* end for */
  
  /* Check global deadlock as soon as you give access to somebody else. */
  for (temp = mainnode -> wait; temp != NULL; temp = perm)
    {
      
#ifdef LMTRACE
      printf ("=====>>Inside the for loop walkfile (%d,%d) .\n",
	      mainnode->trans_id,temp->trans_id);
#endif
      
      if (check_deadlock_u (temp -> trans_id, mainnode ) == DEADLOCK)
	{
	  
#ifdef LMTRACE
	  printf ("Inside Walkfile DEADLOCK OCCURED between trans %d, %d.  \n",mainnode -> trans_id, temp -> trans_id);
#endif
	  
	  transone = find_trans (temp -> trans_id);
	  transone -> aborted = TRUE;
	  abort_trans (temp->trans_id);
	  SendSem (&temp->monitor->semid);

	  /* Clean up the cell containing the node.*/
	  perm = temp->flink;
	  if (temp -> flink != NULL)
	    temp -> flink -> blink = temp -> blink;
	  if (temp -> blink != NULL)
	    temp -> blink -> flink = temp -> flink;
	  if (mainnode -> wait == temp)
	    mainnode -> wait = perm;
	  FREE_WAIT_NODE_M(temp);

	}
      else perm = temp -> flink;
    }
} /* end of walkfile */


LOCKTYPE release_owner_node (bucket, trans_id)
     struct   node    *bucket;
     int    trans_id;
{
  struct    wait_node    *temp;
  LOCKTYPE translockmode;
  LOCKTYPE strongest;
  struct      wait_node              *mainone;
  int         loc_trans;
  
  if (bucket -> trans_id == trans_id)
    {
      translockmode = bucket -> serving;
      if (bucket -> o_list != NULL)
	{
	  bucket -> trans_id = bucket -> o_list -> trans_id;
	  bucket -> serving = bucket -> o_list -> serving;
	}
      else
	bucket -> trans_id = -1;
      temp = bucket -> o_list;
    }
  else 
    {
      for (temp = bucket -> o_list; (temp != NULL && (temp -> trans_id != trans_id)); temp = temp -> flink);
      if (temp == NULL)
	{
	  printf ("\n ERROR, EXTREMELY DANGEROUS, unknown owner %d to release.\n",trans_id);
	  printf ("The owners of the resource are : ");
	  printf ("The head of the owners is %d\n",bucket->trans_id);
	  for (temp = bucket -> o_list; (temp != NULL); temp=temp->flink) 
	    printf ("%d ",temp->trans_id);
	  exit(1);
	}
      translockmode = temp -> serving;
    }
  if (temp != NULL)
    {
      if (bucket -> o_list == temp)
	bucket -> o_list = temp -> flink;
      if (temp -> flink != NULL)
	temp -> flink -> blink = temp -> blink;
      if (temp -> blink != NULL)
	temp -> blink -> flink = temp -> flink;
      FREE_WAIT_NODE_M(temp);
    }
  if (bucket -> o_list == NULL)
    return (translockmode);
  
  if (bucket -> no_locktypes > 1)
    {
      mainone = NULL;
      strongest = bucket -> serving;
      for (temp = bucket -> o_list; temp != NULL; temp = temp -> flink)
	{
	  if (LM_supr[temp -> serving][strongest] != strongest)
	    {
	      strongest = temp -> serving;
	      mainone = temp;
	    }
	}
      if (mainone != NULL)
	{
	  loc_trans = mainone->trans_id;
	  mainone -> serving = bucket -> serving;
	  mainone -> trans_id = bucket -> trans_id;
	  bucket -> trans_id = loc_trans;
	  bucket -> serving = strongest;
	}
    }
  return (translockmode);
}


/*
 *  lm_committ_trans (trans_id)
 *  Given a transaction it releases all the resources it acquired during trans
 *  life (notion of two phase locking).  The transaction is terminated so
 *  that there would not be any entry for it in the hash table. It makes
 *  sure that no resources are allocated to it by locking the transaction 
 *  node itself.
 */
lm_committ_trans (trans_id)
     int    trans_id;
{
 struct  lockq    *lockqTemp;
  struct    lockq            *perm;
  struct    graph_bucket     *node;
  struct wait_node *waitp;
  struct  node     *mresptr;  /* don't make this a register variable!!! */
		/* as it seems to induce a compiler bug on the mips 3000 cpu */
		/* under ultrix (5 days wasted here) */
  int	    lockTableIndex;
  FID	fid;
  
  node = find_trans (trans_id);
  
#ifdef LMTRACE
  printf ("\t COMMITTING TRANSACTION with trans_id = %d.\n",trans_id);
#endif
  
  if (node == NULL)
  {
      printf ("\nERROR, Transaction %d is not active and can't be commited.\n",trans_id);
      return ;
  }
  
#ifdef LMTRACE
  /*
   ** The following lines prinout what resource the node is holding
   ** before they are really released.
   */
/*
    printf ("node contains:\n");
    for (lockqTemp = node -> locks_held; lockqTemp != NULL; lockqTemp = lockqTemp -> flink)
    {
    	switch(lockqTemp ->resptr-> owner)
    	{
    	    case (PAGER) : printf ("\tPAGE (%d, %d), ",
			lockqTemp->resptr->owner_id.page_id.Pvolid, 
			lockqTemp->resptr->owner_id.page_id.Ppage);
    	    break;
    	    case (FILER) : printf ("\tFILE (%d, %d), ",
			lockqTemp->resptr->owner_id.file_id.Fvolid, 
			lockqTemp->resptr->owner_id.file_id.Ffilenum);
    	    break;
    	}
    }
    printf ("\n");
*/
#endif
  
  for (lockqTemp = node->locks_held; lockqTemp != NULL; )
  {
      mresptr = lockqTemp -> resptr;
      if (mresptr-> owner == FILER)
      {
	  fid = lockqTemp->resptr->owner_id.file_id;
/*
	  printf("in commit fid =%d.%d\n",fid.Fvolid, fid.Ffilenum);
*/
		
	  lockTableIndex = hash_file(fid);
	  /* get latch on proper hash table entry so no one else
	  can modify the chain of locks hanging from the entry */

	  SetLatch(&smPtr->locktable[lockTableIndex].concurrent, procNum, NULL);
	  /* then set latch on the actual lock node */
          SetLatch (&mresptr->concurrent, procNum, NULL);
	  RELEASE_FILE_LOCK_M(node,lockqTemp->resptr->owner_id.file_id,mresptr);
	  ReleaseLatch(&smPtr->locktable[lockTableIndex].concurrent, procNum);
	  /* the macro release_file_lock will release the latch on */
	  /* on the lock itself */

	  /* NOTE, in this case we do not bother to removed each file
	  from the transaction's private file hash table since we 
	  toss the entire hash table at the end of the transaction */
      }
      else 
      {
	  if (mresptr-> owner == PAGER)
	  {
	      lockTableIndex = hash_page(lockqTemp->resptr->owner_id.page_id);
	      /* get latch on proper hash table entry so no one else
	      can modify the chain of locks hanging from the entry */
	      SetLatch(&smPtr->locktable[lockTableIndex].concurrent, procNum, NULL);
	      /* then set latch on the actual lock node */
              SetLatch (&mresptr->concurrent, procNum, NULL);
	      RELEASE_PAGE_LOCK_M(node,lockqTemp->resptr->owner_id.page_id,mresptr);
	      ReleaseLatch(&smPtr->locktable[lockTableIndex].concurrent, procNum);
	      /* the macro release_file_lock will release the latch on */
	      /* on the lock itself */

	      /* NOTE, in this case we do not bother to removed each page
	      from the transaction's private page hash table since we 
	      toss the entire hash table at the end of the transaction */
	  }
	  else {
	    printf ("Illegal lock mode in lm_committ_trans = %d\n",
		(int) mresptr->owner);
	    LM_dumpevent();
	  }
      }
      perm = lockqTemp;
      lockqTemp = lockqTemp -> flink;
/* a meta comment to anyone whoever looks at this code again (having spent
5 days looking for a bug here.  The following call does not actually
unlink the lockq node from the chain of locks hanging from node->locks_held
until terminate (node) is called below.  The call puts locks back on the
freelock list but leaves them hanging from the locks_held chain.  Since
other concurrently executing processes might be grabbing lockq nodes
simultaneously, the value of locks_held is garbage immediately after
the first call to FREE_LOCK_NODE below.  
*/
      FREE_LOCK_NODE_M(perm);
  }
  
  if (node -> resource_wait != NULL)
  {
     /*
      *  If we get here the transaction is actually waiting on a resource
      *  and thus we are probably in the middle of an abort_trans call.
      *  That is, somehow a transaction that should be waiting has actually
      *  called abort_trans() - the user probably hit a control-C
      *
      */
      SendSem (&node->resource_wait->monitor->semid); /* wake up the waiter */

      /* remove the wait-node from the wait list */
      waitp = node->resource_wait;
      if (waitp->flink != NULL) waitp->flink->blink = waitp->blink;
      if (node->res_wait->wait == waitp)
      {
	node->res_wait->wait = node->res_wait->wait->flink;
/* NEW dewitt 11/91 */
	/* now it may just be the case that since the transaction that */
	/* is being aborted is at the front of the wait list */
	/* that there are other transactions waiting which are actually */
	/* compatible with the current lock mode.  call walkfile() to */
	/* see if any can be woken up */
	mresptr = node->res_wait;
	walkfile(mresptr, mresptr->serving);
/* NEW dewitt 11/91 */
      }
      else
      {
	   /* if test added by dewitt 10/14/91 */
	   /* in response to O2 bug report */
	   if (waitp->blink) waitp->blink->flink = waitp->flink;
      }

      FREE_WAIT_NODE_M(waitp);
      node->res_wait = NULL;
      node->resource_wait = NULL;
  }
  
  /*  THESE SET OF STMTS ARE FOR PERFORMANCE PERPUSES */
  /*  ----------------------------------------------- */
#ifdef LMTRACE
  /*printf ("\t\t----->  transid %d  <-----\n",trans_id);
    printf ("\t\t(%d) no page locks %d.\n",trans_id,node->no_pagelocks);
    printf ("\t\t(%d) no file locks %d.\n",trans_id,node->no_filelocks);
    printf ("\t\t(%d) no page releases %d.\n",trans_id,node->no_pagereleases);
    printf ("\t\t(%d) no file releases %d.\n",trans_id,node->no_filereleases);
    printf ("\t\t(%d) no blocks %d.\n",trans_id,node->no_blocks);
    printf ("\t\t(%d) no upgrades %d.\n",trans_id,node->no_upgrades);*/
#endif
  /*  ----------------------------------------------- */
  
  /*  ----------------------------------------------- */
  /*  SET the GLOBAL STATISTICS                       */
  smPtr->no_blocks += node->no_blocks;
  /*  ----------------------------------------------- */
  
/*
    printf ("\n\t\t\t===> GLOBAL STATS <===\n");
    printf ("\t\t total lock requests %d.\n",smPtr->locks_req);
    printf ("\t\t total no blocks %d.\n",smPtr->no_blocks);
    printf ("\t\t total no deadlocks %d.\n",smPtr->no_deadlocks);
*/
  
  terminate (node);
  return(OK);
}


/*
 *  lm_abort_trans (trans_id)
 *  Given a transaction it is aborted by releasing all the resources it
 *  acquired and taking it out of the wait queues of any resource it 
 *  might be waiting for.
 *  The jobs is accomplished by calling lm_committ_trans to release all the
 *  locks of the resource and then directly pulling the wait node corresponding
 *  to the transaction out of the awaited resource.
 */
lm_abort_trans (trans_id)
     int    trans_id;
{
  struct    node         *temp;
  struct    wait_node     *perm;
  struct    graph_bucket  *transaction;
  
  transaction = find_trans (trans_id);
  if (transaction == NULL)
    {
      printf ("ERROR, Transaction %d does not exist to terminate.\n",trans_id);
      return ;
    }
  
#ifdef LMTRACE
  printf ("\t------------->  aborting transid %d  <-------------\n",trans_id);
#endif
  
  transaction -> aborted = TRUE;
  
  /* --- Cancel out the affect of commit trans --- */
  smPtr->no_blocks -= transaction -> no_blocks;
  /* ------------------------------------- */
  
  lm_committ_trans (trans_id);
}

/*
 *  m_release_page(trans_cell, page_id);
 *  This routine releases the manual lock held by this transaction on the page
 */
m_release_page(transId, page_id)
     int transId;
     PID page_id;
{
  struct  lockq    *lockqTemp, *next;
  struct graph_bucket *trans_cell;
  struct  node     *mresptr;
  int		lockTableIndex;

/*
  printf("m_release called. transid=%d, page=%d.%d\n",transId, 
	page_id.Pvolid, page_id.Ppage);
*/

  trans_cell = find_trans(transId);
  if (trans_cell == NULL)
    {
      printf ("\nERROR, Transaction is not active and can't be commited.\n");
      return (lm_NULLTRANSCELL);
    }
  
  if (trans_cell -> res_wait != NULL) {
    printf
      ("\nERROR (proc lock_file), Transaction %d is supposed to be sleeping.\n",
       trans_cell->trans_id);
    return(lm_SLEEPING);
  }

  /* get a pointer to the lockq node for the page */
  lockqTemp = is_page_acquired(trans_cell, page_id);
  if (lockqTemp != NULL)
  {
      mresptr = lockqTemp -> resptr;
      next = lockqTemp->flink;

     /* get latch on proper hash table entry so no one else
      can modify the chain of locks hanging from the entry */

     lockTableIndex = hash_page(mresptr->owner_id.page_id);
     SetLatch(&smPtr->locktable[lockTableIndex].concurrent, procNum, NULL);

     /* then set latch on the actual lock node */
     SetLatch (&mresptr->concurrent, procNum, NULL);

     /* delete the page from the transaction's private hash table */
     pidHashDelete (page_id, trans_cell->pidHashTbl, lockqTemp);

     RELEASE_PAGE_LOCK_M(trans_cell, mresptr->owner_id.page_id, mresptr);
     ReleaseLatch(&smPtr->locktable[lockTableIndex].concurrent, procNum);
     /* the macro release_file_lock will release the latch on */
	      /* on the lock itself */

     if (trans_cell->locks_held == lockqTemp) trans_cell->locks_held = next;
     FREE_LOCK_NODE_M(lockqTemp);
  }
  return(OK);
}

/*
 *  m_release_file(trans_cell, file_id);
 *  This routine releases the manual lock held by this transaction on the file
 */
m_release_file(transId, file_id)
     int transId;
     FID file_id;
{
  struct graph_bucket *trans_cell;
  struct  lockq    *lockqTemp, *next;
  struct  node     *mresptr;
  struct  node     *page;
  int		lockTableIndex;

  trans_cell = find_trans(transId);
  if (trans_cell == NULL)
    {
      printf ("ERROR, Transaction is not active and can't be commited.\n");
      return (lm_NULLTRANSCELL) ;
    }
  
  if (trans_cell -> res_wait != NULL) {
    printf
      ("ERROR (proc lock_file), Transaction %d is supposed to be sleeping.\n",
       trans_cell->trans_id);
    return(lm_SLEEPING);
  }

  /* get a pointer to the lockq node for the file */
  lockqTemp = is_file_acquired(trans_cell, file_id);

  if (lockqTemp != NULL)
  {
      mresptr = lockqTemp -> resptr;
      next = lockqTemp->flink;

      /* first release the pages belonging to this file */
      for (page = mresptr->f_page; page != NULL; page = page->f_page)
            m_release_page(transId, page->owner_id.page_id);

      /* next get latch on proper hash table entry so no one else
         can modify the chain of locks hanging from the entry */
      lockTableIndex = hash_file(mresptr->owner_id.file_id);
      SetLatch(&smPtr->locktable[lockTableIndex].concurrent, procNum, NULL);

     /* delete the file from the transaction's private hash table */
     fidHashDelete (file_id, trans_cell->fidHashTbl, lockqTemp);

      /* delete the actual file lock, releasing the latch at the same time */
      RELEASE_FILE_LOCK_M(trans_cell,lockqTemp->resptr->owner_id.file_id,mresptr);
      ReleaseLatch(&smPtr->locktable[lockTableIndex].concurrent, procNum);
      if (trans_cell->locks_held == lockqTemp) trans_cell->locks_held = next;
      FREE_LOCK_NODE_M(lockqTemp);
  }
  return(OK);
}

