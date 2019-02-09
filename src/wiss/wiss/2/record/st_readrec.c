
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
/* Module st_readrecord : read a record  
/* also st_readfield : read a field from a record

   IMPORTS :
	r_getrecord(filenum, ridptr, pageptr, recptr, trans_id, lockup, mode, cond)
	bf_freebuf(filenum, pageid, pageptr)

   EXPORTS :
	st_readrecord(filenum, ridptr, recaddr, reclen, trans_id, lockup, 
		mode, cond)
	st_readfield(filenum, ridptr, fieldaddr, fieldoffset, fieldlen,
		trans_id, lockup, mode, cond)

*/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

st_readrecord(filenum, ridptr, recaddr, reclen, trans_id, lockup, mode, cond)
int		filenum;	/* open file number */
RID		*ridptr;	/* which record */
char		*recaddr;	/* record buffer address */
int		reclen;		/* number of bytes to read */
int             trans_id;       
short           lockup;         /* if TRUE acquire the page */
LOCKTYPE	mode;
short		cond;

/* Read a record into a buffer.

   Returns:
	Number of bytes actually read

   Side Effects:
	None

   Errors:
	e2NULLRIDPTR:	no record ID
	e2NULLRECPTR: 	no buffer address
	number of bytes read < length requested, if reclen is greater than
	  the actual length of the record
*/
{
	int		e;	/* for returned errors */
	RECORD		*recptr;/* record pointer */
	DATAPAGE	*dp;	/* page buffer pointer */

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_readrecord(open_file_no=%d, RID=", filenum);
		PRINTRIDPTR(ridptr);
		printf(" , recaddr=0x%x, reclen=%d)\n", recaddr, reclen);
	}
#endif

	/* check input parameters */
	CHECKOFN(filenum);
	if (reclen <= 0)	 return(0);
	if (ridptr == NULL) return(e2NULLRIDPTR);
	if (recaddr == NULL) return(e2NULLRECADDR);
	
	/* read the page in, and locate the record */
	e = r_getrecord(filenum, ridptr, &dp, &recptr, trans_id, 
		lockup, mode, cond);
	CHECKERROR(e);
	if (recptr->length < reclen)	
		reclen = recptr->length;	/* request out of bound, trancate */
	movebytes(recaddr, recptr->data, reclen);	/* read the data */

	e = bf_freebuf(filenum, &(dp->thispage), dp);	/* unfix the buffer */
	CHECKERROR(e);

	return(reclen);	/* return actual # of bytes read */

}	/* st_readrecord */


st_readfield(filenum, ridptr, fieldaddr, fieldoffset, fieldlen, trans_id, lockup, mode, cond)
int		filenum;	/* open file number */
RID		*ridptr;	/* which record */
char		*fieldaddr;	/* buffer address of field*/
int		fieldlen;	/* number of bytes to read */
int		fieldoffset;	/* offset of field from beginning of record */
int             trans_id;       
short           lockup;         
int		mode;		/* lock mode */

/* Read a field of a record into a buffer.

   Returns:
	field via fieldaddr
	Number of bytes actually read
	

   Side Effects:
	None

   Errors:
	e2NULLRIDPTR:	no record ID
	e2NULLRECPTR: 	no buffer address
*/
{
	int		e;	/* for returned errors */
	RECORD		*recptr;/* record pointer */
	DATAPAGE	*dp;	/* page buffer pointer */

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_readrecord(open_file_no=%d, RID=", filenum);
		PRINTRIDPTR(ridptr);
		printf(" , fieldaddr=0x%x, fieldlen=%d)\n", fieldaddr, fieldlen);
	}
#endif

	/* check input parameters */
	CHECKOFN(filenum);
	if (fieldlen <= 0)	 return(0);
	if (ridptr == NULL) return(e2NULLRIDPTR);
	if (fieldaddr == NULL) return(e2NULLRECADDR);
	
	/* read the page in, and locate the record */
	e = r_getrecord(filenum, ridptr, &dp, &recptr, trans_id, lockup, 
		mode, cond);
	CHECKERROR(e);
	if (recptr->length < fieldlen)	
	    fieldlen = recptr->length;	/* request out of bound, truncate */
	movebytes(fieldaddr, recptr->data + fieldoffset, fieldlen);
	    /* read the data */
	e = bf_freebuf(filenum, &(dp->thispage), dp);	/* unfix the buffer */
	CHECKERROR(e);

	return(fieldlen);  /* return actual # of bytes read */

}	/* st_readfield */

