
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



/* program to traverse the sun benchmark database */

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

static  KEY     key = {TINTEGER, "", sizeof(int)};
static  KEYINFO keyattr = { 0, sizeof(int), TINTEGER};

int	trans_id;
char	*volumeName;

/* the following 3 variables are globals to avoid passing them */
/* repeatedly in ftraverse() and rtraverse() - gross!!! */
int	ofn;
short  	lockup;		
short  	cond = FALSE;

void xdummy(partid, x, y, type)
int partid, x, y; char* type;
{
/*
     printf("part=%d x=%d y=%d type=%s\n", partid, x, y, type); 
*/
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
    int 	hofn, e;
    RID     	partRid;
    PART	part;
    XCURSOR	cursor;
    int  	starttime, endtime;
    float   	total, sec, fract, cum_time=0.0;
    int   	partId;
    int	  	dbSize;	   /* number of parts in the db */
    int	  	depth;     /* depth of traversal */
    int	  	repeatCnt; /* do it this many times */
    int		partCnt;   /* number of parts visited on traversal */
    char	*FileName;
    int		tmp;

    if (argc < 7) {
       printf("usage: traverse volume filename dbSize depth repeatCnt locking_on(TRUE/FALSE)\n");
	exit(1);
    }
    volumeName = argv[1];
    FileName = argv[2];
    dbSize = atoi(argv[3]);
    depth = atoi(argv[4]);
    repeatCnt = atoi(argv[5]);
    if (!strcmp (argv[6], "TRUE")) lockup = TRUE; else lockup=FALSE;

    printf("traverse to depth %d in database containing %d parts\n",
	depth, dbSize);
    printf("locking on = %s, repeat %d times\n", argv[6], repeatCnt);

    wiss_init();

    trans_id = begin_trans();
    printf("new transaction id = %d\n",trans_id);

    vol = wiss_mount(volumeName);  	/* mount the device wiss_disk */
    CHECKERR("build/wiss_mount", vol);
	
    ofn = wiss_openfile(vol, FileName, READ);
    CHECKERR("build/wiss_openfile(for insertion)", ofn);

    hofn = wiss_openhash(vol, FileName, HNO, READ);
    CHECKERR("build/wiss_openhash", hofn);

    if (lockup) {
	e = wiss_lock_file(trans_id, ofn, l_IS, COMMIT, cond);
        CHECKERR("build/wiss_lock_file", e);
	e = wiss_lock_file(trans_id, hofn, l_IS, COMMIT, cond);
        CHECKERR("build/wiss_lock_file", e);
    }

    /* first do it in the forward direction */
    printf("in the forward direction first\n");
    total=0.0; sec=0.0; fract=0.0; cum_time =0.0;
    for (i=0;i<repeatCnt; i++)
    {
        starttime = time100();

	/* compute which part to start with */
	/* generate a random part id */
	tmp = ((int) (random()%dbSize)) + 1; 
	bcopy(&tmp, key.value, sizeof(tmp));

	/* get rid of part with this part number out of the hash index */
	e = wiss_gethash(hofn, &key, &cursor, &partRid, trans_id, 
			lockup, cond);
        CHECKERR("build/wiss_get_hash", e);

	partCnt = ftraverse(partRid, depth);  /* do the transitive closure */
        endtime = time100();

	printf("\tVisited %d parts \n",partCnt);
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
    }
    printf ("\t\t\tAvg time per warm run in the forward direction: %5.2f\n", 
	cum_time/(float)(repeatCnt-1));

    /* next do it in the forward direction */
    printf("in the reverse direction next\n");
    total=0.0; sec=0.0; fract=0.0; cum_time =0.0;
    for (i=0;i<repeatCnt; i++)
    {
        starttime = time100();

	/* compute which part to start with */
	/* generate a random part id */
	tmp = ((int) (random()%dbSize)) + 1; 
	bcopy(&tmp, key.value, sizeof(tmp));

	/* get rid of part with this part number out of the hash index */
	e = wiss_gethash(hofn, &key, &cursor, &partRid, trans_id, 
			lockup, cond);
        CHECKERR("build/wiss_get_hash", e);

	partCnt = rtraverse(partRid, depth);  /* do the transitive closure */
        endtime = time100();

	printf("\tVisited %d parts \n",partCnt);
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
    }
    printf ("\t\t\tAvg time per warm run in the reverse direction: %5.2f\n", 
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
   wiss_final(); /* clean up shared memory and semaphore */

   wiss_fatalerror(p,(int) e);
}


/* recursive procedure for forward traversals  */
int ftraverse (partRid, depth)
RID	partRid;
int	depth;
{
    register int i;
    int 	cnt,e;
    PART	*partPtr;
    DATAPAGE	*partDp;
    RID		to[3];

    e = wiss_getrecord(ofn, &partRid, &partDp, &partPtr, trans_id,
		lockup, l_S, cond);
    CHECKERR("build/wiss_getrecord", e);

    /* begin by calling the dummy procedure on the current part */
    xdummy(partPtr->partId, partPtr->x, partPtr->y, partPtr->type);

    for (i=0;i<3;i++) to[i] = partPtr->to[i].part;

    e = bf_unpin(partDp, FALSE);
    CHECKERR("build/bf_unpin", e);

    cnt = 1;
    if (depth > 0) 
    {
	for (i=0;i<3;i++) 
	    cnt += ftraverse(to[i], depth-1);
    }
    return (cnt);
}

/* recursive procedure for reverse traversals */
int rtraverse (partRid, depth)
RID	partRid;
int	depth;
{
    register int i, fromCnt;
    PART	part;
    int cnt, e;
    PART	*partPtr;
    DATAPAGE	*partDp;
    RID		from[50];

    e = wiss_getrecord(ofn, &partRid, &partDp, &partPtr, trans_id,
		lockup, l_S, cond);
    CHECKERR("build/wiss_getrecord", e);

    /* begin by calling the dummy procedure on the current part */
    xdummy(partPtr->partId, partPtr->x, partPtr->y, partPtr->type);

    fromCnt = partPtr->fromCnt;
    for (i=0;i<fromCnt;i++) from[i] = partPtr->from[i];

    e = bf_unpin(partDp, FALSE);
    CHECKERR("build/bf_unpin", e);

    cnt = 1;
    if (depth > 0) 
    {
	for (i=0;i<fromCnt;i++) 
	    cnt += rtraverse(from[i], depth-1);
    }
    return (cnt);
}

