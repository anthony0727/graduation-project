
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



/* program to lookup a set of random parts in the sun benchmark database */

#include <stdio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <wiss_r.h>
#include <wiss.h>
#include <st.h>
#include <lockquiz.h>
#include "data.h"

#define CHECKERR(p,c) if((int)(c)<0) fatalerror(p,(int)(c))

static  KEY     key = {TINTEGER, "",sizeof(int)};
static  KEYINFO keyattr = { 0, sizeof(int), TINTEGER};

int	trans_id;
char	*volumeName;



void xdummy(partid, x, y, type)
int partid, x, y; char* type;
{
    /* printf("part=%d x=%d y=%d type=%s\n", partid, x, y, type); */
}

long    random();
int	time100();
int 	gettimeofday();

main(argc,argv)
int     argc;
char    **argv;
{
    int 	i,j,k;
    int		vol;  /* wiss volume */
    int 	ofn, hofn, e;
    RID     	partRid;
    PART	*partPtr;
    DATAPAGE	*partDp;
    XCURSOR	cursor;
    short   	lockup;
    short   	cond = FALSE;
    int  	starttime, endtime;
    float   	total, sec, fract, cum_time=0.0;
    int   	partId;
    int	  	dbSize;	 /* number of parts in the db */
    int	  	partCnt;    /*  number of parts to be retrieved */
    int	  	repeatCnt;   /*  do it this many times */
    char	*FileName;

    if (argc < 7) {
       printf("usage: lookup volume filename dbSize partCnt repeatCnt locking_on(TRUE/FALSE\n");
	exit(1);
    }
    volumeName = argv[1];
    FileName = argv[2];
    dbSize = atoi(argv[3]);
    partCnt = atoi(argv[4]);
    repeatCnt = atoi(argv[5]);
    if (!strcmp (argv[6], "TRUE")) lockup = TRUE; else lockup=FALSE;

    printf("lookup %d parts from sun benchmark database containing %d parts\n",
	partCnt, dbSize);
    printf("locking on = %s, repeat %d times\n", argv[6], repeatCnt);

    wiss_init();

    trans_id = begin_trans();
    printf("new transaction id = %d\n",trans_id);

    vol = wiss_mount(volumeName);  	/* mount the device wiss_disk */
    CHECKERR("build/wiss_mount", vol);
	
    ofn = wiss_openfile(vol, FileName, READ);
    CHECKERR("build/wiss_openfile(for insertion)", ofn);

    hofn = wiss_openindex(vol, FileName, HNO, READ);
    CHECKERR("build/wiss_openindexhash", hofn);

    if (lockup) {
	e = wiss_lock_file(trans_id, ofn, l_IS, COMMIT, cond);
        CHECKERR("build/wiss_lock_file", e);
	e = wiss_lock_file(trans_id, hofn, l_IS, COMMIT, cond);
        CHECKERR("build/wiss_lock_file", e);
    }

    starttime = time100();
    for (i=0;i<repeatCnt; i++)
    {
        for (j=0;j<partCnt; j++)
        {
	    /* generate a random part id */
	    partId = ((int) (random()%dbSize)) + 1; 
	    bcopy (&partId, key.value, sizeof(partId));

	    e = wiss_getindex(hofn, &key, &cursor, &partRid, trans_id, 
			lockup, BT_READ, cond);
            CHECKERR("build/wiss_getindex", e);

	    e = wiss_getrecord(ofn, &partRid, &partDp, &partPtr, trans_id,
		lockup, l_S, cond);
	    CHECKERR("build/wiss_getrecord", e);

/*
	    if (partPtr->partId != partId) 
		printf ("partId was %d but part.partId is %d\n", partId,
			partPtr->partId);
*/

	    /* now call the dummy procedure */
	    xdummy(partPtr->partId, partPtr->x, partPtr->y, partPtr->type);

	    e = bf_unpin(partDp, FALSE);
	    CHECKERR("build/bf_unpin", e);
	}
        endtime = time100();

        printf ("\tTotal running time for loop %d is %d.%02d seconds\n",
          i+1,
          (endtime - starttime) / 100,
          (endtime - starttime) % 100);
	  fflush(stdout);

          /* Take cumulative totals */
          sec        = (float) ((endtime - starttime)/100);
          fract      = (float) ((endtime - starttime)%100)/100.;
          total      = sec + fract;
          if ( i != 0 ) cum_time       += total;

          starttime = time100();
    }
    printf ("\t\t\tAvg time per warm run: %5.2f\n", 
	cum_time/(float)(repeatCnt-1));

    e = wiss_closefile(ofn);
    CHECKERR("build/wiss_closefile", e);
    e = wiss_closefile(hofn);
    CHECKERR("build/wiss_closefile", e);

    e = commit_trans(trans_id);
    if (e != 1) 
      printf("error status return from commit_trans = %d\n", e);
    else printf("commit ok\n");

    e = wiss_dismount(volumeName);  /* dismount the device wiss_disk */
    CHECKERR("build/st_dismount", e);

    (void) wiss_final();

}


/* =================================================================== */
/* get time in .01 seconds units */
time100 ()
{
    struct timeval tv;
    struct timezone tz;
    
    gettimeofday(&tv,&tz);

    return (tv.tv_sec * 100 + tv.tv_usec / 10000);
}

fatalerror(p, e)
char *p; int e;
{
   int ex;

   printf("Fatal wiss error,  abort the transaction\n");
   wiss_abort_trans(trans_id);

   /* dismount the device */
   (void) wiss_dismount(volumeName);  
   wiss_final();	/* clean up shared memory and semaphore */

   wiss_fatalerror(p,(int) e);
}
