
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



/* program to build the sun benchmark database */

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

extern int movedtonewpage;

long    random();
time_t time();

#define  MODULUS 100000
#define  NTIMES 100

char    *types[10] = {
    "type000", "type001", "type002",
    "type003", "type004", "type005",
    "type006", "type007", "type008",
    "type009"
};

#define DATE_RANGE	10*365*24*60*60
static  KEY     key = {TINTEGER, "",sizeof(int)};
static  KEYINFO keyattr = { 0, sizeof(int), TINTEGER};

int	trans_id;
 char	*volumeName;

void xdummy(partid, x, y, type)
int partid, x, y; char* type;
{
    /* printf("part=%d x=%d y=%d type=%s\n", partid, x, y, type); */
}

main(argc,argv)
int     argc;
char    **argv;
{
    int 	i,j,k;
    int		vol;  /* wiss volume */
    int 	ofn, hofn, e;
    RID     	curPartRid, toPartRid;
    DATAPAGE	*partDp, *toPartDp;
    PART	*partPtr, *toPartPtr, part;	
    XCURSOR	cursor;
    short   	lockup;
    short   	cond = FALSE;
    int  	starttime, endtime;
    char	*FileName;
    time_t	baseDate;
    int	  	partCnt;     /* number of parts to be added */
    int	  	OnePerCentRange;
    int   	next, slot;
    int		expansionCnt = 0;

    if (argc < 5) {
        printf("usage:  build volume filename partcnt locking_on(TRUE/FALSE)\n");
	exit(1);
    }

    movedtonewpage = 0;

    volumeName = argv[1];
    FileName = argv[2];
    partCnt = atoi(argv[3]);
    printf("building sun benchmark with %d parts, locking=%s\n", partCnt, 
	argv[4]);
    if (!strcmp (argv[4], "TRUE")) lockup = TRUE; else lockup=FALSE;

    wiss_init();

    trans_id = begin_trans();
    printf("new transaction id = %d\n",trans_id);

    vol = wiss_mount(volumeName);  	/* mount the device wiss_disk */
    CHECKERR("build/wiss_mount", vol);
	
    wiss_destroyfile(vol, FileName, trans_id, lockup, cond);
    wiss_dropindex(vol, FileName, HNO, trans_id, lockup, cond);
    starttime = time100();
	
    /* 25 for 4K pages, 50 for 8K pages */
    e = wiss_createfile(vol, FileName, (partCnt/50), 100,90);
    CHECKERR("build/wiss_createfile", e);

    ofn = wiss_openfile(vol, FileName, WRITE);
    CHECKERR("build/wiss_openfile(for insertion)", ofn);

    if (lockup) {
	e = wiss_lock_file(trans_id, ofn, l_IX, COMMIT, cond);
        CHECKERR("build/wiss_lock_file", e);
    }

    OnePerCentRange = (int) partCnt/100;

    /* get base for date */
    baseDate = time((time_t *) NULL);

    for (i=1;i<=partCnt; i++)
    {
/*
	if ((i%1000) == 0) printf("adding part %d\n",i); 
*/
	/* printf("adding part %d\n",i); */

	/* initialize the new part  */
	part.partId = i;
	strcpy(part.type, types[(int) (random()%10)]);
	part.x = (int) (random()%MODULUS);
	part.y = (int) (random()%MODULUS);
	part.date = (long) (baseDate + (random()%DATE_RANGE));
	part.fromCnt = 0;
	part.fromSpace = 5;

	xdummy(part.partId, part.x, part.y, part.type);

	for (j=0;j<3;j++)
	{
	    part.to[j].length = (int) (random()%MODULUS);
	    strcpy(part.to[j].type, types[(int) (random()%10)]);
	}   

	e = wiss_appendfile(ofn, (char *) &part, sizeof(PART), &curPartRid, 
				    trans_id, lockup, cond);
	CHECKERR("build/wiss_appendfile", e);
    }
/*
    ofn = wiss_closefile(vol, FileName, WRITE);
    CHECKERR("build/wiss_openfile(for insertion)", ofn);
*/

    /* now build the index on the parts file */
    e = wiss_createindex(vol, FileName, HNO, &keyattr, 100, TRUE, FALSE,
			    trans_id, lockup, cond);
    CHECKERR("build/wiss_createindex", e);

    printf("loaded all the parts, now start making connections\n");

/*
    ofn = wiss_openfile(vol, FileName, WRITE);
    CHECKERR("build/wiss_openfile(for insertion)", ofn);
*/
    hofn = wiss_openindex(vol, FileName, HNO, WRITE);
    CHECKERR("build/wiss_openindex", hofn);

    if (lockup) {
	e = wiss_lock_file(trans_id, hofn, l_IS, COMMIT, cond);
        CHECKERR("build/wiss_lock_file", e);
    }

    /*  now go through and make the connections between parts */
    for (i=1;i<=partCnt; i++)
    {
/*
	if ((i%1000) == 0) printf("connecting part %d\n",i); 
*/

	bcopy (&i, key.value, sizeof(i));
	e = wiss_getindex(hofn, &key, &cursor, &curPartRid, trans_id, 
		lockup, BT_READ, cond);
	CHECKERR("build/wiss_gethash", e);

	e = wiss_getrecord(ofn, &curPartRid, &partDp, &partPtr, trans_id, 
		lockup, l_X, cond);
	CHECKERR("build/wiss_getrecord", e);

	/* 
	    for each to connection,  pick a part to connect to.
	    90% of the connections are randomly selected among 1%
	    of the parts athat are "closest"
	*/

	for (j=0;j<3;j++)
	{
	    if ( (int)(random()%10) < 9 ) 
	    {
		k = i - (OnePerCentRange/2);
		if (k < 1) k = 1;
		else 
		    if ((k + OnePerCentRange) >= partCnt) 
		    	k = partCnt - OnePerCentRange + 1;
		next = k + (int) (random()%OnePerCentRange);
            }
            else
        	next = (int)((random() % partCnt) + 1);
	    
/*
	    printf("connecting part %d.link %d to part%d\n",i,j,next);
*/
	    if ((next < 1) || (next > partCnt)) 
		printf("error next =%d, i=%d, j=%d\n", next, i, j);

	    if (i != next)
	    {
		/* not connecting a part with itself */
	        /* look up the part in the hash index */
		bcopy (&next, key.value, sizeof(next));
	        e = wiss_getindex(hofn, &key, &cursor, &toPartRid, trans_id, 
			lockup, BT_READ, cond);
	        CHECKERR("build/wiss_gethash", e);

	        partPtr->to[j].part = toPartRid; /* make forward connection */

		/* now do the reverse connection */
		e = wiss_getrecord(ofn, &toPartRid, &toPartDp, &toPartPtr, 
			trans_id, lockup, l_X, cond);
		CHECKERR("build/wiss_getrecord", e);
	        slot = toPartPtr->fromCnt;
		if (slot < toPartPtr->fromSpace)
		{
		    /* sufficient space in reverse connection so there
		    *  is no need to expand the toPart record */
	            toPartPtr->from[slot] = curPartRid;
	            toPartPtr->fromCnt++;
		}
		else
		{
		    /* out of space in toPart from space. must expand it */

		    /* first unpin the toPart record because it may end up */
		    /* getting moved off the curent page */
		    e = bf_unpin(toPartDp, FALSE); /* did not dirty it */
		    CHECKERR("build/bf_unpin", e);
		    e = bf_unpin(partDp, TRUE);
	            CHECKERR("build/bf_unpin", e);

		    /* expansion are done in NUMFROMSLOTS increments */
		    e = wiss_expandrecord(ofn, &toPartRid, EXPANDAMOUNT,
			trans_id, lockup, cond);
		    CHECKERR("build/wiss_expandrecord", e);

		    /* reestablish pointers to both parts */

		    e = wiss_getrecord(ofn, &toPartRid, &toPartDp, &toPartPtr, 
			trans_id, lockup, l_X, cond);
		    CHECKERR("build/wiss_getrecord", e);
		    e = wiss_getrecord(ofn, &curPartRid, &partDp, &partPtr, 
			trans_id, lockup, l_X, cond);
		    CHECKERR("build/wiss_getrecord", e);

		    toPartPtr->fromSpace = toPartPtr->fromSpace + NUMFROMSLOTS;
	            toPartPtr->from[slot] = curPartRid;
	            toPartPtr->fromCnt++;
expansionCnt++;
		}
		e = bf_unpin(toPartDp, TRUE); /* dirtied it */
		CHECKERR("build/bf_unpin", e);
	    }
	    else
	    {
		/* connecting a part with itself. no need to read the part */
	        partPtr->to[j].part = curPartRid; /* make forward connection */
	        slot = partPtr->fromCnt;

		if (slot < partPtr->fromSpace)
		{
		    /* sufficient space in reverse connection so there
		    *  is no need to expand the toPart record */
	            partPtr->from[slot] = curPartRid;
	            partPtr->fromCnt++;
		}
		else
		{
		    /* out of space in Part's from space. must expand it */

		    /* first unpin the part because it may end up */
		    /* getting moved off the curent page */
		    e = bf_unpin(partDp, TRUE);
	            CHECKERR("build/bf_unpin", e);

		    /* expansion are done in NUMFROMSLOTS increments */
		    e = wiss_expandrecord(ofn, &curPartRid, EXPANDAMOUNT,
			trans_id, lockup, cond);
		    CHECKERR("build/wiss_expandrecord", e);

		    /* reestablish pointers to part */
		    e = wiss_getrecord(ofn, &curPartRid, &partDp, &partPtr, 
			trans_id, lockup, l_X, cond);
		    CHECKERR("build/wiss_getrecord", e);

		    partPtr->fromSpace = partPtr->fromSpace + NUMFROMSLOTS;
	            partPtr->from[slot] = curPartRid;
	            partPtr->fromCnt++;
expansionCnt++;
		}
	    }
	}
	e = bf_unpin(partDp, TRUE);
	CHECKERR("build/bf_unpin", e);
    }
    e = wiss_closefile(ofn);
    CHECKERR("build/wiss_closefile", e);
    e = wiss_closefile(hofn);
    CHECKERR("build/wiss_closefile", e);

    e = commit_trans(trans_id);
    if (e != 1) 
      printf("error status return from commit_trans = %d\n", e);
    else printf("commit ok\n");

    printf("Size of b-tree index = %d pages\n",
	wiss_indexpages(vol,FileName,HNO));
    printf("Size of part file = %d pages\n",
	wiss_filepages(vol,FileName));
    printf("had to expand records %d times\n",expansionCnt);
    printf("expansion caused %d records to move to a new page\n",
	movedtonewpage);

    e = wiss_dismount(volumeName);  /* dismount the device wiss_disk */
    CHECKERR("build/st_dismount", e);
    endtime = time100();

    printf ("\tTotal running time for build is %d.%02d seconds\n",
          (endtime - starttime) / 100, (endtime - starttime) % 100);
    fflush(stdout);

    (void) wiss_final();

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

/* =================================================================== */
/* get time in .01 seconds units */
time100 ()
{
    struct timeval tv;
    struct timezone tz;
    
    gettimeofday(&tv,&tz);

    return (tv.tv_sec * 100 + tv.tv_usec / 10000);
}
