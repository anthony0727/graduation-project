
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


#include	"benchmark.h"
#include <sys/time.h>
#define	CHECKERR(p,c) if((int)(c)<0) fatalerror(p,(int)(c))

/* this program simply scans an entire relation, reading each
tuple into a local buffer.  The program takes three parameters:

1) the name of the relation

2) the upper limit to be applied to the unique1 attribute

3) a mode switch with values 0, 1, 2, 3, 4. With mode=0,
the qualifying record is left directly in the wiss buffer pool
and is not copied into the users address space.  With mode=1
the qualifying record is copied from the wiss buffer pool
into the address space of the user.  With mode=2,  the
record is copied twice.  Once from the wiss buffer pool
into the user's address space and then once again - to simulate
the copy into the o2 object space.  In mode=3,  the fixed length
fields of the record are bcopied in one shot but the string
fields are bcopied one at a time.  In this mode, the same space
is used repeatedly so a malloc is done only once.  Mode=4 operates
the same as mode equal to 3 but the space for each record and
string is malloced repeatedly.

*/

static	BOOLEXP	boolexp[] = { {LE, {0, 4, TINTEGER}, NULL, ""}};

typedef struct 
	{ 
		int 	unique1;
		int 	unique2;
		int 	two;
		int 	four;
		int	ten;
		int	twenty;
		int	onePercent;
		int	tenPercent;
		int	twentyPercent;
		int	fiftyPercent;
		int	unique3;
		int	oddOnePercent;
		int	evenOnePercent;
	 	char	*string1Ptr;
	 	char	*string2Ptr;
	 	char	*string3Ptr;
	} FIXEDTUP;

#define FIXEDSIZE sizeof(FIXEDTUP)

#define STRINGSIZE 52

int transId;

main(argc, argv)
int	argc;	
char	**argv;
{
    int	i, e, vol, w;
    int 	upperlimit;  /* number of tuples in result relation */
    char	*relname;  /* name of output file */
    int	scanid;
    long    startTime;		
    long    totalTime;
    int	copymode;

    TUPLE	tuple;
    char	recbuf2[RECSIZE];
    RID	rid;
    KEYINFO	keyattr;

    FIXEDTUP  fixedtup, *fptr;  /* fixed length tuple fields plus 
					string ptrs */
    char	xstring1[STRINGSIZE];
    char	xstring2[STRINGSIZE];
    char	xstring3[STRINGSIZE];
    char 	*p;
    int		verboseFlag;

    keyattr.offset = 0;
    keyattr.length = 4;
    keyattr.type = TINTEGER;

    if (argc < 5) {
	printf("usage: relname upperlimit copymode verboseFlag\n");
	exit();
    }

    relname = argv[1];
    upperlimit = atoi (argv[2]);
    copymode = atoi (argv[3]);
    if (strcmp(argv[4], "TRUE") == 0) verboseFlag = TRUE;
    else verboseFlag = FALSE;

    printf("About to scan benchmark relation %s, upperlimit = %d, copymode=%d \n",
	relname, upperlimit, copymode);

    e = wiss_init();
    CHECKERR("wiss_init", e);

    transId = begin_trans();
/*
    printf("new transaction id = %d\n",transId);
*/

    /* mount the volume */
    vol = wiss_mount(VOLUME);	
    CHECKERR("select/wiss_mount", vol);

    *((int *) boolexp[0].value) = upperlimit;

    startTime = time(0);  

    w = wiss_openfile(vol, relname, READ);
    CHECKERR("select/wiss_openfile", w);

    /* selection (restriction) on the second file */
    scanid = wiss_openfilescan(w, boolexp, transId, TRUE, l_IS, FALSE);
    CHECKERR("select/wiss_openfilescan", scanid);

    e = wiss_fetchfirst(scanid, &rid, NONE);
    for (i = 0; e >= eNOERROR; i++)
    {
	if (copymode !=0) 
	{
		e = wiss_readscan(scanid, (char *) &tuple, RECSIZE);
		CHECKERR("select/wiss_readscan", e);
	}
	if (copymode == 2) bcopy((char *) &tuple, recbuf2, RECSIZE);
	else
	if (copymode == 3)
	{
		bcopy ((char *) &tuple, (char *) &fixedtup, FIXEDSIZE);
		bcopy ((char *) tuple.string1, xstring1, STRINGSIZE);
		bcopy ((char *) tuple.string2, xstring2, STRINGSIZE);
		bcopy ((char *) tuple.string3, xstring3, STRINGSIZE);
	}
	else
	if (copymode == 4)
	{
		fptr = (FIXEDTUP *) malloc(FIXEDSIZE);
		fptr->string1Ptr = (char *) malloc (STRINGSIZE);
		fptr->string2Ptr = (char *) malloc (STRINGSIZE);
		fptr->string3Ptr = (char *) malloc (STRINGSIZE);
	    
		/* don't copy over the string pointers */
		bcopy ((char *) &tuple, (char *) fptr, FIXEDSIZE-12);
		bcopy (tuple.string1, fptr->string1Ptr, STRINGSIZE);
		bcopy (tuple.string2, fptr->string2Ptr, STRINGSIZE);
		bcopy (tuple.string3, fptr->string3Ptr, STRINGSIZE);
	}

	e = wiss_fetchnext(scanid, &rid, NONE);
    }

    e = wiss_closescan(scanid);
    CHECKERR("select/wiss_closescan", e);
    e = wiss_closefile(w);
    CHECKERR("select/wiss_closefile", e);

    /* now commit the transaction */
    e = commit_trans(transId);
    if (e != 1)
      printf("error status return from commit_trans = %d\n", e);
      else printf("commit ok\n");

    /* dismount the volume */
    e = wiss_dismount(VOLUME);  
    CHECKERR("select/wiss_dismount", e);
    (void) wiss_final();

    if (verboseFlag)
    {
        totalTime = time(0) - startTime;
        printf("WISS took %d seconds to scan %d records\n", totalTime, i);
        printf("Each qualifying tuple was copied %d times\n",copymode);
	printf("\n");
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
