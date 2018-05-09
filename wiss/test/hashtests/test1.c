
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



#include <wiss_r.h>
#include <wiss.h>
#include <st.h>
#include <lockquiz.h>

#define	BUFTRACE	0

#define	NUMREC		2500
#define	RECLEN		50
#define	HNO		11

#define	CHECKERR(p,c) if((int)(c)<0) fatalerror(p,(int)(c))

#define		VOLUME		"hp0a"

#define FILENAME	"tfile1"

static	KEYINFO	keyattr = { 0, sizeof(int), TINTEGER};
static	KEY	key = {TINTEGER, "", sizeof(int)};
int	flag[NUMREC];
char 	buf[RECLEN];
int     trans_id;        

main(argc,argv)
int	argc;
char	**argv;
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
	
	wiss_destroyfile(vol, FILENAME, trans_id, lockup, cond);
	wiss_destroyhash(vol, FILENAME, HNO, trans_id, lockup, cond);
	
	printf(" Creating an empty file\n");
	e = wiss_createfile(vol, FILENAME, 9, 90,90);
	CHECKERR("test1/wiss_createfile", e);

	printf(" Creating an empty, unique, hash index\n");
	e = wiss_createhash(vol, FILENAME, HNO, &keyattr, 100, TRUE, 
			    trans_id, lockup, cond);
	CHECKERR("test1/wiss_createhash", e);

/* populate the database */
	printf(" About to insert %d records with random keys\n", NUMREC);
	ofn = wiss_openfile(vol, FILENAME, WRITE);
	CHECKERR("test1/wiss_openfile(for insertion)", ofn);
	for (i = 0; i < NUMREC; i++) flag[i] = 0;
	for (i = 0; i < NUMREC; i++) {
		j = rand() % NUMREC;
		if (flag[j]) { /* find another */
			for (k = j - 1; k >= 0; k--) if (!flag[k]) break;
			if (k < 0)
			  for (k = j + 1; k < NUMREC; k++) if (!flag[k]) break;
			j = k;
		}
		flag[j] = 1;
		*(int *)buf = j;
		e = wiss_insertrecord(ofn, buf, RECLEN, NULL, &rid, 
				    trans_id, lockup, cond);
		CHECKERR("test1/wiss_insertrecord", e);
	}
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* generate a hash file by insertion */
	printf(" Creating a hash index by insertion ...\n");
	hofn = wiss_openhash(vol, FILENAME, HNO, WRITE);
	CHECKERR("test1/wiss_openhash", hofn);

	i = 0;
	for (e = wiss_firstfile(ofn, &rid, trans_id, lockup, l_S, cond); 
	     e >= eNOERROR;
	     e = wiss_nextfile(ofn, &rid, &rid, trans_id, lockup, l_S, cond)) 
	{
	      e = wiss_readrecord(ofn, &rid, buf, RECLEN, 
			      trans_id, lockup, l_S, cond);
	      CHECKERR("test1/wiss_readrecord", e);

/*
	      printf(" %d (key= %d, rid= %d/%d/%d) \n", i
		++, *(int *)buf, rid.Rslot, rid.Rpage, rid.Rvolid);
*/

	      movebytes(key.value, buf+keyattr.offset, keyattr.length);
	      e = wiss_inserthash(hofn, &key, &rid, trans_id, lockup, cond);
	      CHECKERR("test1/wiss_inserthash", e);
	}

	e = wiss_closefile(ofn);
	CHECKERR("test1/wiss_closefile", e);
	e = wiss_closefile(hofn);
	CHECKERR("test1/wiss_closefile", e);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* check the correctness of the hash index */
	check_hash(vol, FILENAME, HNO, trans_id, lockup, cond);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	e = commit_trans(trans_id);
	if (e != 1) 
	  printf("error status return from commit_trans = %d\n", e);
	else printf("commit ok\n");

	trans_id = begin_trans();
	printf("new transaction id = %d\n",trans_id);

	/* delete all the hash entries */
	delete_all(vol, FILENAME, HNO, trans_id, lockup, cond);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	e = wiss_destroyhash(vol, FILENAME, HNO, trans_id, lockup, cond);
	CHECKERR("test1/wiss_destroyhash", e);

	/* create a hash file in batch */
	printf(" Creating a hash index in batch\n");
	e = wiss_createhash(vol, FILENAME, HNO, &keyattr, 100, TRUE, 
			    trans_id, lockup, cond);
	CHECKERR("test1/wiss_createhash", e);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif
	printf("batch creation of hash index succeeds\n");

	/* check the correctness of the hash index */
	check_hash(vol, FILENAME, HNO, trans_id, lockup, cond);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* delete all the hash entries */
	delete_all(vol, FILENAME, HNO, trans_id, lockup, cond);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	e = wiss_destroyhash(vol, FILENAME, HNO, trans_id, lockup, cond);
	CHECKERR("test1/wiss_destroyhash", e);

	/* final clean-up */
	printf("Cleaning up the disk\n");
	e = wiss_destroyfile(vol, FILENAME, trans_id, lockup, cond);
	CHECKERR("test1/wiss_destroyfile", e);

	/* now commit the transaction */
	e = commit_trans(trans_id);
	if (e != 1) 
	  printf("error status return from commit_trans = %d\n", e);
	else printf("commit ok\n");

	e = wiss_dismount(VOLUME);  /* dismount the device wiss_disk */
	CHECKERR("test1/st_dismount", e);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	printf("That's all folks\n");
	(void) wiss_final();
}


