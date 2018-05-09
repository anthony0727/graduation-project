
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


#include <wiss.h>
#include <wiss_r.h>
#include <lockquiz.h>
#include <locktype.h>
/* error handler */
#define WISSERROR(p,c)  if((int)(c)<0) fatalerror(p,(int)(c))

#include <sys/time.h>
#include <sys/resource.h>

#define NUMPIDS 25000
#define deviceName "hp0a"
int transId;

main( argc, argv )
int argc;
char **argv;
{
	register int i,j,cnt;
	register PID *pidPtr;
	int vol, e, ofn;
	FID fid;
	PID pid;
	int volid;
	int status;
	long    startTime, endTime;

	/* warm up */
	(void) wiss_init();

	transId = begin_trans();
	printf("id of new transaction=%d\n",transId);

	fid.Fvolid = 1983;
	fid.Ffilenum = 1;

	pid.Pvolid = 1983;
	pid.Ppage = 2;

	e = lock_file(transId, fid, l_IS,  COMMIT,  FALSE);
	WISSERROR ("build/wiss_lock_file", e);

	printf("Start by locking %d pages\n", NUMPIDS);
	cnt = NUMPIDS;

	startTime = time(NULL);
	for (i=0;i<cnt;i++, pid.Ppage++)
	{
	    /* lock page i */
	    status = lock_page(transId, fid, pid,l_S, MANUAL, FALSE); 
	    if (status != GRANTED) 
	  	printf("error status return from lock_page=%d\n", status);
	}
	endTime = time(NULL);
	printf("Time to set %d locks = %d seconds\n", cnt,
			endTime - startTime);

	/* now commit the transaction */
	startTime = time(NULL);
	status = commit_trans(transId);
	endTime = time(NULL);
	if (status != OK) 
	  printf("error status return from commit_trans = %d\n", status);
	else printf("commit ok\n");
	printf("Time to commit transaction, releasing %d locks = %d seconds\n",
		cnt, endTime - startTime);
         printf("wiss_final ...\n");
	(void) wiss_final();
}

fatalerror(p, e)
char *p; int e;
{
   int ex;

   printf("Fatal wiss error,  abort the transaction\n");
   wiss_abort_trans(transId);

   /* dismount the device */
   (void) wiss_dismount(deviceName);  

   wiss_final();  /* clean up processes and shared memory segments */

   wiss_fatalerror(p,(int) e);
}
