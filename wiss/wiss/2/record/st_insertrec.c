
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
/* Module  st_insertrecord :

   IMPORTS:
	bf_readbuf(trans_id, filenum, fid, pageid, returnpage);
	bf_freebuf(filenum, pageid, pagebuf);
	r_addrec(filenum, onpid, recaddr, reclen, newrid, append, trans_id, 
		 lockup, cond)
	r_hookup(filenum, firstpid, secondpid, newpid, trans_id, lockup, cond)
	st_appendrecord(filenum, recaddr, reclen, newrid, trans_id, lockup, cond)

   EXPORTS:
	st_insertrecord(filenum, recaddr, reclen, nearrid, newwrid, 
		trans_id, lockup, cond)

*/

#include	<wiss.h>
#include	<st.h>

st_insertrecord(filenum, recaddr, reclen, nearrid, newrid, trans_id, lockup, cond)
int	filenum;	/* index into open file array */
char	*recaddr;	/* pointer to the record */
int	reclen;		/* how long it is */
RID	*nearrid;	/* close to this record if possible */
RID	*newrid;	/* the value of the new rid */
int     trans_id;
short   lockup;
short   cond;

/* insert a record to the file

   Returns:
	the RID of the newly created record

   Side Effects:
	If need be, a new page is allocated from a nearby location.

   Errors:
	e2NULLRECADDR: null record address

*/
{
	PID		pid, nextpid, newpid;
	int		e; 		/* for returned errors */
	DATAPAGE	*dp;	

#ifdef	TRACE
	if (checkset(&Trace2,tINTERFACE)) {
		printf("st_insertrecord(filenum=%d,",filenum);
		printf(" nearrid="); PRINTRIDPTR(nearrid);
		printf(", recaddr=0x%x, reclen=%d, *newrid=0x%x, trans_id %d, lockup %d)\n",
				recaddr,reclen, newrid, trans_id, lockup);
	}
#endif

	/* check input parameters */
	CHECKOFN(filenum);
	CHECKWP(filenum);
	if (recaddr == NULL) return(e2NULLRECADDR);
	if (reclen <= 0) return(0);

	/* if no desired location specified, append it at the end */
	if (nearrid == NULL || TESTRIDCLEAR(*nearrid))
	  return(st_appendrecord(filenum, recaddr, reclen, newrid, 
			trans_id, lockup, cond));

	/* insert the record as close as possible to the given location */
	GETPID(pid, *nearrid);
	e = r_addrec(filenum, &pid, recaddr, reclen, newrid, FALSE, 
		trans_id, lockup, cond);
	if (e >= eNOERROR) goto done;	/* record inserted */
	if (e != e2RECWONTFIT) return(e);

	/* try the next page */
	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, &dp);
	CHECKERROR(e);
	nextpid.Pvolid = pid.Pvolid;
	nextpid.Ppage = dp->nextpage;
	e = bf_freebuf(filenum, &pid, dp);
	CHECKERROR(e);

	if (nextpid.Ppage != NULLPAGE) {
		e = r_addrec(filenum, &nextpid, recaddr, reclen, newrid, FALSE, 
			trans_id, lockup, cond);
		if (e >= eNOERROR) goto done;	/* record inserted */
	}
	else {
		PIDCLEAR(nextpid);
	}

	/* get a new page and put the record on it */
	e = r_hookup(filenum, &pid, &nextpid, &newpid, trans_id, lockup, cond);
	CHECKERROR(e);
	e = r_addrec(filenum, &newpid, recaddr, reclen, newrid, FALSE, 
		trans_id, FALSE, cond);
	/* lockup is set equal to false as r_hookup() will have locked the page */
	CHECKERROR(e);
done:
	if (F_FILETYPE(filenum) == DATAFILE) 
	{
		F_CARD(filenum)++;   /* the file has one more record */
	}
	return(eNOERROR);

} /* st_insertrecord */

