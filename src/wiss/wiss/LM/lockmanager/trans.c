
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


#include <wiss.h>
#include <page.h>
#include <sm.h>
#include <lockquiz.h>


extern SMDESC   *smPtr; /* ptr to common data structures in shared memory */
extern  int     procNum; /* index of the process in the smPtr->users table */

extern int lmPageLocks;

/* interface routines to begin, committ, and abort transactions */

begin_trans()
{
	int 	transId;
	int	status;

	/* first get a unique transaction id */
	transId = smPtr->transCounter++;
	lmPageLocks=0;

	/* next call lock manager to activate the transaction */
	status = activate(transId);
	if (status != OK) 
	   printf("begin_trans:  error return of %d from activate\n",
			status);
	smPtr->users[procNum].transId = transId;
	return (transId);
}


commit_trans(transId)
	int 	transId;
{
	int status;

	/* next call lock manager to commit the transaction */
	status = lm_committ_trans(transId);
	smPtr->users[procNum].transId = -1; /* no active transaction */
	if (status < 0) return(status);
		else return(OK);
}

abort_trans(transId)
	int 	transId;
{
	int status;

	/* next call lock manager to abort the transaction */
	status = lm_abort_trans(transId);
	smPtr->users[procNum].transId = -1; /* no active transaction */
	if (status < 0) return(status);
		else return(OK);
}

