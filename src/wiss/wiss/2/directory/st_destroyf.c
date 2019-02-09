
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
/* Module st_destroyf : destroy a data file 

   IMPORTS:
	io_destroyfile(fid)
	ST_zapfiledesc(volid, filename, fid)
	ST_accessfiledesc(volid, filename, f_desc, RorW)
	int sys_getuser()

   EXPORTS:
	st_destroyfile(volid, filename, trans_id, lockup, cond)

*/


#include	<wiss.h>
#include	<st.h>
#include	<lockquiz.h>

extern    struct    node    *findfile();

st_destroyfile (VolID, FileName, trans_id, lockup, cond)
int	VolID;			/* volume on which file resides */
char	*FileName;		/* name of file to create */
int     trans_id;
short   lockup;
short	cond;

/* Destroy a file

   Returns:
	None

   Side Effects :
        None

   Errors :
	e2NOPERMISSION : No permission to destroy the file
*/
{
	int	 e;		/* for error checking */
	FID	 oldfid;		/* File ID of doomed file */
	unsigned p;
	FILEDESC fd;		/* file descriptor */
	int      ret_val;

#ifdef TRACE
	if (checkset (&Trace2, tINTERFACE))
		printf("st_destroyfile(VolID=%d, FileName=%s)\n",
			VolID, FileName);
#endif

	SetLatch(&smPtr->level2Latch, procNum, NULL);
	e = ST_accessfiledesc(VolID, FileName, &fd, READ); 
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);

	/* check ownership and access permission */
	p = (fd.owner == sys_getuser()) ? OWNERWRITE : OTHERWRITE;
	if (!(fd.flags & p))
        { 
	    ReleaseLatch(&smPtr->level2Latch, procNum);
            return(e2NOPERMISSION);
        }

	/* release level2Latch before trying to acquire the lock on the file */
	ReleaseLatch(&smPtr->level2Latch, procNum);

	/* acquire the file in exclusive mode before taking actions on it */
	/* and flag everybody else that the file is no longer available */
	if (lockup == TRUE)
	{
	    if ((e = lock_file(trans_id, fd.fileid, l_X, COMMIT, cond))
		< eNOERROR)
	    {
		return(e);
	    }
	}
	/* reacquire level2Latch upon return */
	SetLatch(&smPtr->level2Latch, procNum, NULL);

	e = ST_zapfiledesc(VolID, FileName, &oldfid); /* zap directory entry */
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);

	/* next remove the file from the device */
	ret_val =  io_destroyfile(&oldfid);

	ReleaseLatch(&smPtr->level2Latch, procNum);
	return (ret_val);		

} /* st_destroyfile */
