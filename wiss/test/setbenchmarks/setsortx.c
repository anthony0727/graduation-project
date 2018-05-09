
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


/* This program implements set strategy #3.  */

/* 

    First, the new elements are written to a temporary file (on disk) 
    and then this file is sorted to order the elements of the set by value.
    This sorted file is then scanned using the WiSS scan mechanism and 
    adjacent elements are compared for duplicates.  When duplicates 
    are found,  all but one duplicate is deleted.  As unique values 
    are produced,
    they are inserted into the index used to implement the set.
    Since the values are in sorted order, the buffer requirements of
    this new mechanism consist of one buffer pool page for each
    level of the B-tree.
    Once a leaf page has been filled it can be written to disk
    as no further elements will be added to it.  This mechanism
    has the advantage that the number of disk access is proportional
    to the number of leaf pages that are added to the index and not 
    the number of elements in the set. 

*/


extern int	io_diskreads;
extern int	io_diskwrites;
extern TRACEFLAGS	Trace;

int transId;

long	prime, generator;  

main (argc, argv)
int argc;
char *argv[];
{
    long    startTime;		/* start time for load */
    long    totalTime;		/* total time for load */
    int	    indexNo;
    KEYINFO	keyAttr;
    KEY		key;
    int		index;
    
    int	e;
    int vol, f1;
    int setcount;
    RID	rid;
    char *relname;
    int	current;
    long seed;
    int scanid;
    int	uniqueCount;

    int createstime, createetime;
    int sortstime, sortetime;
    int dupsstime, dupsetime;
    int indexstime, indexetime;

    int key1, key2;
    int		verboseFlag;

    register int i;

    if (argc < 4) {
	printf("usage: relname setcount verboseFlag\n");
	exit();
    }

    relname = argv[1];
    setcount = atoi (argv[2]);  /* number of entries to stick in index/set */
    if (strcmp(argv[3], "TRUE") == 0) verboseFlag = TRUE;
    else verboseFlag = FALSE;

    setGenerator(setcount);  /* initialize prime and generator values */

    seed = generator;

    /* warm up wiss */
    (void) wiss_init();

    transId = begin_trans();
/*
    printf("new transaction id = %d\n",transId);
*/

    /* mount the volume */
    vol = wiss_mount(VOLUME);	
    CHECKERR("load/wiss_mount", vol);

    /* make sure the relation  does not exist already */
    (void) wiss_destroyfile(vol, relname, transId, FALSE, FALSE);

    indexNo=1;
    /* make sure the indices do not already exist */
    (void) wiss_dropindex(vol, relname, indexNo, transId, FALSE, FALSE);

    printf("Insert %d entries into a set\n",setcount);

    startTime = time(0);  
    createstime = startTime;

     /* Create an empty data file */
    e = wiss_createfile(vol, relname, 10, 100, 100);
    CHECKERR("load/wiss_createfile", e);

    /* build an index on the empty relation */

    keyAttr.offset = 0; keyAttr.type = TINTEGER; keyAttr.length = 4;

    /* unique but don't sort file */
    e = wiss_createindex(vol, relname, indexNo, &keyAttr, 95, TRUE, TRUE, 
	transId, TRUE, FALSE); 
    CHECKERR("buildidx/wiss_createindex", e);

    f1 = wiss_openfile(vol, relname, WRITE);
    CHECKERR("load/wiss_openfile", f1);

    e = wiss_lock_file(transId, f1, l_IX,  COMMIT,  FALSE);
    CHECKERR ("build/wiss_lock_file", e);

    current = 1;	/* tuple being generated */
    while (current <= setcount)
    {
    	seed = rand(seed,(long)setcount);  /* generate random number between 
				between 1 and setcount */
	key1 = (int) seed - 1;

	/* now insert the key into the file */
        e = wiss_appendfile(f1, &key1, 4, &rid, transId, TRUE, FALSE);
        CHECKERR("load/wiss_appendfile", e);

	current++;  /* increment  number of tuples generated */
    }
    printf("got through append ok\n");

    e = wiss_closefile(f1);
    CHECKERR("load/wiss_closefile", e);
    createetime = time(0);

    sortstime = time(0);
    /* now sort the file to get duplicates together */
    e=st_sort(vol, relname, &keyAttr, 2, transId, TRUE, FALSE);
    sortetime = time(0);
    printf("got through sort ok\n");

    /* finally scan the file comparing adjacent values */
    /* non-duplicates are inserted into the index */

    dupsstime = time(0);

    f1 = wiss_openfile(vol, relname, READ);
    CHECKERR("load/wiss_openfile", f1);

    scanid = wiss_openfilescan(f1, NULL, transId, TRUE, l_IX, FALSE);
    CHECKERR("select/wiss_openfilescan", scanid);

    index = st_openbtree(vol, relname, indexNo, WRITE);
    CHECKERR("openbtree", index);

    /* lock the index in X mode and don't lock on each insert */
    e = wiss_lock_file(transId, index, l_X,  COMMIT,  FALSE);
    CHECKERR ("build/wiss_lock_file", e);

    key.type = TINTEGER; 
    key.length = 4; 

    uniqueCount = 0;

    e = wiss_fetchfirst(scanid, &rid);
    CHECKERR("load/wiss_fetchfirst", e);

    e = wiss_readscan(scanid, &key1, 4);
    while (e >= eNOERROR)
    {
	e = wiss_fetchnext(scanid, &rid);
	if (e >= eNOERROR)
	{
		e = wiss_readscan(scanid, &key2, 4);
    		CHECKERR("select/wiss_openfilescan", e);

		if (key1 != key2)
		{
		    /* key1 is unique so stick it in the index */
		    /* rid is a dummy value */

		    bcopy(&key1, key.value,sizeof(key1));
		    /* no locking on insertindex call - index is locked in X */
		    e = st_insertindex(index, &key, &rid, transId, FALSE, FALSE);
		    if (e < 0) 
		    {
			printf("got error in st_insertindex with key=%d\n",key);
		    }
        	    CHECKERR("st_insertindex", e);

		    uniqueCount++;
		}
		key1 = key2;
	}
    }

    e = wiss_closescan(scanid);
    CHECKERR("select/wiss_closescan", e);
    e = wiss_closefile(f1);
    CHECKERR("select/wiss_closefile", e);

    e = wiss_closefile(index);
    CHECKERR("load/wiss_closefile", e);

    printf(" %d pages and %d keys in %s.%d\n", 
       	wiss_indexpages(vol, relname, indexNo),
        wiss_keycard(vol, relname, indexNo), relname, indexNo);

    dupsetime = time(0);

    /* now commit the transaction */
    e = commit_trans(transId);
    if (e != 1) 
      printf("error status return from commit_trans = %d\n", e);
    else printf("commit ok\n");

    /* dismount the device */
    e = wiss_dismount(VOLUME);  
    CHECKERR("build/wiss_dismount", e);

    (void) wiss_final();

    if (verboseFlag)
    {
        printf("%d seconds to insert %d keys into set\n", 
		time (0)-startTime, uniqueCount);  
        printf ("%d seconds for creating\n",createetime-createstime);
        printf ("%d seconds for sorting\n",sortetime-sortstime);
        printf ("%d seconds for duplication elimination\n",dupsetime-dupsstime);

    }

}

long rand (seed, limit)
long seed, limit;
{
        do {
		seed = (generator * seed) % prime;
	    } while (seed > limit);
	return (seed);
}

setGenerator(setcount)
{
    /* choose a prime and generator value 
       appropriate for the size of the table.  These values are taken directly
       from the Tandem benchmark generator 
    */

    if (setcount <= 1000) /* 1,000 */
    {
	generator = 279;
	prime = 1009;
    }
    else
    if (setcount <= 10000)  /* 10,000 tuples */
    {
	generator = 2969;
	prime = 10007;
    }
    else
    if (setcount <= 100000) /* 100,000 tuples */
    {
	generator = 21395;
	prime = 100003;
    }
    else
    if (setcount <= 1000000) /* 1 million tuples */
    {
	generator = 2107;
	prime = 1000003;
    }
    else
    if (setcount <= 10000000)  /* 10 million tuples */
    {
	generator = 211;
	prime = 10000019;
    }
    else
    if (setcount <= 100000000)  /* 100 million tuples */
    {
	generator = 21;
	prime = 100000007;
    }
    else
    {
	printf("too many rows requested\n");
	exit();
    }
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
