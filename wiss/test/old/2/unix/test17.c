/* program to test record routines */
/* 	
	st_insertrecord
	st_deleterecord
	st_appendrecord
	st_readrecord
	st_writerecord
	
	st_firstfile
	st_lastfile
	st_nextfile
	st_prevfile
*/

#include <wiss.h>

#define SHORT		20
#define LONG		1000
#define	NUMREC		100
extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, bf_error((int)(c)));bf_final(); io_final(); exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();}


#define PRREC(rec) 	if (*(rec+12) == 's') printf("%20.20s\n", rec);\
			else printf("%20.20s...%20.20s\n",rec,rec +980)
#define FILENAME	"tfile10"
#define DUMPFILE	ofn = st_openfile(vol, FILENAME, READ);\
			STERROR("test17/st_openfile", ofn);\
			dumpfile_forward(ofn);\
			i = st_closefile(ofn);\
			STERROR("test17/st_closefile", i)

dumpfile_forward(ofn)
int	ofn;
{
	int	i,j;
	RID	rid1, rid2;
	char	buf[LONG];

	printf(" dump file forward with open file number %d\n", ofn);

	i = st_firstfile(ofn, &rid1);
	if (i < eNOERROR)
	{
		printf(" empty file\n");
		return;
	}

	for(; i >= 0; rid1 = rid2)
	{	
		for (j = 0; j < LONG; buf[j++] = ' ');
		i = st_readrecord(ofn, &rid1, buf, LONG);
		STERROR("test17/st_readrecord", i);
		PRREC(buf);	/* print the record */

		i = st_nextfile(ofn, &rid1, &rid2); 
	}
}
	

dumpfile_backward(ofn)
int	ofn;
{
	int	i,j;
	RID	rid1, rid2;
	char	buf[LONG];

	printf(" dump file backward with open file number %d\n", ofn);

	i = st_lastfile(ofn, &rid1);
	if (i < eNOERROR)
	{
		printf(" empty file\n");
		return;
	}

	for(; i >= 0; rid1 = rid2)
	{	
		i = st_readrecord(ofn, &rid1, buf, LONG);
		STERROR("test17/st_readrecord", i);
		PRREC(buf);	/* print the record */

		i = st_prevfile(ofn, &rid1, &rid2); 
	}

}
	

main(argc,argv)
int	argc;
char	**argv;
{
	int i, j, k, vol, ofn;
	RID rid[NUMREC];	
	int	flag[NUMREC];
	char rec[NUMREC][ 1001];

/* initilaize test records */

	for (i = 0; i < NUMREC; i++)
	{
		sprintf(rec[i], "[record %d] (1000b)   ", i);
		for (j = SHORT; j < LONG-SHORT; j++)
			rec[i][j] = i + '0';
		sprintf(&rec[i][j], "   [end of record %d]", i);
	}

/* other initialization */

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test17/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test17/bf_init", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test17/st_init", i);

	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test17/st_mount", vol);

	i = st_createfile(vol, FILENAME, 9, 90,90);
	STERROR("test17/st_createfile", i);

/* testing */

	printf(" about to insert %d long records\n", NUMREC);

	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test17/st_openfile(for insertion)", ofn);

	for (i = 0; i < NUMREC; i++)
	{
		j = st_insertrecord(ofn, rec[i], LONG, NULL, &rid[i]);
		STERROR("test17/st_insertrecord", j);
	}

	printf(" after insertions, the order should be 0 - %d\n", NUMREC - 1);
	i = st_closefile(ofn);
	STERROR("test17/st_closefile(for insertion)", i);

	DUMPFILE;

	printf(" # of records %d, # of pages %d\n", 
		st_recordcard(vol, FILENAME), st_filepages(vol, FILENAME));

	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test17/st_openfile(for deletion)", ofn);

	for (i = 0; i < NUMREC; i++)
		flag[i] = 0;
	for (i = 0; i < NUMREC; i++)
	{
		j = rand() % NUMREC;
		if (flag[j])
		{ /* find another */
			for (k = j - 1; k >= 0; k--) if (!flag[k]) break;
			if (k < 0)
			  for (k = j + 1; k < NUMREC; k++) if (!flag[k]) break;
			j = k;
		}

		flag[j] = 1;
		j = st_deleterecord(ofn, &rid[j]);
		STERROR("test17/st_deleterecord", j);

	}

	i = st_closefile(ofn);
	STERROR("test17/st_closefile(for insertion)", i);

	printf(" after deletions, there should be no records\n");

	DUMPFILE;

	printf(" # of records %d, # of pages %d\n", 
		st_recordcard(vol, FILENAME), st_filepages(vol, FILENAME));

	i = st_destroyfile(vol, FILENAME);
	STERROR("test17/st_destroyfile", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test17/st_dismount", i);

	printf("That's all folks\n");

}
