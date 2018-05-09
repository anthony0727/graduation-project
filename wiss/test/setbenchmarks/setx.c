
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

extern int	io_diskreads;
extern int	io_diskwrites;
extern TRACEFLAGS	Trace;

#define	CHECKERR(p,c) if((int)(c)<0) fatalerror(p,(int)(c))
int transId;

/* This program implements set strategy #4 */

/*
    First, the new elements are written
    to a temporary file (on disk) and then this file is sorted to
    order the elements of the set by value.  This sorted file is then
    scanned using the WiSS scan mechanism and adjacent elements
    are compared for duplicates.  When duplicates are found,  all
    but one duplicate is deleted.  As unique values are produced,
    they are inserted into the result file.  No indices are used.
*/


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
    
    int	e;
    int vol, f1, f2;
    int setcount;
    RID	rid, ridf2;
    int index;
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

    int i;
    indexNo=1;

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

    /* warm up */
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

    /* make sure the relation  does not exist already */
    (void) wiss_destroyfile(vol, "tmp", transId, FALSE, FALSE);

    /* make sure the index  does not exist already */
    wiss_dropindex(vol, relname, indexNo, transId, FALSE, FALSE);

    printf("Insert %d entries into a set\n",setcount);
    createstime = time(0);

     /* Create an empty data file */
    e = wiss_createfile(vol, "tmp", 10, 100, 100);
    CHECKERR("load/wiss_createfile", e);

    startTime = time(0);  
    f1 = wiss_openfile(vol, "tmp", WRITE);
    CHECKERR("load/wiss_openfile", f1);

    e = wiss_lock_file(transId, f1, l_IX,  COMMIT,  FALSE);
    CHECKERR ("build/wiss_lock_file", e);

    keyAttr.offset = 0; keyAttr.type = TINTEGER; keyAttr.length = 4;

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
    e = wiss_closefile(f1);
    CHECKERR("load/wiss_closefile", e);
    createetime = time(0);

    sortstime = time(0);
    /* now sort the file to get duplicates together */
    e=st_sort(vol, "tmp", &keyAttr, 2, transId, TRUE, FALSE);
    sortetime = time(0);

    /* finally scan the file comparing adjacent values */
    /* non-duplicates are written to relname */

    dupsstime = time(0);

    f1 = wiss_openfile(vol, "tmp", READ);
    CHECKERR("load/wiss_openfile", f1);

     /* Create an empty data file to hold the final set */
    e = wiss_createfile(vol, relname, 10, 100, 100);
    CHECKERR("load/wiss_createfile", e);

    f2 = wiss_openfile(vol, relname, WRITE);
    CHECKERR("load/wiss_openfile", f1);

    scanid = wiss_openfilescan(f1, NULL, transId, TRUE, l_IX, FALSE);
    CHECKERR("select/wiss_openfilescan", scanid);

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
		    /* key1 is unique so append it */
        	    e = wiss_appendfile(f2, &key1, 4, &ridf2, transId, TRUE,
			FALSE);
        	    CHECKERR("load/wiss_appendfile", e);
		    uniqueCount++;
		}
		key1 = key2;
	}
    }
    /* append the last key */
    e = wiss_appendfile(f2, &key1, 4, &ridf2, transId, TRUE, FALSE);
    CHECKERR("load/wiss_appendfile", e);
    uniqueCount++;

    e = wiss_closescan(scanid);
    CHECKERR("select/wiss_closescan", e);
    e = wiss_closefile(f1);
    CHECKERR("select/wiss_closefile", e);
    e = wiss_closefile(f2);
    CHECKERR("select/wiss_closefile", e);
    dupsetime = time(0);

     /* build an index on the relname */
    keyAttr.offset = 0; keyAttr.type = TINTEGER; keyAttr.length = 4;

    indexstime = time(0);
     /* unique but don't sort file */
    e = wiss_createindex(vol, relname, indexNo, &keyAttr, 95, TRUE, TRUE,
	transId, TRUE, FALSE);
    CHECKERR("buildidx/wiss_createindex", e);
    indexetime = time(0);


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
        printf ("%d seconds to create index \n",indexetime-indexstime);
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
