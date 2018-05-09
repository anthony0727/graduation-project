
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



/* program to insert 100 parts the sun benchmark database */

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

long    random();
time_t time();
int	time100();

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
char 	*volumeName;

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
    float   	total, sec, fract, cum_time=0.0;
    int	  	newPartNo;
    int	  	repeatCnt;   /* do it this many times */
    int		dbSize, origDbSize;
    char	*FileName;

    time_t	baseDate;
    int	  partCnt;     /* number of parts to be added */
    int	  OnePerCentRange;
    int   base, next, slot;

    if (argc < 7) {
        printf("usage: insert volume filename dbSize partcnt repeatCnt locking_on(TRUE/FALSE) \n");
	exit(1);
    }

    volumeName = argv[1];
    FileName = argv[2];
    dbSize = atoi(argv[3]);
    partCnt = atoi(argv[4]);
    repeatCnt = atoi(argv[5]);
    if (!strcmp (argv[6], "TRUE")) lockup = TRUE; else lockup=FALSE;

    printf("insert %d parts from sun benchmark database containing %d parts\n",
	partCnt, dbSize);
    printf("locking=%s, repeat %d times\n", argv[6], repeatCnt);
    printf("after each insert, the inserted parts are then deleted\n");

    wiss_init();

    trans_id = begin_trans();
    printf("new transaction id = %d\n",trans_id);

    vol = wiss_mount(volumeName);  	/* mount the device wiss_disk */
    CHECKERR("build/wiss_mount", vol);
	
    ofn = wiss_openfile(vol, FileName, WRITE);
    CHECKERR("build/wiss_openfile(for insertion)", ofn);

    hofn = wiss_openhash(vol, FileName, HNO, WRITE);
    CHECKERR("build/wiss_openhash", hofn);

    if (lockup) {
	e = wiss_lock_file(trans_id, ofn, l_IX, COMMIT, cond);
        CHECKERR("build/wiss_lock_file", e);
	e = wiss_lock_file(trans_id, hofn, l_IX, COMMIT, cond);
        CHECKERR("build/wiss_lock_file", e);
    }

    OnePerCentRange = (int) dbSize/100;

    /* get base for date */
    baseDate = time((time_t *) NULL);
    origDbSize = dbSize;

    for (k=0;k<repeatCnt; k++, dbSize = dbSize+partCnt)
    {
        starttime = time100();
	newPartNo = dbSize+1;  /* starting part number of new parts */
        for (i=0;i<partCnt; i++, newPartNo++)
        {
	    /* initialize the new part  */
	    part.partId = newPartNo;
	    strcpy(part.type, types[(int) (random()%10)]);
	    part.x = (int) (random()%MODULUS);
	    part.y = (int) (random()%MODULUS);
	    part.date = (long) (baseDate + (random()%DATE_RANGE));
	    part.fromCnt = 0;

    	    xdummy(part.partId, part.x, part.y, part.type);

	    for (j=0;j<3;j++)
	    {
	        part.to[j].length = (int) (random()%MODULUS);
	        strcpy(part.to[j].type, types[(int) (random()%10)]);
	    }   

	    e = wiss_appendfile(ofn, (char *) &part, sizeof(PART), 
		&curPartRid, trans_id, lockup, cond);
    	    CHECKERR("build/wiss_appendfile", e);

	    bcopy(&part.partId, key.value, sizeof(part.partId));
	    e = wiss_inserthash(hofn, &key, &curPartRid, trans_id, 
		lockup, cond);
	    CHECKERR("build/wiss_inserthash", e);
        }

        /*  now go through and make the connections between parts */
	newPartNo = dbSize+1;  /* starting part number of new parts */
        for (i=0;i<partCnt; i++, newPartNo++)
        {
	    bcopy(&newPartNo, key.value, sizeof(newPartNo));
	    e = wiss_gethash(hofn, &key, &cursor, &curPartRid, trans_id, 
		lockup, cond);
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
		    base = newPartNo - (OnePerCentRange/2);
		    if (base < 1) base = 1;
		    else 
		    if ((base + OnePerCentRange) >= (dbSize+partCnt)) 
		    	base = (dbSize+partCnt) - OnePerCentRange + 1;
		    next = base + (int) (random()%OnePerCentRange);
                }
                else
        	    next = (int)((random() % (dbSize+partCnt)) + 1);

	        if (newPartNo != next)
	        {
		    /* not connecting a part with itself */
	            /* look up the part in the hash index */
		    bcopy(&next, key.value, sizeof(next));
	            e = wiss_gethash(hofn, &key, &cursor, &toPartRid, 
			trans_id, lockup, cond);
	            CHECKERR("build/wiss_gethash", e);

		    /* make forward connection */
	            partPtr->to[j].part = toPartRid; 

		    /* now do the reverse connection */
		    e = wiss_getrecord(ofn, &toPartRid, &toPartDp, &toPartPtr, 
			trans_id, lockup, l_X, cond);
		    CHECKERR("build/wiss_getrecord", e);
	            slot = toPartPtr->fromCnt;
/*
	            printf("slot=%d fromCnt = %d\n",slot, toPart->fromCnt);
*/
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

		        e = wiss_getrecord(ofn, &toPartRid, &toPartDp, 
				&toPartPtr, trans_id, lockup, l_X, cond);
		        CHECKERR("build/wiss_getrecord", e);
		        e = wiss_getrecord(ofn, &curPartRid, &partDp, &partPtr, 
			    trans_id, lockup, l_X, cond);
		        CHECKERR("build/wiss_getrecord", e);

		        toPartPtr->fromSpace = toPartPtr->fromSpace + NUMFROMSLOTS;
	                toPartPtr->from[slot] = curPartRid;
	                toPartPtr->fromCnt++;
		    }
		    e = bf_unpin(toPartDp, TRUE); /* dirtied it */
		    CHECKERR("build/bf_unpin", e);
	        }
	        else
	        {
		    /* connect a part with itself. no need to read the part */
	            partPtr->to[j].part = curPartRid; 
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
		    }
	        }
	    }
	    e = bf_unpin(partDp, TRUE);
	    CHECKERR("build/bf_unpin", e);
        }
        endtime = time100();

        printf ("\tTotal running time for loop %d is %d.%02d seconds\n",
          k+1, (endtime - starttime) / 100, (endtime - starttime) % 100);
	fflush(stdout);

        /* Take cumulative totals */
        sec        = (float) ((endtime - starttime)/100);
        fract      = (float) ((endtime - starttime)%100)/100.;
        total      = sec + fract;
        if ( k != 0 ) cum_time       += total;


    }
    printf ("\t\t\tAvg time per warm run: %5.2f\n", 
	cum_time/(float)(repeatCnt-1));

    /* check db consistency before the delete */
