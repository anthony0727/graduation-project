
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
/* Module : am_openhashscan
  	Opens a hash scan on an opened data file.

   IMPORTS:
  	int	 AM_addscan(OpenFileNum)     
  	SCANINFO *AM_getscan(scanid)
  	int	 st_volid(open_file_number)
  	int	 st_accessmode(open_file_number)
  	int	 st_createfile(volid, filename)
  	int	 st_openfile(volid, filename, mode)
  
   EXPORT:
  	int am_openhashscan(openfilenum, hashfilenum, *KeyAttr, *booleanexpr,
		trans_id, lockup, lockmode, cond)
*/

#include <wiss.h>
#include <am.h>
#include <lockquiz.h>

extern	SCANINFO *AM_getscan();

int
am_openhashscan (openfilenum, hashfilenum, KeyAttr, hashkey, booleanexpr,
	trans_id, lockup, lockmode, cond)

int	openfilenum;	/* open file # of the data file */
int	hashfilenum;	/* open file # of the hash file */
KEYINFO	*KeyAttr;	/* key attribute of the hash file */
KEY	*hashkey;	/* key of the hash scan */
BOOLEXP	*booleanexpr;   /* pointer to a search predicate */
int	trans_id;
short	lockup;
LOCKTYPE lockmode;
short	 cond;

/* Opens a hash scan on the given file. Only those records that satisfy
  	the boolean expression and has a key between the lower and the upper 
  	bound will be fetched by use of the scan ID returned by this routine.
  
   Returns:
  	scan id associated with the scan.
     
   Side effects: 
  	A new entry is added to the scan table.
  	A temporary file is created if the scan is opened for update.
  
   Errors:
  	None
*/
{

    int			volid;	/* volum id of this file */
    register SCANINFO	*sptr;	/* address of the scan info record */
    register int		scanid; /* scanid associated with new scan */
    register int		e;	/* for returned errors */
    char	tname[40];		/* name of the temporary file */
	
#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))
	{
		printf("am_openhashscan(datafile=%d, hashfile=%d, keyattr=",
				 openfilenum, hashfilenum);
		PRINTATTR(KeyAttr);
		printf(", boolean expression ="); AM_dumpboolean(booleanexpr);
		printf(")\n");
	}
#endif

    /* add a new scan to the open scan table */
    scanid = AM_addscan(openfilenum);
    CHECKERROR(scanid);

    /* get the address of the scan info record */
    sptr = AM_getscan(scanid);
    if (sptr == NULL) return(e3BADSCANID);

    /* fill in the scan info for a hash scan */
    sptr->scantype = HASHSCAN;
    sptr->accessflag = st_accessmode(openfilenum);
    sptr->boolexp = booleanexpr;
    sptr->keyattr = KeyAttr;
    sptr->lb = sptr->ub = hashkey;
    sptr->indexfile = hashfilenum;
    sptr->trans_id = trans_id;
    sptr->lockup  = lockup;
    sptr->cond = cond;

    /* clear cursors of the data file and the hash file */
    RIDCLEAR(sptr->rid);
    PIDCLEAR(sptr->cursor.pageid); 

    /*
       The lock modes are used in the following way.

        lockmode    Action

	S       lock file & index in S mode,   set no page locks in doing scan
        XX      lock file & index in X mode,   set no page locks in doing scan
	IS      lock file & index in IS mode,  lock pages in S mode
	IX      lock file & index is IX mode,  lock pages in X mode
	SIX     lock file & index is SIX mode, lock pages in X mode
    */


    if (lockup)
    {
        switch (lockmode) /* switch on lock mode of file */
        {
        case l_S:	
    	    e = lock_file(trans_id, F_FILEID(openfilenum), l_S, COMMIT, cond);
	    CHECKERROR(e);
    	    e = lock_file(trans_id, F_FILEID(hashfilenum), l_S, COMMIT, cond);
	    CHECKERROR(e);
            sptr->lockup = FALSE; /* do not set page locks */
	    sptr->mode = l_NL;
	    break;
        case l_X:	
    	    e = lock_file(trans_id, F_FILEID(openfilenum), l_X, COMMIT, cond);
	    CHECKERROR(e);
    	    e = lock_file(trans_id, F_FILEID(hashfilenum), l_X, COMMIT, cond);
	    CHECKERROR(e);
            sptr->lockup = FALSE; /* do not set page locks */
	    sptr->mode = l_NL; 
	    break;
        case l_IS:	
    	    e = lock_file(trans_id, F_FILEID(openfilenum), l_IS, COMMIT, cond);
	    CHECKERROR(e);
    	    e = lock_file(trans_id, F_FILEID(hashfilenum), l_IS, COMMIT, cond);
	    CHECKERROR(e);
            sptr->lockup = TRUE; /* set page locks */
	    sptr->mode = l_S; /* page lock mode */
	    break;
        case l_IX:	
    	    e = lock_file(trans_id, F_FILEID(openfilenum), l_IX, COMMIT, cond);
	    CHECKERROR(e);
    	    e = lock_file(trans_id, F_FILEID(hashfilenum), l_IX, COMMIT, cond);
	    CHECKERROR(e);
            sptr->lockup = TRUE; /* set page locks */
	    sptr->mode = l_X; /* page lock mode */
	    break;
        case l_SIX:	
    	    e = lock_file(trans_id, F_FILEID(openfilenum), l_SIX, COMMIT, cond);
	    CHECKERROR(e);
    	    e = lock_file(trans_id, F_FILEID(hashfilenum), l_SIX,COMMIT, cond);
	    CHECKERROR(e);
            sptr->lockup = TRUE; /* set page locks */
	    sptr->mode = l_X; /* page lock mode */
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

    /* 
       check access mode of the hash file, if it is different from
       the mode of the data file, then only READ access is allowed
       on this scan. (we are more conservative here)		
    */

    if (st_accessmode(hashfilenum) != sptr->accessflag)
	sptr->accessflag = READ;

    /* create a temporary update (log) file for update scan */
    if (sptr->accessflag == WRITE)
    {
	suffixname(tname, "tmp", 't', scanid);
	volid = st_volid(openfilenum);
	CHECKERROR(volid);
	e = st_createfile(volid, tname, 1, 100, 100);
	CHECKERROR(e);
	sptr->deltafile = st_openfile(volid, tname, WRITE);
	CHECKERROR(sptr->deltafile);
    }

    return(scanid);

} /* am_openhashscan */
