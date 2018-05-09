
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

/* This program (setindex2.c) implements the original o2 set mechanism via 
a B-tree.  It inserts integers "objects" into a b-tree.  The difference
between this program and setindex is that in this program the set
is locked in exclusive mode and the individual elements are inserted
into the b-tree with locking turned off.
*/

/* 
    As elements are added to the set, they are inserted into a B-tree
    which guarantees that the elements in the set  will be unique
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
    int vol, f1;
    int setcount;
    RID	rid;
    int index;
    char *relname;
    int	current;
    long seed;
    int u1;
    TUPLE tuple, *tupPtr;
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

     /* Create an empty data file */

    e = wiss_createfile(vol, relname, 1, 100, 100);
    CHECKERR("load/wiss_createfile", e);

    tupPtr = &tuple;

    f1 = wiss_openfile(vol, relname, WRITE);
    CHECKERR("load/wiss_openfile", f1);

    e = wiss_lock_file(transId, f1, l_X,  COMMIT,  FALSE);
    CHECKERR ("build/wiss_lock_file", e);

    tupPtr->unique1 = 1000000;

    e = wiss_appendfile(f1, tupPtr, sizeof(TUPLE), &rid, transId, TRUE, FALSE);
    CHECKERR("load/wiss_appendfile", e);

    e = wiss_closefile(f1);
    CHECKERR("load/wiss_closefile", e);

    indexNo = 0;
    keyAttr.offset = 0; keyAttr.type = TINTEGER; keyAttr.length = 4;

    /* make sure the index does not already exist */
    (void) wiss_dropindex(vol, relname, indexNo, transId, FALSE, FALSE);

    e = wiss_createindex(vol, relname, indexNo, &keyAttr, 90, 
        TRUE, TRUE, transId, TRUE, FALSE); /* unique but don't sort file */
    CHECKERR("buildidx/wiss_createindex", e);

    printf("next, insert %d entries into the index\n",setcount);
    startTime = time(0);  

    index = wiss_openindex(vol, relname, indexNo, WRITE);
    CHECKERR("openbtree", index);

    /* lock the index in IX mode */
    e = wiss_lock_file(transId, index, l_X,  COMMIT,  FALSE);
    CHECKERR ("build/wiss_lock_file", e);

    current = 1;	/* tuple being generated */

    key.type = TINTEGER; 
    key.length = 4; 

    while (current <= setcount)
    {
    	seed = rand(seed,(long)setcount);  /* generate random number between 
				between 1 and setcount */
	u1 = (int) seed - 1;

	bcopy(&u1, key.value,sizeof(u1));
/*

	printf("key.type =%d, key.length=%d, key.value=%d\n",
		key.type, key.length,u1);
*/

	/* now insert the key into the index */
	e = wiss_insertindex(index, &key, &rid, transId, FALSE, FALSE);
        CHECKERR("st_insertindex", e);

	current++;  /* increment  number of tuples generated */
    }
    e = wiss_closefile(index);
    CHECKERR("load/wiss_closefile", e);

    /* now commit the transaction */
    e = commit_trans(transId);
    if (e != 1) 
	  printf("error status return from commit_trans = %d\n", e);
    else printf("commit ok\n");


    printf(" %d pages and %d keys in %s.%d\n", 
       wiss_indexpages(vol, relname, indexNo),
       wiss_keycard(vol, relname, indexNo), relname, indexNo);

    /* dismount the device */
    e = wiss_dismount(VOLUME);  
    CHECKERR("build/wiss_dismount", e);

    (void) wiss_final();

    if (verboseFlag)
    {
        printf("%d seconds to insert %d keys into index\n", 
		time (0)-startTime, current);  
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
