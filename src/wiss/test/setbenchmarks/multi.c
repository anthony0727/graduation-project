
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

#define	CHECKERR(p,c,transId) if((int)(c)<0) fatalerror(p,(int)(c),transId)

/************************************************************************

This program is used for multiuser tests of the storage manager.
It begins by loading RELSIZE tuples into a file and then proceeds
to repeatedly scan or reload the table.  The tuple format
is similiar to the wisconsin benchmark relations but only the
unique1 and unique2 attribute values are actually initialized 

*************************************************************************/
 
#define RELSIZE 10000
long random();

static	BOOLEXP	boolexp[] = { {LE, {0, 4, TINTEGER}, NULL, ""}};
extern int procNum;

load_relation (relname, tupCount)
char *relname;  /* name of output relation */
int tupCount;  /* number of tuples in relation */
{
    int	 	i, j, e, vol, f1, f2;
    RID		rid;
    long    	startTime;		/* start time for load */
    long    	totalTime;		/* total time for load */
    TUPLE 	tuple, *tupPtr;
    int 	current;
    int 	transId;

    transId = begin_trans();
    printf("process %d load relation %s with %d tuples. transId=%d\n",
	procNum, relname, tupCount, transId);

    /* mount the volume */
    vol = wiss_mount(VOLUME);	
    CHECKERR("load/wiss_mount", vol, transId);

    /* make sure the relation  does not exist already */
    (void) wiss_destroyfile(vol, relname, transId, FALSE, FALSE);

     /* 
     * Create and load the data file.
     */

    e = wiss_createfile(vol, relname, ((tupCount*sizeof(TUPLE))/PAGESIZE)+1,
		100, 100);
    CHECKERR("load/wiss_createfile", e,transId);

    /* generate tuples */

    current = 1;	/* tuple being generated */
    tupPtr = &tuple;

    startTime = time(0);  
    f1 = wiss_openfile(vol, relname, WRITE);
    CHECKERR("load/wiss_openfile", f1,transId);

    e = wiss_lock_file(transId, f1, l_IX,  COMMIT,  FALSE);
    CHECKERR ("build/wiss_lock_file", e,transId);

    while (current <= tupCount)
    {
	tupPtr->unique1 = current;
	tupPtr->unique2 = random()%tupCount;

	e = wiss_appendfile(f1, tupPtr, sizeof(TUPLE), &rid,
		transId, TRUE, FALSE);
	CHECKERR("load/wiss_appendfile", e,transId);

	if (e < eNOERROR) 
 	{
		printf("%d error return from appendfile\n",e);
		break;
	}

	if ((current % 10000) == 0)
    		printf("Total # of tuples written = %d\n",current);

	current++;  /* increment  number of tuples generated */
    }
    /* now commit the transaction */
    e = commit_trans(transId);
    if (e != 1)
      printf("error status return from commit_trans = %d\n", e);
    else printf("commit ok\n");

    e = wiss_closefile(f1);
    CHECKERR("load/wiss_closefile", e,transId);

    /* dismount the volume */
    e = wiss_dismount(VOLUME);  
    CHECKERR("load/wiss_dismount", e,transId);

   totalTime = time(0) - startTime;
   printf("WISS took %d seconds to load %d records\n", totalTime, current-1);

}

scan_relation(relname, tupCount)
char *relname;
int tupCount;
{
    int		i, e, vol, w;
    int 	upperlimit;  /* number of tuples in result relation */
    int		scanid;
    long    	startTime;		
    long    	totalTime;
    TUPLE	tuple;
    RID		rid;
    KEYINFO	keyattr;
    int 	transId;
	
    keyattr.offset = 0;
    keyattr.length = 4;
    keyattr.type = TINTEGER;

    upperlimit = tupCount+1;

    printf("process %d scan relation %s, upperlimit = %d\n", 
	procNum, relname, upperlimit);

    transId = begin_trans();
    printf("new transaction id = %d\n",transId);

    /* mount the volume */
    vol = wiss_mount(VOLUME);	
    CHECKERR("select/wiss_mount", vol,transId);

    *((int *) boolexp[0].value) = upperlimit;

    startTime = time(0);  
    w = wiss_openfile(vol, relname, READ);
    CHECKERR("select/wiss_openfile", w,transId);

    /* selection (restriction) on the second file */
    scanid = wiss_openfilescan(w, boolexp, transId, TRUE, l_IS, FALSE);
    CHECKERR("select/wiss_openfilescan", scanid,transId);

    e = wiss_fetchfirst(scanid, &rid);
    for (i = 0; e >= eNOERROR; i++)
    {
/*
	e = wiss_readscan(scanid, (char *) &tuple, RECSIZE);
	CHECKERR("select/wiss_readscan", e,transId);
*/
	e = wiss_fetchnext(scanid, &rid);
    }

    e = wiss_closescan(scanid);
    CHECKERR("select/wiss_closescan", e,transId);
    e = wiss_closefile(w);
    CHECKERR("select/wiss_closefile", e,transId);

    /* now commit the transaction */
    e = commit_trans(transId);
    if (e != 1)
      printf("error status return from commit_trans = %d\n", e);
      else printf("commit ok\n");

    /* dismount the volume */
    e = wiss_dismount(VOLUME);  
    CHECKERR("select/wiss_dismount", e,transId);

    totalTime = time(0) - startTime;
    printf("WISS took %d seconds to scan %d records\n", totalTime, i);
}

main (argc, argv)
int argc;
char *argv[];
{
    int 	iterationCnt;  /* number of queries to run */
    char	*relname;  /* name of file */
    int 	i,j, k,e;

    if (argc < 3) {
	printf("usage: multi relname iterationCnt\n");
	exit();
    }

    relname = argv[1];
    iterationCnt = atoi (argv[2]);

    /* warm up wiss */
    e = wiss_init();			
    CHECKERR("load/wiss_init", e,-1);

    /* begin by loading a relation */
    load_relation(relname, RELSIZE);

    for (i=0;i<iterationCnt;i++)
    {

	j = random()%10;  /* pick a random number between 0 and 19 */
	sleep(j); 
	k = random()%10;  /* pick a random number between 0 and 9 */
	/* 70% of the time scan the relation, otherwise load another copy */
	if (k<7) scan_relation(relname, RELSIZE);
    	else load_relation(relname, RELSIZE);
    }
    (void) wiss_final();
    printf("final completed\n");
}

fatalerror(p, e, transId)
char *p; int e; int transId;
{
   printf("fatal error. first abort the transaction\n");
   wiss_abort_trans(transId);

   /* dismount the device */
   (void) wiss_dismount(VOLUME);  

   wiss_final();  /* clean up processes and shared memory segments */

   wiss_fatalerror(p,(int) e);
}