/* This routine check the correctness of a hash index */
check_hash(vol, filename, hno, trans_id, lockup, cond)
int	vol;
char	*filename;
int	hno;
int     trans_id;
short   lockup;
short	cond;
{
	register i;
	int	j, k;
	int	ofn, hofn;
	int	e, e1;
	RID	rid;
	XCURSOR	cursor;
	int	before;

	printf(" Checking the hash index ... \n");

	/* retrieve and check the records */
	ofn = wiss_openfile(vol, filename, READ);
	CHECKERR("test1/wiss_openfile", ofn);
	hofn = wiss_openhash(vol, filename, hno, READ);
	CHECKERR("test1/wiss_openhash", hofn);

	for (i = 0; i < NUMREC; i++) flag[i] = 0;
	e1 = 0;
	for (i = 0; i < NUMREC; i++) {
		j = rand() % NUMREC;
		if (flag[j]) { /* find another */
			for (k = j - 1; k >= 0; k--) if (!flag[k]) break;
			if (k < 0)
			  for (k = j + 1; k < NUMREC; k++) if (!flag[k]) break;
			j = k;
		}
		flag[j] = 1;
		bcopy(&j, key.value, sizeof(j));
		e = wiss_gethash(hofn, &key, &cursor, &rid, 
				 trans_id, lockup, cond);
		if (e < 0) {
		    printf("gethash returned error:%d on key.value=%d\n",
			e, j);
		    return(e);
		}
    /*
		CHECKERR("test1/wiss_gethash", e);
    */

/*
		printf(" %d key= %d rid= %d/%d/%d\n", 
			i, j, rid.Rslot, rid.Rpage, rid.Rvolid);
*/

		e = wiss_readrecord(ofn, &rid, (char *) (&k), 4, 
				    trans_id, lockup, l_S, cond);
		CHECKERR("test1/wiss_readrecord", e);

		if (k != j) {
			printf(" bug in hash (key=%d, but rec=%d)\n", j, k);
			e1 = 1;
		}
	}

	e = wiss_closefile(ofn);
	CHECKERR("test1/wiss_closefile", e);
	e = wiss_closefile(hofn);
	CHECKERR("test1/wiss_closefile", e);

	if (e1) printf(" - the hash index is incorrect\n");
	else printf(" - the hash index is correct\n");
}

/* This routine randomly deletes all the entries of a hash index */
delete_all(vol, filename, hno, trans_id, lockup, cond)
int	vol;
char	*filename;
int	hno;
int     trans_id;
short   lockup;
short	cond;
{

	register i, k;
	int	j, hofn;
	int	e;
	RID	rid;
	XCURSOR	cursor;

	printf(" Removing all hash entries ...\n");

	/* delete the hash entry randomly */
	hofn = wiss_openhash(vol, filename, hno, WRITE);
	CHECKERR("test1/wiss_openhash", hofn);

	/* lock the file in IX mode */
	e = wiss_lock_file(trans_id, hofn, l_IX, COMMIT, cond);
	CHECKERR("test1/lockfile", e);

	for (i = 0; i < NUMREC; i++) flag[i] = 0;
	for (i = 0; i < NUMREC; i++) {
		j = rand() % NUMREC;
		if (flag[j]) { /* find another */
			for (k = j - 1; k >= 0; k--) if (!flag[k]) break;
			if (k < 0)
			  for (k = j + 1; k < NUMREC; k++) if (!flag[k]) break;
			j = k;
		}

		flag[j] = 1;
		bcopy(&j, key.value, sizeof(j));
		e = wiss_gethash(hofn, &key, &cursor, &rid, 
				 trans_id, lockup, cond);
		CHECKERR("test1/wiss_gethash", e);
		e = wiss_deletehash(hofn, &key, &rid, trans_id, lockup, cond);
		CHECKERR("test1/st_deletehash", e);
	}

/*
	printf(" After the deletions, the hash file is:\n");
	h_dumpfile(hofn);
*/

	e = wiss_closefile(hofn);
	CHECKERR("test1/wiss_closefile", e);
	printf("delete all completes without errors\n");
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
