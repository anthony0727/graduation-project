#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

r_pinpage(filenum, pidptr, pageptr, trans_id, lockup, mode, cond)
int		filenum;	/* open file number */
PID		*pidptr;	/* *RID */
DATAPAGE	**pageptr;	/* out parameter - pointer to page buffer */
int             trans_id;
short           lockup;
LOCKTYPE	mode;
short           cond;
{
	int		e;	/* for returned errors */
	PID		pid;	/* the page the record is on */
	DATAPAGE	*dp;	/* pointer to the page buffer */

#ifdef TRACE
	if (checkset(&Trace2, tGETRECORD)) {
		printf("r_getslot(filenum=%d%, RID=", filenum);
		PRINTRIDPTR(ridptr);
		printf(", pageptr=0x%x", pageptr);
		printf(", trans_id=%d, lockup=%c, ", trans_id, lockup ? 'T' : 'F');
		printf(")\n");
	}
#endif

	/* read the page in */
	pid = *pidptr;

	/* BF_event(CurrentTask->ncb_name,"getrec",&pid,-1); */

	/* lock the page containing the record in the required access mode */
	if (lockup)
	{
		e = lock_page(trans_id, FC_FILEID(filenum), pid, mode, COMMIT, cond);
		CHECKERROR(e);
	}

	e = bf_pinpage(trans_id, filenum, FC_FILEID(filenum), &pid, &dp);
	CHECKERROR(e);

	if (e < eNOERROR) (void)bf_freebuf(filenum, &pid, dp);
	else *pageptr = dp; /* return a pointer to the page buffer */

	return(e);

}	/* r_getpage */