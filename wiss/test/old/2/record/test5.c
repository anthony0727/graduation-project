/* program to test long data item routines */

#include <wiss.h>
#include <st.h>

#define	BUFTRACE	1

extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n",p,bf_error((int)(c)));bf_final();io_final();exit(-1);}
#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
			io_final();exit(-1);}

#define FILENAME	"tfile5"
#define	PFF		50
#define	MAXRECSIZE	4000
#define	NUMTESTS	1000
#define	MAXNUMRECS	NUMTESTS

/* for keeping track of the records */
int	num_records = 0;
int	num_bytes = 0;
int	reclens[MAXNUMRECS+1];
RID	rids[MAXNUMRECS];
char	seeds[MAXNUMRECS];

/* for checking the data file */
char	checked[MAXNUMRECS];	

/* for gathering statistics on record operations */
int	num_insert, num_append, num_delete, num_read, num_write;

/* record buffer */
char	buf[MAXRECSIZE];

main(argc,argv)
int	argc;
char	**argv;
{
	int	e;
	int	vol, ofn;
	register i, j, k;
	register pos;
	register *ibuf = (int *)buf;

/* initialization */

	printf(" Initializing ... \n");
	wiss_checkflags(&argc,&argv);
	e = io_init();			/* initialize level 0 */
	IOERROR("test5/io_init", e);
	e = bf_init();		/* initialize level 1 */
	BFERROR("test5/bf_init", e);
	e = st_init();			/* initialize level 2 */
	STERROR("test5/st_init", e);
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test5/st_mount", vol);
	e = st_createfile(vol, FILENAME, 9, PFF,PFF);
	STERROR("test5/st_createfile", e);
	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test5/st_openfile(for insertion)", ofn);

/* start of the tests */
	num_records = num_bytes = 0;
	num_insert = num_delete = num_read = num_write = num_append = 0;
	for (i = 0; i < MAXNUMRECS+1; i++) reclens[i] = 0;

	printf("Start testing\n");
	for (i = 0; i < NUMTESTS; i++) {

		/* randomly pick a position in the test domain */
		pos = random() % MAXNUMRECS;

		if (reclens[pos] > 0) { /* the record already exists */
			e = st_readrecord(ofn, &rids[pos], buf, reclens[pos]);
			if (e != reclens[pos]) {
				printf("ERR: length of record %d incorrect\n", 
					pos);
				exit(-1);
			}
			k = reclens[pos]/sizeof(int);
			for (j = 0; j < k && ibuf[j] == pos; j++);
			if (j < reclens[pos]/sizeof(int)) {
				printf("ERR: bad data in record %d\n", pos);
				exit(-1);
			}
			num_read++;

			if (random() % 2) { /* expand the record if possible */
				if (reclens[pos] > MAXRECSIZE/2) 
					continue;	/* too large to grow */
				j = reclens[pos]/sizeof(int);
				k = j+j;
				for (; j < k; j++) ibuf[j] = pos;
				num_bytes += reclens[pos];
				reclens[pos] += reclens[pos];
				e = st_writerecord(ofn, &rids[pos], 
					buf, reclens[pos]);
				if (e != reclens[pos]) {
					printf("ERR: write record %d failed\n",
						pos);
					exit(-1);
				}
				num_write++;
			}
			else { /* delete the record */
				e = st_deleterecord(ofn, &rids[pos]);
				STERROR("test5/st_deleterecord", e);
				num_bytes -= reclens[pos];
				reclens[pos] = 0;
				num_delete++;
				num_records--;
			}
		}
		else { /* create a new record */
			j = random()%(MAXRECSIZE-1)/sizeof(int) + 1;
			reclens[pos] = j * sizeof(int);
			num_bytes += reclens[pos];
			for (k = 0; k < j; k++) ibuf[k] = pos;

			if (reclens[pos+1] > 0) { /* insert record */
				e = st_insertrecord(ofn, buf, reclens[pos],
					&rids[pos+1], &rids[pos]);
				STERROR("test5/st_insertrecord", e);
				num_insert++;
			}
			else { /* append record */
				e = st_appendrecord(ofn, buf, reclens[pos],
					&rids[pos]);
				STERROR("test5/st_appendrecord", e);
				num_append++;
			}
			num_records++;
		}

		/* a complete check once every 50 iterations */
		if ( !((i+1) % 50) ) {
#ifdef	BUFTRACE
	BF_dumpfixed();	/* check if any buffer fixed in the buffer pool */
#endif
			printf(" Checking the file after %d iterations", i+1);
			printf(" (%d records and %d bytes)\n", 
				num_records, num_bytes);

			if (random()%2) check_file_forward(ofn);
			else check_file_backward(ofn);
		}
	}

/* Clean up the mess */
	printf(" Cleaning up ...\n");
	e = st_closefile(ofn);
	STERROR("test5/st_closefile", e);
	e = st_destroyfile(vol, FILENAME);
	STERROR("test5/st_destroyfile", e);
	e = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test5/st_dismount", e);

	printf("END-OF-TEST\n");
	printf(" %d inserts, %d deletes, %d reads, %d writes, %d appends\n",
		num_insert, num_delete, num_read, num_write, num_append);
	printf(" %d records (%d bytes) were in the file when test ended\n", 
		num_records, num_bytes);

}

/* check the data file in the forward direction */
check_file_forward(ofn)
int	ofn;
{
	register	i, j, k;
	register	count;
	register	*ibuf;
	int		e, err;
	RID		rid;

	err = count = 0;
	for (j = 0; j < MAXNUMRECS; j++) checked[j] = 0;
	for(e = st_firstfile(ofn, &rid); e >= eNOERROR;
			e = st_nextfile(ofn, &rid, &rid), count++) {

		k = st_readrecord(ofn, &rid, buf, MAXRECSIZE);
		STERROR("test5/st_readrecord", k);
		ibuf = (int *) buf;
		i = *ibuf;
		err = ((!RIDEQ(rid, rids[i])) || (k != reclens[i]));
		for (j = 0; j < k/sizeof(int); j++) 
			if (*(ibuf++) != i) err = 1;
		if (err) printf("ERR: bad contents in record %d\n", i);
		else if (checked[i]) printf("ERR: duplicate record %d\n", i);
		else checked[i] = 1;
	}
	if (count != num_records) 
		printf("ERR: # of records should be %d not %d\n",
			num_records, count);
}

/* check the data file in the backward direction */
check_file_backward(ofn)
int	ofn;
{
	register	i, j, k;
	register	count;
	register	*ibuf;
	int		e, err;
	RID		rid;

	err = count = 0;
	for (j = 0; j < MAXNUMRECS; j++) checked[j] = 0;
	for(e = st_lastfile(ofn, &rid); e >= eNOERROR;
			e = st_prevfile(ofn, &rid, &rid), count++) {

		k = st_readrecord(ofn, &rid, buf, MAXRECSIZE);
		STERROR("test5/st_readrecord", k);
		ibuf = (int *) buf;
		i = *ibuf;
		err = ((!RIDEQ(rid, rids[i])) || (k != reclens[i]));
		for (j = 0; j < k/sizeof(int); j++) 
			if (*(ibuf++) != i) err = 1;
		if (err) printf("ERR: bad contents in record %d\n", i);
		else if (checked[i]) printf("ERR: duplicate record %d\n", i);
		else checked[i] = 1;
	}
	if (count != num_records) 
		printf("ERR: # of records should be %d not %d\n",
			num_records, count);
}

