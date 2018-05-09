
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


#include "benchmark.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <strings.h>
#define	CHECKERR(p,c) if((int)(c)<0) fatalerror(p,(int)(c))

int transId;

main (argc, argv)
int argc;
char *argv[];
{
    int	 j, e, vol, f1, f2;
    RID	rid;
    char    c, *bufptr;
    long    startTime;		/* start time for load */
    long    totalTime;		/* total time for load */
    int	    recSize;

    /* warm up wiss */

    e = wiss_init();			
    CHECKERR("load/wiss_init", e);

    transId = begin_trans();
    printf("new transaction id = %d\n",transId);

    /* mount the volume */
    vol = wiss_mount(VOLUME);	
    CHECKERR("load/wiss_mount", vol);

    /* now commit the transaction */
    e = commit_trans(transId);
    if (e != 1)
      printf("error status return from commit_trans = %d\n", e);
    else printf("commit ok\n");


    /* dismount the volume */
    e = wiss_dismount(VOLUME);  
    CHECKERR("load/wiss_dismount", e);
    printf("dismount completed\n");

    (void) wiss_final();
    printf("final completed\n");

}



fatalerror(p, e)
char *p; int e;
{
   printf("fatal error. first abort the transaction\n");
   wiss_abort_trans(transId);

   /* dismount the device */
   (void) wiss_dismount(VOLUME);  

   wiss_final();  /* clean up processes and shared memory segments */

   wiss_fatalerror(p,(int) e);
}
