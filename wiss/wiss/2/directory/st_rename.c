
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
/* Module st_rename : rename a file

   IMPORTS:
	ST_accessfiledesc(volid, filename, *descriptor, accessflag)
	ST_newfiledesc(volid, filename, *fid, pff)
	ST_zapfiledesc(volid, filename)
	int sys_getuser()

   EXPORTS:
	st_rename(volid, new, old)

*/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

extern	char	*strcpy();

st_rename(volid, newname, oldname, trans_id, lockup, cond)
int	volid;
char	*newname, *oldname;
int     trans_id;
short   lockup;
short	cond;

/* rename a file

   Returns:
	NONE

   Side Effects:
	Modify file descriptor in directory.

   Errors generated here:
	e2NOSUCHFILE
	e2FILEALREADYEXISTS
	e2NOPERMISSION : No permission to destroy the file

*/
{
	register int	e;		/* for error checking */
	FILEDESC	desc;
	unsigned p;

#ifdef TRACE
	if (checkset (&Trace2, tINTERFACE))
		printf("st_rename(newname = %s, oldname = %s)\n",
		newname, oldname);
#endif
	SetLatch(&smPtr->level2Latch, procNum, NULL);
	/* check if the new file already exists */
	e = ST_accessfiledesc(volid, newname, &desc, READ);
	if (e >= eNOERROR)
        { 
		ReleaseLatch(&smPtr->level2Latch, procNum);
	  	return(e2FILEALREADYEXISTS);
	}
	if (e != e2NOSUCHFILE)
	{ 
		ReleaseLatch(&smPtr->level2Latch, procNum);
		return(e);
	}

	/* check ownership and access permission */
	e = ST_accessfiledesc(volid, oldname, &desc, READ);
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);

	p = (desc.owner == sys_getuser()) ? OWNERWRITE : OTHERWRITE;
	if (!(desc.flags & p)) 
	{
		ReleaseLatch(&smPtr->level2Latch, procNum);
		return(e2NOPERMISSION);
	}

	/* release latch before calling lock_file in case the lock blocks */
	ReleaseLatch(&smPtr->level2Latch, procNum);

	/* lock the new file in eXclusive mode */
	if (lockup == TRUE)
	    if ((e = lock_file(trans_id, desc.fileid, l_X, COMMIT, cond))
		< eNOERROR)
	    {
		ReleaseLatch(&smPtr->level2Latch, procNum);
		return (e);
	    }

	/* reacquire latch upon return. we don't refetch the descriptor
	for old because nothing important could have changed while we were off
	setting the lock */
	SetLatch(&smPtr->level2Latch, procNum, NULL);

	/* create a new file descriptor */
	e = ST_newfiledesc(volid, newname, &(desc.fileid), desc.pff);
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);

	(void)strcpy(desc.filename, newname);
	e = ST_accessfiledesc(volid, newname, &desc, WRITE);
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);

	/* destroy the old file descriptor */
	e = ST_zapfiledesc(volid, oldname, &(desc.fileid));
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);

	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(eNOERROR);

} /* st_rename */
