
#include <wiss.h>
#include <st.h>

#define	BUFTRACE	1

#define	NUMREC		4000
#define	NUMKEY		(NUMREC/400)
#define	RECLEN		50
#define	HNO		12
extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,io_error((int)(c)));io_final();exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,bf_error((int)(c)));bf_final();io_final();exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
			io_final();exit(-1);}

#define FILENAME	"tfile2"

static	KEYINFO	keyattr = { 0, sizeof(int), TINTEGER};
static	KEY	key = {TINTEGER, sizeof(int), "\0"};
int	flag[NUMREC];
char 	buf[RECLEN];
RID	rids[NUMREC];
int	card[NUMKEY];

main(argc,argv)
int	argc;
char	**argv;
{
	int i, j, k, vol, ofn;
	int	e;
	int	hofn;
	RID	rid;

	printf("Initializing ... \n");
	wiss_checkflags(&argc,&argv);
	e = io_init();			/* initialize level 0 */
	IOERROR("test2/io_init", e);
	e = bf_init();		/* initialize level 1 */
	BFERROR("test2/bf_init", e);
	e = st_init();			/* initialize level 2 */
	STERROR("test2/st_init", e);
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test2/st_mount", vol);
	printf(" Creating an empty file\n");
	e = st_createfile(vol, FILENAME, 9, 90,90);
	STERROR("test2/st_createfile", e);
	printf(" Creating an empty hash index\n");
	e = st_createhash(vol, FILENAME, HNO, &keyattr, 100, 0);
	STERROR("test2/st_createhash", e);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* populate the database */

	printf(" About to insert %d records in random key order\n", NUMREC);

	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test2/st_openfile(for insertion)", ofn);
	for (i = 0; i < NUMKEY; i++) card[i] = 0;
	for (i = 0; i < NUMREC; i++) {
		j = rand() % NUMKEY;
		*(int *)buf = j;
		card[j]++;
		e = st_insertrecord(ofn, buf, RECLEN, NULL, &rid);
		STERROR("test2/st_insertrecord", e);
	}
	for (i = 0; i < NUMKEY; i++) 
		printf(" k%3.3d has %3d RIDs%c", i, card[i], (i%4==3)?'\n':';');
	printf("\n");
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

/* generate a hash file by insertion */
	printf(" Create a hash index by insertion\n");
	hofn = st_openhash(vol, FILENAME, HNO, WRITE);
	STERROR("test2/st_openhash", hofn);
	for (e = st_firstfile(ofn, &rid); e >= eNOERROR;
			e = st_nextfile(ofn, &rid, &rid)) {
		e = st_readrecord(ofn , &rid, buf, RECLEN);
		STERROR("test2/st_readrecord", e);
		movebytes(key.value, buf+keyattr.offset, keyattr.length);
		e = st_inserthash(hofn, &key, &rid);
		STERROR("test2/st_inserthash", e);
	}

	e = st_closefile(hofn);
	STERROR("test2/st_closefile", e);
	e = st_closefile(ofn);
	STERROR("test2/st_closefile", e);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* check the correctness of the hash index */
	check_hash(vol, FILENAME, HNO);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* delete all the index entries */
	delete_all(vol, FILENAME, HNO);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	e = st_destroyhash(vol, FILENAME, HNO);
	STERROR("test2/st_destroyhash", e);

/* generating  a hash file in batch */
	printf(" Creating a hash index in batch\n");
	e = st_createhash(vol, FILENAME, HNO, &keyattr, 100, 0);
	STERROR("test2/st_createhash", e);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* check the correctness of the hash index */
	check_hash(vol, FILENAME, HNO);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	/* delete all the index entries */
	delete_all(vol, FILENAME, HNO);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	e = st_destroyhash(vol, FILENAME, HNO);
	STERROR("test2/st_destroyhash", e);

/* final clean-up */

	printf("Cleaning up the disk\n");
	e = st_destroyfile(vol, FILENAME);
	STERROR("test2/st_destroyfile", e);
	e = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test2/st_dismount", e);
#ifdef	BUFTRACE
	BF_dumpfixed();
#endif

	printf("That's all folks\n");
}

/* This routine checks the hash index in random key order */
check_hash(vol, filename, hno)
int	vol;
char	*filename;
int	hno;
{

	register i, j;
	int	k;
	int	ofn, hofn;
	int	e, e1;
	int	count = 0;
	RID	rid;
	XCURSOR	cursor;

	printf(" Checking the hash index ... \n");

	/* retrieve and check the records */
	ofn = st_openfile(vol, filename, READ);
	STERROR("test2/st_openfile", ofn);
	hofn = st_openhash(vol, filename, hno, READ);
	STERROR("test2/st_openhash", hofn);

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
		*((int *) (key.value)) = j;
		for (count=0, e = st_gethash(hofn,&key,&cursor,&rid);
		    e >= eNOERROR; e = st_nexthash(hofn,&cursor,&rid),count++) {
			e = st_readrecord(ofn, &rid, (char *) (&k), 4);
			STERROR("test2/st_readrecord", e);
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

	e = st_closefile(ofn);
	STERROR("test2/st_closefile", e);
	e = st_closefile(hofn);
	STERROR("test2/st_closefile", e);

	if (e1) printf(" - the hash index is incorrect\n");
	else printf(" - the hash index is correct\n");

}

/* This routine deletes all the entries in a hash index in random order */
delete_all(vol, filename, hno)
int	vol;
char	*filename;
int	hno;
{
	register i, j;
	int	k;
	int	ofn, hofn;
	int	e, e1;
	int	count;
	RID	rid;
	XCURSOR	cursor;

	printf(" Removing all hash entries ...\n");

	/* delete the hash entry randomly */
	hofn = st_openhash(vol, filename, hno, WRITE);
	STERROR("test2/st_openhash", hofn);
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
		*((int *) (key.value)) = j;

		for (count = 0, e = st_gethash(hofn, &key, &cursor, &rid); 
		   e >= eNOERROR; e = st_nexthash(hofn, &cursor, &rid))
				rids[count++] = rid;
		if (count != card[j]) 
		  printf("BUG! there should be %d but %d RIDs with key %d\n",
			card[j], count, j);
		else printf("  deleting %d records with key %d \n", 
			count, j);
		for (k = 0, e = eNOERROR; k < count && e >= eNOERROR; k++) {
			e = st_deletehash(hofn, &key, &rids[k]);
			STERROR("test2/st_deletehash", e);
		}
		if (count - k != 0)
			printf("%d RIDs should be but wasn't deleted (k=%d)\n",
				count - k, j);
	}

	e = st_closefile(hofn);
	STERROR("test2/st_closefile", e);
}

