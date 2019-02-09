
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
/* Module : am_writelog
      writes a log record on a delta file for the current record of the scan

   IMPORTS:
      int    st_writerecord(OpenFileNum, *RID, RecAdr, Len, 
		trans_id, lockup, cond)
      int    st_appendrecord(OpenFileNum, RecAdr, Len, *NewRID, 
		trans_id, lockup, cond)

   EXPORTS:
      int    am_writelog( ... )
*/
  
#include    <wiss.h>
#include    <am.h>
#include     <lockquiz.h>


int am_writelog(type, sptr, oldvalue, oldvaluelen, newvalue, 
	newvaluelen)

SCANINFO *sptr;     /* identifies the scan */
enum upd_type type;    /* INSERT, UPDATE, or DELETE */
char     *oldvalue;    /* pointer to the old value */
int      oldvaluelen;  /* length of old value */
char     *newvalue;    /* pointer to the new value */
int      newvaluelen;  /* length of new value */
  
{
    RID        dummy;    /* used for st_appendrecord */
    char       buf[PAGESIZE];    /* buffer for update record */
    UPDRECORD  *uptr = (UPDRECORD *) buf;
    int        keyoffset, keylength;
    int        e;    /* for returned errors */

    /* load the old attribute value into the log record */
        bcopy(oldvalue, uptr->image, oldvaluelen);

    /* if the type is UPDATE, also include the new new value */
    if (type == UPDATE)
        bcopy(newvalue, uptr->image+oldvaluelen, newvaluelen);

    /* finish constructing the update record  */
    uptr->type = type;
    uptr->datarid = sptr->rid;

    /* add record to the delta file for the scan */
    e = st_appendrecord(sptr->deltafile, buf, 
      		(UPDHEADERLEN + oldvaluelen + newvaluelen), &dummy, 
		sptr->trans_id, FALSE, sptr->cond);
    return(e);    /* return any possible error code */

} /* am_writelog */
