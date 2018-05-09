
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
/* Module : am_openfilescan
      Opens a scan on an opened data file.
  
   IMPORTS:
      int	 AM_addscan(OpenFileNum)     
      SCANINFO *AM_getscan(scanID)
  
   EXPORT:
      int	am_openfilescan(openfilenum, booleanexpr, trans_id, lockup,
			mode, cond)
*/

#include <wiss.h>
#include <am.h>
#include <lockquiz.h>

extern    SCANINFO       *AM_getscan();

int
am_openfilescan (openfilenum, booleanexpr, trans_id, lockup, mode, cond)

int         openfilenum;	/* id of the open file */
BOOLEXP     *booleanexpr;  /* pointer to a boolean expression list */
short       trans_id;         
short       lockup;        
LOCKTYPE    mode;
short	    cond;

/* Opens a new scan on the given file.  Only those records that 
      satisfy the boolean expression will be fetched by use of
      the scan ID returned by this routine.
  
   Returns:
      scan id associated with the scan.
     
   Side effects: 
      A new entry is added to the scan table
  
   Errors:
      None
*/

{
    register int        scanid;	/* scanid associated with new scan */
    register SCANINFO	*sptr;	/* address of the scan info record */
    int	e;
    FID fid;

    
#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    {
    	printf("am_openfilescan(openfilenum = %d", openfilenum);
    	printf(", boolean expression = ");
    	AM_dumpboolean(booleanexpr); printf(")\n");
    }
#endif
    
/*
 *  Since a scan is being opened lock the file in the desirable mode.
 */

/*
    The lock modes are used in the following way.  

    lockmode  	Action
    S		lock file in S mode,  set no page locks in doing scan
    X		lock file in X mode,  set no page locks in doing scan
    IS		lock file in IS mode,  lock pages in S mode 
    IX		lock file is IX mode, lock pages in X mode 
    SIX		lock file is SIX mode, lock pages in X mode 
*/
    
/*
    printf("in openfilescan openfilenum = %d\n", openfilenum);
*/
    fid = F_FILEID(openfilenum);
/*
    printf("in openfilescan fid returned form F_FILEID = %d.%d\n", 
	fid.Fvolid, fid.Ffilenum);
*/
    if (lockup)
    {
    	e = lock_file (trans_id, fid, mode, COMMIT, cond) ;
	CHECKERROR(e);
    }

    /* add a new scan to the open scan table, and get the address 
       of that scan info record	    	    */
    scanid = AM_addscan(openfilenum);
    CHECKERROR(scanid);
    sptr = AM_getscan(scanid);
    if (sptr == NULL) return(e3BADSCANID);

    /* fill in the scan info for a sequential file scan */
    sptr->scantype = SEQUENTIAL;
    sptr->boolexp = booleanexpr;
    sptr->trans_id = trans_id;
    sptr->lockup = lockup;
    sptr->cond = cond;

    if (lockup)
    {
        switch (mode) /* switch on lock mode of file */
        {
        case l_S:	
            sptr->lockup = FALSE; /* do not set page locks */
	    sptr->mode = l_NL;
	    break;
        case l_X:	
            sptr->lockup = FALSE; /* do not set page locks */
	    sptr->mode = l_NL;
	    break;
        case l_IS:	
            sptr->lockup = TRUE; /* set page locks */
	    sptr->mode = l_S;
	    break;
        case l_IX:	
            sptr->lockup = TRUE; /* set page locks */
	    sptr->mode = l_X;
	    break;
        case l_SIX:	
            sptr->lockup = TRUE; /* set page locks */
	    sptr->mode = l_X;
	    break;
	case l_NL:
            sptr->lockup = FALSE; /* do not set page locks */
	    sptr->mode = l_NL;
	    break;
	default:
	    printf("Illegal lock mode in wiss_openfilescan\n");
	    return (LM_ILLEGALMODE);
	}
    }
    RIDCLEAR(sptr->rid);
    return(scanid);
} /* am_openfilescan */
