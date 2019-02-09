
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
  
extern    char    *shmAlloc ();

extern SMDESC   *smPtr; /* ptr to common data structures in shared memory */
extern  int     procNum; /* index of the process in the smPtr->users table */


/*
 *  Every time the list of free nodes is exhausted shmAlloc is called to 
 *  allocate a bunch of nodes.  They are then initialized by fixing their
 *  front and back pointer to point to each other.  Notice that no matter
 *  what smPtr->freenodes [0] always remains the head of the free nodes.
 */
alloc_more_node ()
{
  register     struct	node	*p, *perm;
  register     int		counter;
  
#ifdef LMTRACE
  printf ("\n===========  ALLOCATING MORE NODES   ==========\n");
#endif

  p = (struct node *) shmAlloc (sizeof (smPtr->freenodes));
  if (p == NULL)
  {
    printf("shmAlloc failed in allocating more lock nodes on server\n");
    printf("increase shared memory heap space \n");
  }

  perm = p;
  INIT_RES_NODE_M(perm, perm+1, &smPtr->freenodes[FREECELL], -1);
  for (counter = 2, p++; counter < MAXNODE; counter ++, p ++) 
    INIT_RES_NODE_M(p, p + 1, p - 1, -1);
  INIT_RES_NODE_M((&smPtr->freenodes[FREECELL]), perm, p, -1);
  INIT_RES_NODE_M(p, &smPtr->freenodes [FREECELL], p - 1, -1);

#ifdef LMTRACE
  printf("Returning after acquiring nodes.\n");
#endif

}


/*
 *  alloc_more_waitnode 
 *  PURPOSE : Allocates more nodes every times freewaitnodes is exhausted.
 *  After receiving the nodes it fixes the front and back pointers
 *  of the nodes.  Note that no matter what smPtr->freewaitnodes [0] 
 *  always remains the head of free resources.
 */
alloc_more_waitnode ()
{
  register     struct 	wait_node	*p, *perm;
  register     int		counter;
  
#ifdef LMTRACE
  printf ("\n===========  ALLOCATING MORE WAIT NODES   ==========\n");
#endif

  p = (struct wait_node *) shmAlloc (sizeof (smPtr->freewaitnodes));
  if (p == NULL)
  {
    printf("shmAlloc failed in allocating more lock nodes on server\n");
    printf("increase shared memory heap space \n");
  }

  perm = p;
  INIT_WAIT_NODE_M(perm, 0, perm+1, &smPtr->freewaitnodes [FREECELL], l_NL);
  for (counter = 2,p++; counter < MAXWAITNODE; counter ++, p ++)
    INIT_WAIT_NODE_M(p, 0, p + 1, p - 1, l_NL);
  INIT_WAIT_NODE_M((&smPtr->freewaitnodes [FREECELL]), 0, perm, p, l_NL);
  INIT_WAIT_NODE_M(p, 0, &smPtr->freewaitnodes [FREECELL], p - 1, l_NL);
}

/*
 *  malloc_node ()
 *  PURPOSE : Takes a node from the head of the free node list and returns it to
 *  the requestor. If the list is exhausted calls alloc_more_node to allocate 
 *  more nodes.
 */
struct	node	*malloc_node () 
{
  struct	node	*found;
  
  SetLatch(&smPtr->freeNodeLatch, procNum, NULL);
  if (smPtr->freenodes [FREECELL].flink == &smPtr->freenodes[FREECELL])
    alloc_more_node ();
  
  found = smPtr->freenodes [FREECELL].flink;
  
  /* disconnect found node from free list */
  smPtr->freenodes [FREECELL].flink = found -> flink;
  found -> flink -> blink = &smPtr->freenodes [FREECELL]; 
  found -> trans_id = -1;
  found -> wait = found -> o_list = NULL;
  found -> flink = found -> blink = NULL;
  InitLatch(&found->concurrent);
  ReleaseLatch(&smPtr->freeNodeLatch, procNum);
  return (found);
}


/*
 *  malloc_waitnode ()
 *  PURPOSE : Takes a node from the head of the free wait list and returns it to
 *  the requestor. If the list is exhausted calls alloc_more_waitnode to 
 *  allocate more nodes.
 */
struct	wait_node	*malloc_waitnode () 
{
  struct	wait_node	*found;
  
  SetLatch(&smPtr->lockLatch, procNum, NULL);
  if (smPtr->freewaitnodes [FREEWAIT].flink == &smPtr->freewaitnodes[FREEWAIT])
    alloc_more_waitnode ();
  
  found = smPtr->freewaitnodes [FREEWAIT].flink;			
  
  /*disconnect found node from free list */
  smPtr->freewaitnodes [FREEWAIT].flink = found -> flink;
  found -> flink -> blink = &smPtr->freewaitnodes[FREECELL];
  found -> flink = found -> blink = NULL;
  ReleaseLatch(&smPtr->lockLatch, procNum);
  return (found);
}


/*
 *  free_wait_node ()
 *  Takes the wait node which is to be freed, initializes it, attaches it to 
 *  the head of the free wait nodes. 
 */
free_wait_node (cell)
     struct 	wait_node	*cell;
{
  SetLatch(&smPtr->lockLatch, procNum, NULL);
  INIT_WAIT_NODE_M(cell, 0, (smPtr->freewaitnodes [FREEWAIT].flink), 
	&smPtr->freewaitnodes [0], l_NL);
  smPtr->freewaitnodes [0].flink -> blink = cell;
  smPtr->freewaitnodes [0].flink = cell;
  ReleaseLatch(&smPtr->lockLatch, procNum);
}


/*
 *  free_bucket ()
 *  Takes the node which is to be freed, initializes it, attaches it to 
 *  the head of the free nodes.
 */
free_bucket (cell)
     struct	node	*cell;
{
  INIT_RES_NODE_M(cell, smPtr->freenodes [0].flink, &smPtr->freenodes [0], -1);
  smPtr->freenodes [0].flink -> blink = cell;
  smPtr->freenodes [0].flink = cell;
}



/*
 *  release_node (i, bucket)
 *  Releases the node which was allocated to a resource and returns it to
 *  the free list.  Fixes the pointers of the nodes in the hash table.
 *  since the actions taking place are on one linked list in the table
 */
release_node (i, bucket)
     short 	i;
     struct 	node	*bucket;
{
  
  SetLatch(&smPtr->freeNodeLatch, procNum, NULL);
  if (bucket -> blink != NULL)
    bucket -> blink -> flink = bucket -> flink;
  if (bucket -> flink != NULL)
    bucket -> flink -> blink = bucket -> blink;
  if (smPtr->locktable [i].lockptr == bucket) 
    smPtr->locktable [i].lockptr = bucket -> flink;
  free_bucket (bucket);
  ReleaseLatch(&smPtr->freeNodeLatch, procNum);
}
