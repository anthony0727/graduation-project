
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
/* Module st_getrecord : Level 2 internal routine to locate a record

   IMPORTS:
        bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
	bf_freebuf(filenum, pageid, pagebuf)
	bf_setdirty(filenum, pageid, pagebuf)

   EXPORTS:
	st_getrecord(filenum, ridptr, pageptr, recptr, trans_id, 
		lockup, mode, cond)
*/

#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>


st_getrecord(filenum, ridptr, pageptr, recptr, trans_id, lockup, mode, cond)
int		filenum;	/* open file number */
RID		*ridptr;	/* *RID */
DATAPAGE	**pageptr;	/* out parameter - pointer to page buffer */
char		**recptr;	/* out parameter - record address */
int             trans_id;       
short           lockup;        
int		mode;
short           cond;   

/* Given a RID, locate the record and return its memory address.
   If the record has been moved, follow the forwarding address to find it.

   Returns:
	pointer to data portion of a record (via recptr)
	page address (via pageptr) 
	lenght of the record in bytes

   Side Effects:
	fix the buffer which holds the record

   Errors:
	e2BADSLOTNUMBER - invalid slot number
*/
{
	int		e;	/* for returned errors */
	RECORD		*record;
	DATAPAGE	*dp;	/* pointer to the page buffer */
	
	/* get a pointer to the WiSS record  in record */
	e = r_getrecord(filenum, ridptr, &dp, &record, trans_id, lockup, 
		mode, cond);
	
	if (e < eNOERROR) 
	{
		return(e);
	}
	else
	{
	   *recptr = &record->data[0];
	   *pageptr = dp;
	   return (record->length);
	}
}
