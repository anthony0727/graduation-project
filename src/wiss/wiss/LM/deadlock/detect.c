
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
#include 	<LM_error.h>

extern SMDESC   *smPtr; /* ptr to common data structures in shared memory */
extern  int     procNum; /* index of the process in the smPtr->users table */

extern struct graph_bucket * create_graphnode();

  /*
   *  find_trans (trans_id)
   *  Given a transaction id, it finds the node corresponding to the transaction
   *  in the transaction wait for graph.
   */
struct    graph_bucket    *find_trans (trans_id)
int     trans_id;
{
  register    struct    graph_bucket    *temp;
  int	index;

  index = lockhash(trans_id);
  SetLatch(&smPtr->trans [index].concurrent, procNum, NULL);
  for (temp = smPtr->trans [index].transptr; 
  	(temp != NULL && (temp->trans_id != trans_id)); temp = temp -> flink);
  ReleaseLatch(&smPtr->trans [index].concurrent, procNum);
  return (temp);
}


/*
 *  activate (trans_id)
 *  given a transaction id, a cell for that transaction is created in the wait
 *  for graph.  A check is made to make sure that a transaction is not created
 *  twice.
 */
int activate (trans_id)
     int    trans_id;
{
  short    index;
  
#ifdef LMTRACE
  printf ("-%d-Entering Activate \n",trans_id);
#endif

  index = lockhash (trans_id);
  if (find_trans (trans_id) == NULL) create_graphnode (index, trans_id);
  else
    {
      printf ("\n-%d-ERROR, The given trans id is a duplicate, transactions should be unique.\n", trans_id);
      return (NOT_OK);
    }
  return (OK);
}


/*
 *  terminate (trans_cell)
 *  Given a transaction identifer, it is terminated by taking the entry 
 *  corresponding to it out of the wait for graph.
 */
terminate (trans_cell)
     struct   graph_bucket    *trans_cell;
{
  short    index;
  
#ifdef LMTRACE
  printf ("-%d-Terminate \n",trans_cell->trans_id); 
#endif

  index = lockhash (trans_cell->trans_id);
  
  if (trans_cell == NULL)
    printf ("-%d-ERROR, NO such a transaction to terminate.\n",
	    trans_cell->trans_id);
  else 
    free_graph_node (index, trans_cell);
}


/*
 *  check_global_deadlock (holder, requestor)
 *  Given a holder and requestor it tries to detect global deadlock.
 */
int    check_deadlock (trans_id, finger)
     int    trans_id;
     struct   node      *finger;
{
  short    temp;
  struct   graph_bucket          *trans;
  register struct   wait_node    *ptr;
  
  if (finger == NULL) return (OK);
  
#ifdef LMTRACE
  printf ("-%d-Entering check_deadlock and finger->trans %d.\n",
	  trans_id, finger->trans_id);
#endif
  
  if (trans_id == finger ->trans_id) return(REACQUIRE);
  for (ptr = finger -> o_list; ptr != NULL; ptr = ptr -> flink)
    {
      if (trans_id == ptr -> trans_id) 
	{
	  return(REACQUIRE);
	}
    }
  
  trans = find_trans(finger->trans_id);
  if (trans == NULL) {
    printf("ERROR (proc check_deadlock.1), Null transaction.\n");
    return(lm_NULLTRANSCELL);
  }
  /* latch the transaction node */
  SetLatch (&trans->concurrent, procNum, NULL);
  
  if (trans->res_wait == finger)
    {
      /* this can happen if the resource owner is waiting for an upgrade */
      /*
	printf ("\n-%d-\tERROR,  such a case should have never happenend.\n",
	      trans_id);
      trans->res_wait = NULL;
      */
      ReleaseLatch (&trans->concurrent, procNum);
      return(OK);
    }

  switch (check_deadlock(trans_id, trans->res_wait))
    {
    case DEADLOCK : ;
    case REACQUIRE : 
      	ReleaseLatch (&trans->concurrent, procNum);
    	return(DEADLOCK);
      break;
    }
  
  ReleaseLatch (&trans->concurrent, procNum);
  for (ptr = finger -> o_list; (ptr != NULL); ptr = ptr -> flink)
    {
      trans = find_trans(ptr->trans_id);
      if (trans == NULL) 
      {
	  printf("ERROR (proc check_deadlock.2), Null transaction.\n");
	  return(lm_NULLTRANSCELL);
      }
      SetLatch (&trans->concurrent, procNum, NULL);

      /* to avoid an infinite loop */
      if (trans->resource_wait && trans->resource_wait->upgrade) continue;

      switch (check_deadlock(trans_id, trans->res_wait))
	{
	case DEADLOCK : ;
	case REACQUIRE : 
      		ReleaseLatch (&trans->concurrent, procNum);
		return(DEADLOCK);
	  	break;
	}
    }
  return (OK);
}

/*
 *  check_global_deadlock (holder, requestor)
 *  This deadlock routine is called when Lock_upgrades are allowed
 *  Given a holder and requestor it tries to detect global deadlock.
 */
int    check_deadlock_u (trans_id, finger)
     int    trans_id;
     struct   node      *finger;
{
  short    temp;
  struct   graph_bucket          *trans;
  register struct   wait_node    *ptr;
  
  if (finger == NULL) 
    return (OK);
  
  
#ifdef LMTRACE
  printf ("-%d-Entering check_deadlock_u with finger->trans %d.\n",
	  trans_id, finger->trans_id);
#endif
  
  
  if (trans_id != finger->trans_id) {
    /*
     * Check all holders recursively for deadlock. Ignore this
     * transaction itself, since it is an upgrade situation.
     * However if this transaction is encountered due to a cycle
     * at another level in the recursion, then deadlock is signalled
     */
    trans = find_trans(finger->trans_id);
    if (trans == NULL) {
      printf("ERROR (proc check_deadlock_u.1), Null transaction.\n");
      return(lm_NULLTRANSCELL);
    }
    /* 12/91 moved following statement from before if statement to here */
    SetLatch (&trans->concurrent, procNum, NULL);

    /*
      if (trans->res_wait == finger)
      {
      printf ("\n-%d-\tERROR,  such a case should have never happenend.\n"
      , trans_id);
      trans->res_wait = NULL;
      }
    */
    switch (check_deadlock(trans_id, trans->res_wait))
      {
      case DEADLOCK : ;
      case REACQUIRE : 
    		ReleaseLatch(&trans->concurrent, procNum);
      		return(DEADLOCK);
		break;
      }
  }
  
  /* 12/91 added conditional test as SetLatch call was performed under
  such a conditional test */
  if (trans_id != finger->trans_id) ReleaseLatch(&trans->concurrent, procNum);

  for (ptr = finger -> o_list; (ptr != NULL); ptr = ptr -> flink){
    if (trans_id != ptr -> trans_id) {
      trans = find_trans(ptr->trans_id);
      if (trans == NULL) {
      	  printf("ERROR (proc check_deadlock_u.1), Null transaction.\n");
      	  return(lm_NULLTRANSCELL);
      }
      SetLatch (&trans->concurrent, procNum, NULL);
      switch (check_deadlock(trans_id, trans->res_wait)) {
      case DEADLOCK : ;
      case REACQUIRE : 
    		ReleaseLatch(&trans->concurrent, procNum);
      		return(DEADLOCK);
		break;
      }
    }
  }
  return (OK);
}
