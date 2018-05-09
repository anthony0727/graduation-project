
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
/* Module : am_closescan
      This routine closes the scan associated with scanid
  
   IMPORTS:
      int	st_compresslong(openfileno, *dir)
      int	st_accessmode(open_file_number)
      SCANINFO *AM_getscan(scanid)
      int	AM_commit(dataofn, indexofn, updateofn, *keyattr, trans_id,
    	lockup, cond)
      int	AM_removescan(scanid)
  
   EXPORTS:
      int	am_closescan(scanid)
*/
  
#include    <wiss.h>
#include    <am.h>
#include        <lockquiz.h>


extern    SCANINFO *AM_getscan();

int
am_closescan(scanid)
int     scanid;	    /* ID of the scan to be closed */

/* This routine closes the scan associated with scanid, and possilby
        does some post-processing depending on the type of scan.
  
     Returns:
      NONE
  
     Side Effects:
      Removes the scan identified by ScanId from the scan table.
      If this was a Long Data Item scan, this routine also compresses
      the data if any updates were performed upon it.
      For index scan, all the deferred updates are committed here.
  
     Errors generated here:
      None
*/

{
    register SCANINFO 	*sptr;	/* pointer to scan info record */
    register int 	e;	/* error return value */
    int	    volid;	/* volid ID of the file(s) */
    char	tname[40];	/* name of the temporary update file */

#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    	printf("am_closescan(scanid = %d)\n", scanid);
#endif

    sptr = AM_getscan(scanid);	/* get address of scan info */
    if (sptr == NULL) return(e3BADSCANID);

    switch (sptr->scantype)
    {
        case LONGSCAN:
    	/* compress the long data item if it has been touched */
    	if (sptr->dirty_bit)
    	{
    	    e = st_compresslong(sptr->filenum,&(sptr->rid),100, 
		sptr->trans_id, sptr->lockup, sptr->cond);
    	    CHECKERROR(e);
    	}
    	break;	/* end of LONGSCAN */

        case INDEXSCAN:
    	/* commit deferred updates if this scan is open for WRITE */
    	if (sptr->accessflag == WRITE)
    	{
    	    e = AM_commit(scanid);
    	    CHECKERROR(e);

    	    /* get rid of the temporary update file */
    	    suffixname(tname, "tmp", 't', scanid);

    	    volid = st_volid(sptr->deltafile);
    	    CHECKERROR(volid);

    	    e = st_closefile(sptr->deltafile, TRUE);
    	    CHECKERROR(e);
    	    e = st_destroyfile(volid, tname, sptr->trans_id, FALSE, sptr->cond);
    	    CHECKERROR(e);
    	}
    	break;	/* end of INDEXSCAN */

        case HASHSCAN:
    	/* commit deferred updates if this scan is open for WRITE */
    	if (sptr->accessflag == WRITE)
    	{
    	    e = AM_commit1(scanid);
    	    CHECKERROR(e);

    	    /* get rid of the temporary update file */
    	    suffixname(tname, "tmp", 't', scanid);
    	    volid = st_volid(sptr->deltafile);
    	    CHECKERROR(volid);
    	    e = st_closefile(sptr->deltafile, TRUE);
    	    CHECKERROR(e);
    	    e = st_destroyfile(volid, tname, sptr->trans_id, FALSE, sptr->cond);
    	    CHECKERROR(e);
    	}
    	break;	/* end of HASHSCAN */

    } /* end of switch */

    e = AM_removescan(scanid);	/* remove it from scan table */
    CHECKERROR(e);

    return (eNOERROR);

} /* am_closescan */
