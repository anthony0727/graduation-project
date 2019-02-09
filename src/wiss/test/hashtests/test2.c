
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


#define    NUMREC	    4000
#define    NUMKEY	    (NUMREC/400)
#define    RECLEN	    50
#define    HNO	    12

#define    CHECKERR(p,c) if((int)(c)<0) fatalerror(p,(int)(c))
#define    VOLUME	    "hp0a"

#define FILENAME    "tfile2"

static    KEYINFO	keyattr = { 0, sizeof(int), TINTEGER};
static    KEY	key = {TINTEGER, "", sizeof(int)};
int    	  flag[NUMREC];
char      buf[RECLEN];
RID       rids[NUMREC];
int       card[NUMKEY];

int      trans_id;        

main(argc,argv)
int    argc;
char    **argv;
{
    int i, j, k, vol, ofn;
    int	e;
    int	hofn;
    RID	rid;
    short   lockup = TRUE;
    short   cond = FALSE;

    wiss_init();

    trans_id = begin_trans();
    printf("new transaction id = %d\n",trans_id);

    vol = wiss_mount(VOLUME);  	/* mount the device wiss_disk */
    CHECKERR("test1/wiss_mount", vol);

    wiss_destroyfile(vol, FILENAME, trans_id, FALSE, cond);
    wiss_destroyhash(vol, FILENAME, HNO, trans_id, FALSE, cond);

    printf(" Creating an empty file\n");
    e = wiss_createfile(vol, FILENAME, 9, 90,90);
    CHECKERR("test2/wiss_createfile", e);

    printf(" Creating an empty hash index\n");
    e = wiss_createhash(vol, FILENAME, HNO, &keyattr, 100, FALSE, trans_id, 
    	lockup, cond);
    CHECKERR("test2/wiss_createhash", e);

/* populate the database */

    printf(" About to insert %d records in random key order\n", NUMREC);

    ofn = wiss_openfile(vol, FILENAME, WRITE);
    CHECKERR("test2/wiss_openfile(for insertion)", ofn);
    for (i = 0; i < NUMKEY; i++) card[i] = 0;
    for (i = 0; i < NUMREC; i++) 
    {
    	j = rand() % NUMKEY;
    	*(int *)buf = j;
    	card[j]++;
    	e = wiss_insertrecord(ofn, buf, RECLEN, NULL, &rid, trans_id, lockup,
		cond);
    	CHECKERR("test2/wiss_insertrecord", e);
    }
    for (i = 0; i < NUMKEY; i++) 
    	printf(" k%3.3d has %3d RIDs%c", i, card[i], (i%4==3)?'\n':';');
    printf("\n");

/* generate a hash file by insertion */
    printf(" Create a hash index by insertion\n");
    hofn = wiss_openhash(vol, FILENAME, HNO, WRITE);
    CHECKERR("test2/wiss_openhash", hofn);

    for (e = wiss_firstfile(ofn, &rid, trans_id, lockup, l_S, cond); 
      e >= eNOERROR; 
      e = wiss_nextfile(ofn, &rid, &rid, trans_id, lockup, l_S, cond)) 
    {
    	e = wiss_readrecord(ofn , &rid, buf, RECLEN, 
		trans_id, lockup, l_S, cond);
    	CHECKERR("test2/wiss_readrecord", e);
    	movebytes(key.value, buf+keyattr.offset, keyattr.length);
    	e = wiss_inserthash(hofn, &key, &rid, trans_id, lockup, cond);
    	CHECKERR("test2/wiss_inserthash", e);
    }
    printf("finished creation by insertion \n");

    e = wiss_closefile(hofn);
    CHECKERR("test2/wiss_closefile", e);
    e = wiss_closefile(ofn);
    CHECKERR("test2/wiss_closefile", e);

    /* check the correctness of the hash index */
    check_hash(vol, FILENAME, HNO, trans_id, lockup, cond);

    /* delete all the index entries */
    delete_all(vol, FILENAME, HNO, trans_id, lockup, cond);

    printf("delete all completed successfully\n");
    e = commit_trans(trans_id);
    if (e != 1)
	  printf("error status return from commit_trans = %d\n", e);

    wiss_destroyhash(vol, FILENAME, HNO, trans_id, FALSE, cond);

    /* now now commit the transaction */
    trans_id = begin_trans();
    printf("new transaction id = %d\n",trans_id);
    BF_dumpfixed();

/* generating  a hash file in batch */
    printf(" Creating a hash index in batch\n");
    e = wiss_createhash(vol, FILENAME, HNO, &keyattr, 100, FALSE,
	trans_id, lockup, cond);
printf("error code returned from createhash = %d\n",e);
    CHECKERR("test2/wiss_createhash", e);
#ifdef    BUFTRACE
    BF_dumpfixed();
#endif

    /* check the correctness of the hash index */
    check_hash(vol, FILENAME, HNO, trans_id, lockup, cond);
#ifdef    BUFTRACE
    BF_dumpfixed();
#endif

    /* delete all the index entries */
    delete_all(vol, FILENAME, HNO, trans_id, lockup, cond);
#ifdef    BUFTRACE
    BF_dumpfixed();
#endif
    /* now now commit the transaction */
    e = commit_trans(trans_id);
    if (e != 1)
	  printf("error status return from commit_trans = %d\n", e);
    else printf("commit ok\n");

/* final clean-up */

    wiss_destroyfile(vol, FILENAME, trans_id, FALSE, cond);
    wiss_destroyhash(vol, FILENAME, HNO, trans_id, FALSE, cond);

    e = wiss_dismount(VOLUME);  /* dismount the device hp0a */
    CHECKERR("test2/wiss_dismount", e);
#ifdef    BUFTRACE
    BF_dumpfixed();
#endif

    (void) wiss_final();

    printf("That's all folks\n");

}

