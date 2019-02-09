
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
/* Module : am_filelocking
      filenum interface for setting and releasing locks on files

   IMPORTS:
      int	 lock_file()
      int	 m_release_file()
  
   EXPORT:
      int am_lock_file (transId, openfilenum, lockMode, duration, cond);
      int am_m_release_file (transId, openfilenum)
*/

#include <wiss.h>
#include <am.h>


int
wiss_lock_file (transId, openfilenum, lockMode, duration, cond)
int    transId;		/* a transaction identifier */
int    openfilenum;	/* open file # of the data file */
int    lockMode;	/* defined in lockquiz.h */
short  duration;	/* COMMIT or MANUAL */
short  cond;	        /* TRUE or FALSE */

/* locks a file by calling lock_file()
  
   Returns:
      WAIT, ABORTED, GRANTED, NOT_GRANTED, COND_ABORTED, COND_WAIT
     
   Errors:
      None
*/
{
       int e;
       FID fid;

       fid = F_FILEID(openfilenum);

       e = lock_file(transId, fid , lockMode, duration, cond);
       return (e);
}

wiss_m_release_file (transId, openfilenum)
int    transId;		/* a transaction identifier */
int    openfilenum;	/* open file # of the data file */

/* releases the locks on a file 
  
   Returns:
      OK if all went well
     
   Errors:
      None
*/
{
       int e;
       e = m_release_file(transId, F_FILEID(openfilenum));
       return (e);
}
