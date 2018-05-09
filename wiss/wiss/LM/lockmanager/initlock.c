
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

extern SMDESC   *smPtr; /* ptr to common data structures in shared memory */
extern  int     procNum; /* index of the process in the smPtr->users table */


/*
 *  init_locktable ()
 *  fixes the array which corresponds to the free nodes to form a linked list.
 *  Each node has a front and back pointer.  Theses pointers are used to make
 *  the array into a circular linked list.  USER should note that no matter
 *  what freenodes [0] will always remain the head of the list. This procedure
 *  also initializes the lock table array by initializing the pointers it is 
 *  pointing to NULL.
 */
init_locktable ()
{
  register    int	counter;
  
  /* clear the nodes of the hashtable */
  for (counter = 0; counter < MAXRES; counter ++)
    {
      InitLatch(&smPtr->locktable[counter].concurrent);
      smPtr->locktable[counter].lockptr = NULL;
    }
  
  /* Initialize the free list header */
  INIT_RES_NODE_M((&smPtr->freenodes[FREECELL]), 
	&smPtr->freenodes[FREECELL + 1], &smPtr->freenodes[MAXNODE - 1], -1);
  
  for (counter = 1; counter < MAXNODE; counter ++)
    {
      /* Initialize the rest of the free list */
      INIT_RES_NODE_M((&smPtr->freenodes[counter]), 
	&smPtr->freenodes[counter + 1], &smPtr->freenodes[counter - 1], -1);
    }
  
  /* Initialize the last free element */
  INIT_RES_NODE_M((&smPtr->freenodes[MAXNODE - 1]), 
	&smPtr->freenodes[FREECELL], &smPtr->freenodes[MAXNODE - 2], -1);
}


/*
 *  init_waiters ()
 *  fixes the array which corresponds to the free wait nodes to form a linked list.
 *  Each node has a front and back pointer.  These pointers are used to make
 *  the array into a circular linked list.  USER should note that no matter
 *  what smPtr->freewaitnodes [0] will always remain the head of the list.
 */
init_waiters ()
{
  register  int	  i;
  
  INIT_WAIT_NODE_M((&smPtr->freewaitnodes[FREEWAIT]), 0, 
	&smPtr->freewaitnodes[1], &smPtr->freewaitnodes[MAXWAITNODE - 1], l_NL);
  
  
  /* attach the free nodes together */
  for (i = 1; i < MAXWAITNODE; i++)
    INIT_WAIT_NODE_M((&smPtr->freewaitnodes[i]), 0, 
	&smPtr->freewaitnodes[i + 1], &smPtr->freewaitnodes[i - 1], l_NL);
  INIT_WAIT_NODE_M((&smPtr->freewaitnodes[MAXWAITNODE - 1]), 0, 
	&smPtr->freewaitnodes[0], &smPtr->freewaitnodes[MAXWAITNODE - 2], l_NL);
}

lm_initialize_resources ()
{
  init_locktable ();
  init_waiters ();
  init_waitfor_graph ();
  InitLatch(&smPtr->wfg);
  InitLatch(&smPtr->freeLockLatch);
  InitLatch(&smPtr->freeNodeLatch);
  InitLatch(&smPtr->lockLatch);
}
