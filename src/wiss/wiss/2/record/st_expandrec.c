
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
/* Module st_expandrecord : expands the space allocated to a record 

   IMPORTS :
	bf_setdirty(filenum, pageid, pagebuf)
	bf_freebuf(fileuum, pageid, pagebuf)
	r_slide(dp, slotno, len, recptr, trans_id, lockup, cond)
	r_addrec(filenum, pageid, recaddr, reclen, newrid, append, trans_id
		 lockup, cond);
	r_hookup(filenum, pid1, pid2, &newpid, trans_id, lockup, cond);
	r_getslot(filenum, ridptr, pageptr, recptr, trans_id, lockup, mode, cond)

   EXPORTS :
	st_expandrecord(filenum, ridptr, expandamt, trans_id, lockup, cond)
*/

#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

int movedtonewpage;

st_expandrecord(filenum, ridptr, expandamt, trans_id, lockup, cond)
int	filenum;	/* open file number */
RID	*ridptr;	/* RID */
int	expandamt;	/* number of bytes to extend the record */
int     trans_id;       
short   lockup;
short	cond;

/*
   Returns:
	Number of bytes actually written

   Side Effects:
	None

   Errors:
	e2NULLRIDPTR - null RID pointer
	e2NULLRECADDR - null record address
	e2BADSLOTNUMBER - invalid record ID
*/
{
	int		e;		/* for returned errors */
	int		slotno;		/* which slot on the page */
	RECORD		*brptr;		/* the birth place of the record */
	RECORD		*rrptr;		/* the place the record is now */
	RECORD		*nrptr;		/* the place the record will be */
	PID		pid, newpid;	/* page IDs */
	RID		newrid;		/* the new address of a moved record */
	DATAPAGE	*bp = NULL;	/* the page the record was born */
	DATAPAGE	*rp = NULL;	/* the page the record resides */
	DATAPAGE	*np = NULL;	/* the page the record will be on */
	int		reclen;		/* new length of the record */
	char		*recaddr;	/* data portion of the current record */

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_expandrecord(filenum=%d,RID=", filenum);
		PRINTRIDPTR(ridptr);
		printf(",expandamt=%d)\n",expandamt);
	}
#endif

	/* check input parameters */
	CHECKOFN(filenum);
	CHECKWP(filenum);
	if (expandamt <= 0) return(0);	
	if (ridptr == NULL) return(e2NULLRIDPTR);

	/* check the birth place first */
	slotno = ridptr->Rslot;

/*
 *  if lockup is true then let r_getslot get the record,  notice that
 *  it will also lock the page in eXclusive mode.
 */
	e = r_getslot(filenum, ridptr, &bp, &brptr, trans_id, lockup, l_X, cond);
	CHECKERROR(e);

	/* if the record has been moved, track it down */
	if ( brptr->type == MOVED ) { 
		newrid = *((RID *)brptr->data);
		slotno = newrid.Rslot;
/*
 *  SH and lock the page which really has the record in eXclusive mode.
 */
		e = r_getslot(filenum, &newrid, &rp, &rrptr, 
			trans_id, lockup, l_X, cond);
		if (e < eNOERROR) goto error;
	}
	else	rp = bp, rrptr = brptr;

	reclen = rrptr->length + expandamt;
	recaddr = rrptr->data;

	/* try to expand the record in its current location */
	/* by now the page which has the record has been locked in X mode  */
	/* so r_slide is called with lockup equal to FALSE */
	/* first make room for the record */
	e = r_slide(rp, slotno, reclen, &rrptr, trans_id, FALSE, cond); 
	if (e >= eNOERROR) {
		/* r_slide was successful so the space has been made */
		(void) bf_setdirty(filenum, &(rp->thispage), rp);
		goto done;
	}

movedtonewpage++;
	/* move the record to the end of the file */
        pid = F_LASTPID(filenum);

	/* lock the last page of the file in eXclusive mode */
	e = r_addrec(filenum, &pid, recaddr, reclen, &newrid, TRUE, trans_id, 
		lockup, cond);
	if (e < eNOERROR) { /* need a new page at the end */
		/* Do not lock the page because although r_addrec failed, 
		the page is still locked in X mode */
		e = r_hookup(filenum, &pid, (PID *) NULL, &newpid, 
			trans_id, FALSE, cond);
		if (e < eNOERROR) goto error;
		/* dont lock the new pid since r_hookup has already done it */
		e = r_addrec(filenum, &newpid, recaddr, reclen, &newrid, TRUE, 
			trans_id, FALSE, cond);
		if (e < eNOERROR) goto error;
		pid = newpid;
	}
	/* mark the location of the data record a new home */
	/* no locking necessary since previously page is already locked */
	(void) r_getslot(filenum, &newrid, &np, &nrptr, trans_id, FALSE, l_NL,cond);
	nrptr->type = NEWHOME;	
	nrptr->kind = brptr->kind;
	(void) bf_freebuf(filenum, &pid, np);

	/* adjust the forwarding address */
	if (rp != bp) { /* been moved before, change forwarding address */
		(void) r_slide(rp, slotno, REMOVEREC, &rrptr, trans_id, 
			FALSE, cond);
		(void) bf_setdirty(filenum, &(rp->thispage), rp);
	}
	else { /* record moved for the first time */
		(void) r_slide(bp, slotno, sizeof(RID), &brptr, trans_id, 
			FALSE, cond);
		brptr->type = MOVED;	/* change record type */
	}
	*((RID *) brptr->data) =  newrid;
	(void) bf_setdirty(filenum, &(bp->thispage), bp);

done:
	/* unfix the buffers */
	e = bf_freebuf(filenum, &(bp->thispage), bp);
	if (e < eNOERROR) reclen = e;	/* error occurred */
	if (rp != bp) {
		e = bf_freebuf(filenum, &(rp->thispage), rp);
		if (e < eNOERROR) reclen = e;
	}
	return(reclen);	/* return actual # of bytes write (or an error code) */

error:
	/* unfix the buffers */
	if (bp!=NULL) (void) bf_freebuf(filenum, &(bp->thispage), bp);
	if (rp!=NULL && rp!=bp) (void) bf_freebuf(filenum, &(rp->thispage), rp);
	return(e);	/* return the error code */

}	/* st_expandrecord */

