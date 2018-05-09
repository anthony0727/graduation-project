
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

  
extern    LOCKTYPE lock_strength ();
extern struct node * malloc_node();

extern SMDESC   *smPtr; /* ptr to common data structures in shared memory */
extern  int     procNum; /* index of the process in the smPtr->users table */


/*
 *  addbucket (i)
 *  PURPOSE : adds a node to the linked list of nodes attached to one
 *  entry of the hash table. 
 */
struct   node    *addbucket (i)
     short	i;
{
  struct	node	*cell;
  
  cell = (struct node *)malloc_node ();
  if (smPtr->locktable [i].lockptr == NULL) 
    smPtr->locktable [i].lockptr = cell;
  else {
    cell -> flink = smPtr->locktable[i].lockptr;
    smPtr->locktable [i].lockptr -> blink = cell;
    smPtr->locktable [i].lockptr = cell;
  }
  return (cell);
}

/*
 *  addwaitbucket (i)
 *  PURPOSE : adds a wait bucket to the linked list of buckets attached to one
 *  node representing a resource.  
 */
struct  wait_node   *addwaitbucket (bucket, req, trans_id)
     struct	node		*bucket;
     LOCKTYPE req;
     int   trans_id;
{
  struct	wait_node	*perm, *cell;
  
  MALLOC_WAITNODE_M(cell);
  if (bucket -> wait == NULL)
    bucket -> wait = cell;
  else {
    perm = bucket -> wait;
    for (perm = bucket -> wait; perm -> flink != NULL; perm = perm -> flink);
    perm -> flink = cell;
    cell -> blink = perm;
  }
  cell -> serving = req;
  cell -> trans_id = trans_id;
  return (cell);
}

/*
 *  addwaitbucket_u ()
 *  Same as addwaitbucket, but adds the bucket to the beginning of the list
 *  Called during upgrades
 *  PURPOSE : adds a wait bucket to the linked list of buckets attached to one
 *  node representing a resource.  
 */
struct  wait_node   *addwaitbucket_u (bucket, req, trans_id)
     struct	node		*bucket;
     LOCKTYPE req;
     int   trans_id;
{
  struct	wait_node	*perm, *cell;
  
  MALLOC_WAITNODE_M(cell);
  if (bucket -> wait == NULL)
    bucket -> wait = cell;
  else {
    bucket->wait->blink = cell;
    cell -> flink = bucket->wait;
    bucket->wait = cell;
  }
  cell -> serving = req;
  cell -> trans_id = trans_id;
  return (cell);
}


short   addowner (bucket, req, trans_id)
     struct  node            *bucket;
     LOCKTYPE req;
     int   trans_id;
{
  register  struct       wait_node    *temp, *lastone, *perm, *cell;
  LOCKTYPE stronger;
  short     done;
  int       temptrans;
  
  if (bucket -> trans_id == -1)
    {
      bucket -> trans_id = trans_id;
      bucket -> serving = req;
    }
  else
    {
      MALLOC_WAITNODE_M(cell);
      perm = bucket -> o_list;
      cell -> trans_id = trans_id;
      cell -> serving = req;
      if (bucket -> o_list == NULL)
	bucket -> o_list = cell;
      else
	{
	  cell -> flink = bucket -> o_list;
	  bucket -> o_list -> blink = cell;
	  bucket -> o_list = cell;
	}
      stronger = lock_strength (bucket -> serving, req);
      if (stronger != bucket -> serving)
	{

#ifdef LMTRACE
	  printf ("-%d-Changing the place of cell and bucket.\n", trans_id);
#endif

	  stronger = bucket -> serving;
	  temptrans = bucket -> trans_id;
	  bucket -> serving = cell -> serving;
	  bucket -> trans_id = cell -> trans_id;
	  cell -> serving = stronger;
	  cell -> trans_id = temptrans;
#ifdef LMTRACE
	  printf ("-%d-After change : ", trans_id);
	  printf ("\tthe bucket : ");
	  printlock (bucket -> serving);
	  printf ("\tthe cell : ");
	  printlock (cell -> serving);
#endif
	}
      
    }
  bucket -> no_locks[req]++;
  if (bucket -> no_locks[req] == 1) 
    bucket -> no_locktypes++;
  
  return (OK);
}

short   addowner_u (bucket, req, trans_id)
     struct  node            *bucket;
     LOCKTYPE req;
     int   trans_id;
{
  register  struct       wait_node    *temp, *mainone, *cell;
  LOCKTYPE strongest, oldreq;
  short     done;
  int       temptrans;
  
#ifdef LMTRACE
  printf("-%d-addowner_u: adding req %d\n", trans_id, req);
#endif

  if (bucket -> trans_id == -1) {
    printf("-%d-addowner_u: Cannot have empty owner\n", trans_id);
    return(NOT_OK);
  }
  if (bucket -> trans_id == trans_id){
    oldreq = bucket->serving;
    bucket -> trans_id = trans_id;
    bucket -> serving = req;
    if ((LM_supr[req][oldreq] != req) &&
	(bucket->no_locktypes > 1)){
      /*
       * Swap the strongest with the first
       */
      mainone = NULL;
      strongest = bucket -> serving;
      for (temp = bucket -> o_list; temp != NULL; temp = temp -> flink) {
	if (LM_supr[temp -> serving][strongest] != strongest) {
	  strongest = temp -> serving;
	  mainone = temp;
	}
      }
      if (mainone != NULL) {
	temptrans = mainone->trans_id;
	mainone -> serving = bucket -> serving;
	mainone -> trans_id = bucket -> trans_id;
	bucket -> trans_id = temptrans;
	bucket -> serving = strongest;

#ifdef LMTRACE
	printf ("-%d-Changing the place of cell and bucket.\n",
		trans_id);
	printf ("-%d-After change : ", trans_id);
	printf ("\tthe bucket : ");
	printlock (bucket -> serving);
	printf ("\tthe cell : ");
	printlock (mainone -> serving);
#endif

      }
    }
    if (!--(bucket->no_locks[oldreq]))
      bucket->no_locktypes--;
    if (!(bucket->no_locks[req]++))
      bucket->no_locktypes++;

    return(OK);
  }
  cell = bucket -> o_list;
  while((cell != NULL)) {
    if (cell -> trans_id == trans_id)
      break;
    cell = cell->flink;
  }
  if(cell == NULL){
    printf("-%d-addowner_u: No owner node in the resource list\n",
	   trans_id);
    return(NOT_OK);
  }
  oldreq = bucket->serving;
  cell -> serving = req;
  if ((LM_supr[req][oldreq] == req) && (req != oldreq)) {
    /*
     * Swap the present with the first
     */
    temptrans = bucket -> trans_id;
    bucket -> serving = cell -> serving;
    bucket -> trans_id = cell -> trans_id;
    cell -> serving = oldreq;
    cell -> trans_id = temptrans;

#ifdef LMTRACE
    printf ("-%d-Changing the place of cell and bucket.\n", trans_id);
    printf ("-%d-After change : ", trans_id);
    printf ("\tthe bucket : ");
    printlock (bucket -> serving);
    printf ("\tthe cell : ");
    printlock (cell -> serving);
#endif

  }
  if (!--(bucket->no_locks[oldreq]))
    bucket->no_locktypes--;
  if (!(bucket->no_locks[req]++))
    bucket->no_locktypes++;

  return(OK);
}