/*
    dbCheck(origDbSize+(partCnt*repeatCnt), ofn, hofn, trans_id, 
	lockup, cond);
*/
    
    /* finally go through and deleted the newly inserted parts */
    printf("attempting to delete all newly inserted parts \n");

    deleteParts(origDbSize+1, partCnt*repeatCnt, ofn, hofn, trans_id, 
	lockup, cond);

    /* check db consistency after the delete */
/*
    dbCheck(origDbSize, ofn, hofn, trans_id, lockup, cond);
*/

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


/* 
*  The following delete is not generic.  In particular it
*  will only safely delete the parts add by the insert and
*  not arbitrary parts from the database
*/

deleteParts(firstPartId, partCnt, ofn, hofn, trans_id, lockup, cond)
int	firstPartId, partCnt, ofn, hofn, trans_id, lockup, cond;
{
    int     	i,j, e, partId;
    RID     	partRid, toPartRid;
    PART	*partPtr, *toPartPtr;
    DATAPAGE	*partDp, *toPartDp;
    XCURSOR	cursor;

    /* first go through and delete from references to the deleted parts */

    partId = firstPartId;
    for (i=0;i<partCnt;i++, partId++)
    {
	bcopy(&partId, key.value, sizeof(partId));
	e = wiss_gethash(hofn, &key, &cursor, &partRid, trans_id, lockup, 
		cond);
        CHECKERR("build/wiss_gethash", e);

	e = wiss_getrecord(ofn, &partRid, &partDp, &partPtr, trans_id,
		lockup, l_X, cond);
        CHECKERR("build/wiss_getrecord", e);

        /*
           For each part pointed to by the current part delete its 
           from pointer back to partRid.  Note!!  This is not
           at all correct or safe but works because we are going
           to delete all the parts inserted above.  It may be the
           case that if two new parts i and j both point at the same
           part k, that the following call will actually delete the
           from pointer from k to j and not the one that goes from k to i.
           THis is safe only because we know that we are going to delete
           both i and j eventually.  A hack, yes.
        */

        for (j=0;j<3;j++) 
	{
	    toPartRid = partPtr->to[j].part;
	    e = wiss_getrecord(ofn, &toPartRid, &toPartDp, &toPartPtr, 
		trans_id, lockup, l_X, cond);
            CHECKERR("build/wiss_getrecord", e);

	    toPartPtr->fromCnt--;

	    e = bf_unpin(toPartDp, TRUE);  /* dirtied the page */
	    CHECKERR("build/bf_unpin", e);
        }
	e = bf_unpin(partDp, FALSE);  /* did not dirty the part */
		/* well we may have if a part referenced itself but */
		/* this case is handled by the previous unpin */
	CHECKERR("build/bf_unpin", e);
    }
    partId = firstPartId;
    for (i=0;i<partCnt;i++, partId++)
    {
	bcopy(&partId, key.value, sizeof(partId));
	/* get the rid of the part into partRid */
	e = wiss_gethash(hofn, &key, &cursor, &partRid, trans_id, lockup,
		cond);
        CHECKERR("build/wiss_get_hash", e);

        /* now remove the part from the hash index */
	e = wiss_deletehash(hofn, &key, &partRid, trans_id, lockup, cond);
        CHECKERR("build/wiss_deletehash", e);
	
	e = wiss_deleterecord(ofn, &partRid, trans_id, lockup, cond);
        CHECKERR("build/wiss_deleterecord", e);

    }
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

/* 
*  The following program checks the consistency of a database 
*/

dbCheck(dbSize, ofn, hofn, trans_id, lockup, cond)
int	dbSize,  ofn, hofn, trans_id, lockup, cond;
{
    int     	i,j,k, e, partId, found;
    RID     	partRid, toPartRid;
    PART	*partPtr, *toPartPtr;
    DATAPAGE	*partDp, *toPartDp;
    XCURSOR	cursor;


    partId = 1;
    for (i=0;i<dbSize;i++, partId++)
    {
	bcopy(&partId, key.value, sizeof(partId));
	e = wiss_gethash(hofn, &key, &cursor, &partRid, trans_id, lockup, 
		cond);
        CHECKERR("build/wiss_gethash", e);

	e = wiss_getrecord(ofn, &partRid, &partDp, &partPtr, trans_id,
		lockup, l_X, cond);
        CHECKERR("build/wiss_getrecord", e);

	if (partPtr->partId != partId) printf("bad partId = %d should be %d\n",
		partPtr->partId, partId);

        for (j=0;j<3;j++) 
	{
	    toPartRid = partPtr->to[j].part;
	    e = wiss_getrecord(ofn, &toPartRid, &toPartDp, &toPartPtr, 
		trans_id, lockup, l_X, cond);
            CHECKERR("build/wiss_getrecord", e);

	    /* now check the backwards connections for a pointer to partId */
	    found = FALSE;
	    k = 0;

	    while ((found == FALSE) && (k < toPartPtr->fromCnt))
	    {
		if (RIDEQ(toPartPtr->from[k],partRid)) found = TRUE;
		k++;
	    }
	    if (found == FALSE)
	    {
		printf("error in part db.  part %d points to part %d\n",
			partId, toPartPtr->partId);
		printf("\tbut there is no reverse connection\n");
	    }
	    e = bf_unpin(toPartDp, TRUE);  /* dirtied the page */
	    CHECKERR("build/bf_unpin", e);
        }
	e = bf_unpin(partDp, FALSE);  /* did not dirty the part */
	CHECKERR("build/bf_unpin", e);
    }
}
