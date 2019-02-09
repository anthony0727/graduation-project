
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
#include	<lockquiz.h>


extern SMDESC   *smPtr; /* ptr to common data structures in shared memory */
extern  int     procNum; /* index of the process in the smPtr->users table */
  
init_lock_node (node, owner, flink, blink)
enum	belongsto 	owner;
struct	lockq		*flink, *blink, *node;
{
  node -> resptr = NULL;
  node -> flink = flink;
  node -> blink = blink;
  node -> collision = NULL;
}


init_graph_node (node, trans_id, flink, blink)
     int	trans_id;
     struct	graph_bucket	*node, *flink, *blink;
{
  node -> resource_wait = NULL;
  node -> someone_waiting = FALSE;
  node -> trans_id = trans_id;
  node -> waiting_for = NULL;
  node -> flink = flink;
  node -> blink = blink;
  node -> locks_held = NULL;
}


init_lockq ()
{
  short    i;
  
  init_lock_node (&smPtr->freelocks [0], NOBODY, 
	&smPtr->freelocks [1], &smPtr->freelocks [MAXNODE - 1]);
  for (i = 1; i < (MAXNODE - 1); i++)
    init_lock_node (&smPtr->freelocks [i], NOBODY, 
	&smPtr->freelocks [i + 1], &smPtr->freelocks [i - 1]);
  init_lock_node (&smPtr->freelocks [MAXNODE - 1], NOBODY, &smPtr->freelocks [0], &smPtr->freelocks [MAXNODE - 2]);
}


init_graph ()
{
  short    i;
  
  init_graph_node (&smPtr->freegraph [0], -1, &smPtr->freegraph [1], 
	&smPtr->freegraph [MAXTRANS - 1]);
  for (i = 1; i < MAXTRANS; i ++)
    init_graph_node (&smPtr->freegraph [i], -1, &smPtr->freegraph [i + 1], 
	&smPtr->freegraph [i - 1]);
  init_graph_node (&smPtr->freegraph [MAXTRANS - 1], -1, 
	&smPtr->freegraph [0], &smPtr->freegraph [MAXTRANS - 2]); 
}


init_waitfor_graph ()
{
  short    i;
  
  for (i = 0; i < MAXTRANS; i ++)
  {
    smPtr->trans [i].transptr = NULL;
    InitLatch(&smPtr->trans[i].concurrent);
  }
  init_graph();
  init_lockq();
}