/* This routine checks the hash index in random key order */
check_hash(vol, filename, hno, trans_id, lockup, cond)
int    vol;
char    *filename;
int    hno;
int     trans_id;
short   lockup;
short   cond;
{

    register int i;
    int	j, k;
    int	ofn, hofn;
    int	e, e1;
    int	count = 0;
    RID	rid;
    XCURSOR	cursor;

    printf(" Checking the hash index ... \n");

    /* retrieve and check the records */
    ofn = wiss_openfile(vol, filename, READ);
    CHECKERR("test2/wiss_openfile", ofn);
    hofn = wiss_openhash(vol, filename, hno, READ);
    CHECKERR("test2/wiss_openhash", hofn);

    e1 = 0;
    for (i = 0; i < NUMKEY; i++) flag[i] = 0;
    for (i = 0; i < NUMKEY; i++) {
    	j = rand() % NUMKEY;
    	if (flag[j]) { /* find another */
    	    for (k = j - 1; k >= 0; k--) if (!flag[k]) break;
    	    if (k < 0)
    	      for (k = j + 1; k < NUMREC; k++) if (!flag[k]) break;
    	    j = k;
    	}

    	flag[j] = 1;
	bcopy(&j, key.value, sizeof(j));
    	for (count=0, 
	    e = wiss_gethash(hofn,&key,&cursor,&rid, trans_id, lockup, cond);
    	 e >= eNOERROR; 
	 e = wiss_nexthash(hofn,&cursor,&rid, trans_id, lockup, cond), count++) 
	 {
    	    e = wiss_readrecord(ofn, &rid, (char *) (&k), 4, trans_id, lockup, 
		l_S, cond);
    	    CHECKERR("test2/wiss_readrecord", e);
    	    if (k != j) {
    	    	printf(" bug in hash (key=%d, but rec=%d)\n", 
    	    	    j, k);
    	    	e1 = 1;
    	    }
    	}

    	if (card[j] != count) {
    	    printf("BUG!, %d RIDs with key %d missing\n", 
    	    	card[j] - count, j);
    	    e1 = 1;
    	}
    }

    e = wiss_closefile(ofn);
    CHECKERR("test2/wiss_closefile", e);
    e = wiss_closefile(hofn);
    CHECKERR("test2/wiss_closefile", e);

    if (e1) printf(" - the hash index is incorrect\n");
    else printf(" - the hash index is correct\n");

}

/* This routine deletes all the entries in a hash index in random order */
delete_all(vol, filename, hno, trans_id, lockup, cond)
int    vol;
char    *filename;
int    hno;
int     trans_id;
short   lockup;
short   cond;

{
    register int i;
    int	j, k;
    int	ofn, hofn;
    int	e, e1;
    int	count;
    RID	rid;
    XCURSOR	cursor;

    printf(" Removing all hash entries ...\n");

    /* delete the hash entry randomly */
    hofn = wiss_openhash(vol, filename, hno, WRITE);
    CHECKERR("test2/wiss_openhash", hofn);

    /* lock the file in IX mode */
    e = wiss_lock_file(trans_id, hofn, l_IX, COMMIT, cond);
    CHECKERR("test2/lockfile", e);

    for (i = 0; i < NUMKEY; i++) flag[i] = 0;
    for (i = 0; i < NUMKEY; i++) 
    {
    	j = rand() % NUMKEY;
    	if (flag[j]) { /* find another */
    	    for (k = j - 1; k >= 0; k--) if (!flag[k]) break;
    	    if (k < 0)
    	      for (k = j + 1; k < NUMREC; k++) if (!flag[k]) break;
    	    j = k;
    	}

    	flag[j] = 1;
	bcopy(&j, key.value, sizeof(j));

    	for (count = 0, 
	 e = wiss_gethash(hofn, &key, &cursor, &rid, trans_id, lockup, cond); 
    	 e >= eNOERROR; 
	 e = wiss_nexthash(hofn, &cursor, &rid, trans_id, lockup, cond))
	{
    	    	rids[count++] = rid;
	}
    	if (count != card[j]) 
    	  printf("BUG! there should be %d but %d RIDs with key %d\n",
    	    card[j], count, j);
    	else printf("  deleting %d records with key %d \n", count, j);
    	for (k = 0, e = eNOERROR; k < count && e >= eNOERROR; k++) 
	{
    	    e = wiss_deletehash(hofn, &key, &rids[k], trans_id, lockup, cond);
    	    CHECKERR("test2/wiss_deletehash", e);
    	}
    	if (count - k != 0)
    	    printf("%d RIDs should be but wasn't deleted (k=%d)\n",
    	    	count - k, j);
    }
    printf("finished deleteing about to close the file\n");

    e = wiss_closefile(hofn);
    CHECKERR("test2/wiss_closefile", e);
}



fatalerror(p, e)
char *p; int e;
{
   int ex;

   printf("Fatal wiss error,  abort the transaction\n");
   wiss_abort_trans(trans_id);

   /* dismount the device */
   (void) wiss_dismount(VOLUME);  
   wiss_final();  /* clean up processes and shared memory segments */

   wiss_fatalerror(p,(int) e);
}
