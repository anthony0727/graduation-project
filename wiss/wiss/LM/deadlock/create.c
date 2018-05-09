
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

struct    graph_bucket    *malloc_graph_node ();
struct    lockq           *malloc_lock ();

char	*shmAlloc();

struct    graph_bucket    *create_graphnode (hashIdx, trans_id)
int     hashIdx; 
int trans_id;
{
    register int i;
    register struct    graph_bucket    *cell;
    register struct fidLocalHash  *fidHash;
    register struct pidLocalHash  *pidHash;

    cell = malloc_graph_node ();
    cell -> trans_id = trans_id;

    /* next malloc local file and page hash tables */
    /* NOTE!!!!  These tables MUST go into shared memory */
    /* as if a transaction waits, the transation that wakes it up */
    /* must manipulate these tables in walkfile() */
    /* however, they don't need to be protected with semaphore */

    /* NOTE.  These used to not be in shared memory */

    cell -> fidHashTbl = 
	(struct fidLocalHash *) shmAlloc(sizeof(struct fidLocalHash));
    fidHash = cell->fidHashTbl;
    if (fidHash != NULL) 
    {
        for (i=0;i<FIDLOCKTBLSIZE;i++) fidHash->fidChain[i] = NULL;
    }
    else printf("shmAlloc of fidHashTbl failed in create_graph_node\n");

    cell -> pidHashTbl = 
	(struct pidLocalHash *) shmAlloc(sizeof(struct pidLocalHash));
    pidHash = cell->pidHashTbl;
    if (pidHash != NULL) 
    {
        for (i=0;i<PIDLOCKTBLSIZE;i++) pidHash->pidChain[i] = NULL;
    }
    else printf("shmAlloc of pidHashTbl failed in create_graph_node\n");

    SetLatch(&smPtr->trans [hashIdx].concurrent, procNum, NULL);
    if (smPtr->trans[hashIdx].transptr == NULL) 
    {
	smPtr->trans[hashIdx].transptr = cell;
	cell -> flink = cell -> blink = NULL;
    }
    else 
    {
	cell -> flink = smPtr->trans[hashIdx].transptr;
	cell -> blink = NULL;
	smPtr->trans[hashIdx].transptr->blink = cell;
	smPtr->trans[hashIdx].transptr = cell;
    }
    ReleaseLatch(&smPtr->trans [hashIdx].concurrent, procNum);
    return (cell);
}

