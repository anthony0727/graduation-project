
#include <wiss.h>
#include <wiss_r.h>
#include <st_r.h>
#include <lockquiz.h>
#include <locktype.h>

#define	NUMREC		4000
#define	NUMKEY		(NUMREC/80)
#define	RECLEN		50
#define	HNO		31
#define	WISSERROR(p,c)	if((int)(c)<0) {am_error(p, c); am_final(); exit(-1);}

#define	DEVICE		"wiss3"
#define FILENAME	"tfile30"

static	KEYINFO	keyattr = { 0, sizeof(int), TINTEGER};
static	KEY	key = {TINTEGER, "",sizeof(int) };
int	flag[NUMREC];
int	card[NUMKEY];
char 	buf[RECLEN];
RID	rids[NUMREC];

main(argc,argv)
int	argc;
char	**argv;
{
	int 	i, j, k, vol, ofn;
	int	e;
	int	hofn;
	int	scanid;
	RID	rid;
	RECORD	*recptr;
	int	trans_id;

/* other initialization */
	wiss_checkflags(&argc,&argv);

	(void) wiss_init();			
	WISSERROR("test18/wiss_init", i);

        trans_id = begin_trans();               /* inserted by LSM */
        printf("new transaction id = %d\n",trans_id);

	vol = wiss_mount(DEVICE); /* mount the device */
	WISSERROR("test18/wiss_mount", vol);

	printf("creating an empty file\n");
	i = wiss_createfile(vol, FILENAME, 9, 90,90);
	WISSERROR("test18/wiss_createfile", i);

	printf(" creating an empty hash table \n");
	e = wiss_createhash(vol, FILENAME, HNO, &keyattr, 100, 0, trans_id, TRUE, FALSE);
	WISSERROR("test18/wiss_createhash", e);

/* populate the database */

	printf(" about to insert %d records\n", NUMREC);

	ofn = wiss_openfile(vol, FILENAME, WRITE);
	WISSERROR("test18/wiss_openfile(for insertion)", ofn);

	hofn = wiss_openhash(vol, FILENAME, HNO, WRITE);
	WISSERROR("test18/wiss_openhash", hofn);

        /* lock the file in IX mode */
       	i = wiss_lock_file(trans_id, hofn, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	scanid = wiss_openhashscan(ofn, hofn, &keyattr, NULL, NULL, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test18/wiss_openhashscan", scanid);
	for (i = 0; i < NUMKEY; i++) card[i] = 0;
	for (i = 0; i < NUMREC; i++)
	{
		*(int *)buf = j = rand() % NUMKEY;
		card[j]++;
		j = wiss_insertscan(scanid, buf, RECLEN, NULL);
		WISSERROR("test18/wiss_insertscan", j);
	}
	i = wiss_closescan(scanid);
	WISSERROR("test18/wiss_closescan", i);

	i = wiss_closefile(ofn);
	WISSERROR("test18/wiss_closefile", i);
	i = wiss_closefile(hofn);
	WISSERROR("test18/wiss_closefile", i);

	for (i = 0; i < NUMKEY; i++) 
		printf("k %3d = %3d RIDs%c", i, card[i], (i%4==3)?'\n':';');

	printf(" # of records %d, # of pages %d\n", 
		wiss_recordcard(vol, FILENAME), wiss_filepages(vol, FILENAME));

	check_hash(vol, FILENAME, HNO, trans_id);

	delete_all(vol, FILENAME, HNO, trans_id);

	i = wiss_destroyhash(vol, FILENAME, HNO, trans_id, TRUE, FALSE);
	WISSERROR("test18/wiss_destroyhash", i);

	printf(" after the deletion  ");
	printf(" # of records %d, # of pages %d\n", 
		wiss_recordcard(vol, FILENAME), wiss_filepages(vol, FILENAME));

/* final clean-up */
	i = wiss_destroyfile(vol, FILENAME, trans_id, TRUE, FALSE);
	WISSERROR("test18/wiss_destroyfile", i);

         /* now commit the transaction */
         i = commit_trans(trans_id);
         if (i != 1)
            printf("error status return from commit_trans = %d\n", i);
         else printf("commit ok\n");

	i = wiss_dismount(DEVICE);  /* dismount the device wiss2 */
	WISSERROR("test18/wiss_dismount", i);
        (void) wiss_final();

	printf("That's all folks\n");

}

check_hash(vol, filename, hno, trans_id)
int	vol;
char	*filename;
int	hno;
int	trans_id;
{
	int	i, j, k, l;
	int	ofn, hofn;
	int	e, e1;
	int	count;
	int	scanid;
	RID	rid;
	XCURSOR	cursor;

	printf(" verifying the hash table ... \n");

	/* retrieve and check the records */
	ofn = wiss_openfile(vol, filename, READ);
	WISSERROR("test18/wiss_openfile", ofn);

	hofn = wiss_openhash(vol, filename, hno, READ);
	WISSERROR("test18/wiss_openhash", hofn);

	for (i = e1 = 0; i < NUMKEY; i++)
	{
		j = rand() % NUMKEY;
		if (flag[j]) { /* find another */
			for (k = j - 1; k >= 0; k--) if (!flag[k]) break;
			if (k < 0) for (k = j + 1; k < NUMREC && flag[k]; k++);
			j = k;
		}

		flag[j] = 1;
		*((int *) (key.value)) = j;

              	/* lock the file in IX mode */
        	l = wiss_lock_file(trans_id, hofn, l_IS, COMMIT, FALSE);
        	WISSERROR("test1/lockfile",l);

		scanid = wiss_openhashscan(ofn, hofn, &keyattr, &key, NULL, trans_id, TRUE, l_IS, FALSE);
		WISSERROR("test18/wiss_openscan", scanid);
		e = wiss_fetchfirst(scanid, NULL);
		for(count = 0; e >= eNOERROR; e = wiss_fetchnext(scanid, NULL))
		{
			count++;
			e = wiss_readscan(scanid, (char *) (&k), 4);
			WISSERROR("test18/wiss_readscan", e);
			if (k != j) 
			{ printf(" bug in hash (key=%d, but rec=%d)\n", j, k);
				e1 = 1;
			}
		}
		e = wiss_closescan(scanid);
		WISSERROR("test18/wiss_closescan", e);

		if (card[j] != count)
		   	printf("BUG! %d RIDs with key %d missing\n", 
				card[j] - count, j);
	}

	i = wiss_closefile(ofn);
	WISSERROR("test18/wiss_closefile", i);
	i = wiss_closefile(hofn);
	WISSERROR("test18/wiss_closefile", i);

	if (e1) printf(" the hash file is incorrect\n");
	else printf(" the hash file is correct\n");

}


delete_all(vol, filename, hno, trans_id)
int	vol;
char	*filename;
int	hno;
int	trans_id;
{
	int	i, j, k, l;
	int	ofn, hofn;
	int	e, e1;
	int	scanid;
	int	count;
	RID	rid;
	XCURSOR	cursor;

	printf(" removing all hash entries ...\n");

	/* delete the hash entry randomly */
	ofn = wiss_openfile(vol, filename, WRITE);
	WISSERROR("test18/wiss_openfile", ofn);

	hofn = wiss_openhash(vol, filename, hno, WRITE);
	WISSERROR("test18/wiss_openhash", hofn);

	for (i = 0; i < NUMKEY; i++) flag[i] = 0;
	for (i = e1 = 0; i < NUMKEY; i++)
	{
		j = rand() % NUMKEY;
		if (flag[j]) { /* find another */
			for (k = j - 1; k >= 0; k--) if (!flag[k]) break;
			if (k < 0) for (k = j + 1; k < NUMREC && flag[k]; k++);
			j = k;
		}

		flag[j] = 1;
		*((int *) (key.value)) = j;

              	/* lock the file in IX mode */
        	l = wiss_lock_file(trans_id, hofn, l_IX, COMMIT, FALSE);
        	WISSERROR("test1/lockfile",l);

		scanid = wiss_openhashscan(ofn, hofn, &keyattr, &key, NULL, trans_id, TRUE, l_IX, FALSE);
		WISSERROR("test18/wiss_openscan", scanid);
		e = wiss_fetchfirst(scanid, NULL);
		if (e < eNOERROR) printf(" key %d not found\n", j);
		for(count = 0; e >= eNOERROR; e = wiss_fetchnext(scanid, NULL))
		{
			count++;
			e = wiss_readscan(scanid, (char *) (&k), 4);
			WISSERROR("test18/wiss_readscan", e);
			if (k != j) 
			    printf(" bug in hash (key=%d, but rec=%d)\n", j, k);
			e = wiss_deletescan(scanid);
			CHECKERROR(e);
		}
		if (card[j] != count)
			printf("%d RIDs should be but didn't deleted (k=%d)\n",
				count - card[j], j);
		
		e = wiss_closescan(scanid);
		WISSERROR("test18/wiss_closescan", e);
	}

	printf(" after the deletion, the hash file :\n");
	h_dumpfile(hofn);

	i = wiss_closefile(hofn);
	WISSERROR("test18/wiss_closefile", i);

	i = wiss_closefile(ofn);
	WISSERROR("test18/wiss_closefile", i);
}
