
#include <wiss.h>
#include <st.h>

#define	NUMREC		4000
#define	NUMKEY		(NUMREC/400)
#define	RECLEN		50
#define	HNO		31
extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, bf_error((int)(c)));bf_final(); io_final(); exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();}


#define FILENAME	"tfile30"

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
	RECORD	*recptr;

/* other initialization */

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test32/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test32/bf_init", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test32/st_init", i);

	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test32/st_mount", vol);

	printf("creating an empty file\n");
	i = st_createfile(vol, FILENAME, 9, 90,90);
	STERROR("test32/st_createfile", i);

	printf(" creating an empty hash table \n");
	e = st_createhash(vol, FILENAME, HNO, &keyattr, 100, 0);
	STERROR("test32/st_createhash", e);

/* populate the database */

	printf(" about to insert %d records\n", NUMREC);

	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test32/st_openfile(for insertion)", ofn);

	for (i = 0; i < NUMKEY; i++) card[i] = 0;
	for (i = 0; i < NUMREC; i++)
	{
		j = rand() % NUMKEY;
		*(int *)buf = j;
		card[j]++;
		j = st_insertrecord(ofn, buf, RECLEN, NULL, &rid);
		STERROR("test32/st_insertrecord", j);
	}
	for (i = 0; i < NUMKEY; i++) 
		printf("k %3d = %3d RIDs%c", i, card[i], (i%4==3)?'\n':';');
	printf("\n");

	printf(" # of records %d, # of pages %d\n", 
		st_recordcard(vol, FILENAME), st_filepages(vol, FILENAME));

/* generate a hash file by insertion */
	printf("create a hash file by insertion\n");
	hofn = st_openhash(vol, FILENAME, HNO, WRITE);
	STERROR("test32/st_openhash", hofn);

	for (e = st_firstfile(ofn, &rid); e >= eNOERROR;
				e = st_nextfile(ofn, &rid, &rid))
	{
		e = r_getrecord(ofn, &rid, (DATAPAGE **)NULL, &recptr);
		STERROR("test32/r_getrecord", e);
		movebytes(key.value, &(recptr->data[keyattr.offset]), 
			keyattr.length);
		e = st_inserthash(hofn, &key, &rid);
		STERROR("test32/st_inserthash", e);
	}

	i = st_closefile(ofn);
	STERROR("test32/st_closefile", i);
	i = st_closefile(hofn);
	STERROR("test32/st_closefile", i);

	check_hash(vol, FILENAME, HNO);

	delete_all(vol, FILENAME, HNO);

	i = st_destroyhash(vol, FILENAME, HNO);
	STERROR("test32/st_destroyhash", i);

/* generating  a hash file in batch */
	printf(" creating a hash table in batch\n");
	e = st_createhash(vol, FILENAME, HNO, &keyattr, 100, 0);
	STERROR("test32/st_createhash", e);

	check_hash(vol, FILENAME, HNO);

	delete_all(vol, FILENAME, HNO);

	i = st_destroyhash(vol, FILENAME, HNO);
	STERROR("test32/st_destroyhash", i);

/* final clean-up */
	i = st_destroyfile(vol, FILENAME);
	STERROR("test32/st_destroyfile", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test32/st_dismount", i);

	printf("That's all folks\n");

}


check_hash(vol, filename, hno)
int	vol;
char	*filename;
int	hno;
{

	int	i, j, k;
	int	ofn, hofn;
	int	e, e1;
	int	count = 0;
	RID	rid;
	XCURSOR	cursor;

	printf(" verifying the hash table ... \n");

	/* retrieve and check the records */
	ofn = st_openfile(vol, filename, READ);
	STERROR("test32/st_openfile", ofn);

	hofn = st_openhash(vol, filename, hno, READ);
	STERROR("test32/st_openhash", hofn);

	e1 = 0;
	for (i = 0; i < NUMKEY; i++) flag[i] = 0;
	for (i = 0; i < NUMKEY; i++)
	{
		j = rand() % NUMKEY;
		if (flag[j])
		{ /* find another */
			for (k = j - 1; k >= 0; k--) if (!flag[k]) break;
			if (k < 0)
			  for (k = j + 1; k < NUMREC; k++) if (!flag[k]) break;
			j = k;
		}

		flag[j] = 1;
		*((int *) (key.value)) = j;
		for (count = 0, e = st_gethash(hofn, &key, &cursor, &rid);
			e >= eNOERROR; e = st_nexthash(hofn, &cursor, &rid))
		{
			count++;
			e = st_readrecord(ofn, &rid, (char *) (&k), 4);
			STERROR("test32/st_readrecord", e);
			if (k != j) 
			{
				printf(" bug in hash (key=%d, but rec=%d)\n", 
					j, k);
				e1 = 1;
			}
		}
		if (card[j] != count)
			printf("BUG!, %d RIDs with key %d missing\n", 
				card[j] - count, j);
	}

	i = st_closefile(ofn);
	STERROR("test32/st_closefile", i);
	i = st_closefile(hofn);
	STERROR("test32/st_closefile", i);

	if (e1) printf(" the hash file is incorrect\n");
	else printf(" the hash file is correct\n");

}

delete_all(vol, filename, hno)
int	vol;
char	*filename;
int	hno;
{

	int	i, j, k;
	int	ofn, hofn;
	int	e, e1;
	int	count;
	RID	rid;
	XCURSOR	cursor;

	printf(" removing all hash entries ...\n");

	/* delete the hash entry randomly */
	hofn = st_openhash(vol, filename, hno, WRITE);
	STERROR("test32/st_openhash", hofn);

	for (i = 0; i < NUMKEY; i++) flag[i] = 0;
	for (i = 0; i < NUMKEY; i++)
	{
		j = rand() % NUMKEY;
		if (flag[j])
		{ /* find another */
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
		else printf(" %d records with the key %d to be deleted\n", 
			count, j);
		for (k = 0, e = eNOERROR; k < count && e >= eNOERROR; k++)
		{
			e = st_deletehash(hofn, &key, &rids[k]);
			STERROR("test32/st_deletehash", e);
		}
		if (count - k != 0)
			printf("%d RIDs should be but didn't deleted (k=%d)\n",
				count - k, j);

	}

	i = st_closefile(hofn);
	STERROR("test32/st_closefile", i);

}
