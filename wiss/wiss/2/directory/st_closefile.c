
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
/* Module st_closefile :

   IMPORTS : 
	ST_accessfiledesc();

   EXPORTS : 
	st_closefile (filenum)
   
*/

#include <wiss.h>	
#include <st.h>

st_closefile(filenum, setLatch)
int	filenum;	/* open file number */
int	setLatch;	/* if TRUE, set the level 2 latch, otherwise,
			assume the caller set it */

/* 
   Close the specified file : remove entry from open file table and
	update the directory if necessary.

   Returns:
	NONE

   Side Effects:
	file descriptor (in the file directory) of this file may be modified

   Errors:
	e2BADOPENFILENUM
	e2WRONGUSER
*/

{
	register int	e;	/* for returned errors */
	PID      pid;

#ifdef TRACE
	if (checkset(&Trace2,tINTERFACE))
		printf("ST_closefile(filenum=%d)\n",filenum);
#endif

	CHECKOFN(filenum);		/* check open file number */

	/* BF_event(CurrentTask->ncb_name,"st_closefile",&pid,-1); */

	if (setLatch) SetLatch(&smPtr->level2Latch, procNum, NULL);
	/* first if a page remains pinned, unpin it */
	if (F_LASTPINNED(filenum))
	     bf_freebuf(filenum, &F_LASTPIDPINNED(filenum), F_BUFADDR(filenum));
	(F_REFCOUNT(filenum))--;

	/* close the level 1 file */
	if (F_ACCESSMODE(filenum) == READ) 
		e = bf_closefile(filenum);
	else
		e = bf_flushbuf(filenum, TRUE);
		
	if (setLatch) CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum)
	else CHECKERROR(e);

	if (F_REFCOUNT(filenum) > 0) goto done;

	/* update file descriptor in the directory if necessary */
	if (F_STATUS(filenum) == DIRTY) 
	{	
		e = ST_accessfiledesc(F_VOLUMEID(filenum),
			F_FILENAME(filenum), &(F_DESC(filenum)), WRITE);
		if (setLatch) CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum)
		else CHECKERROR(e);
	}
	F_STATUS(filenum) = NOTINUSE;	/* release table entry */

done:
	smPtr->files[filenum].ptr = -1;
	if (setLatch) ReleaseLatch(&smPtr->level2Latch, procNum);
	return(eNOERROR);
} /* st_closefile */
