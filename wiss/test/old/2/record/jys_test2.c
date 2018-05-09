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

#define	BUFTRACE	1

#include <wiss.h>

#define SHORT		20
#define LONG		1000

extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n", p, io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
	{printf("%s %s\n", p, bf_error((int)(c)));bf_final(); io_final(); exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final(); exit(-1);}


#define PRREC(rec) 	if (*(rec+12) == 's') printf("%20.20s\n", rec);\
			else printf("%20.20s...%20.20s\n",rec,rec +980)
#define FILENAME	"tfile2"
#define DUMPFILE	ofn = st_openfile(vol, FILENAME, READ);\
			STERROR("test2/st_openfile", ofn);\
			dumpfile_forward(ofn);\
			i = st_closefile(ofn);\
			STERROR("test2/st_closefile", i)

dumpfile_forward(ofn)
int	ofn;
{
	int	i,j;
	RID	rid1, rid2;
	char	buf[LONG];

#ifdef	BUFTRACE
	BF_dumpfixed();
#endif
	printf(" Dump file %d (forward)\n", ofn);
	i = st_firstfile(ofn, &rid1);
	if (i < eNOERROR) {
		printf("  This is an empty file\n");
		return;
	}

	for(; i >= 0; rid1 = rid2) {	
		for (j = 0; j < LONG; buf[j++] = ' ');
		i = st_readrecord(ofn, &rid1, buf, LONG);
		STERROR("test2/st_readrecord", i);
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

#ifdef	BUFTRACE
	BF_dumpfixed();
#endif
	printf(" Dump file %d (backward)%d\n", ofn);
	i = st_lastfile(ofn, &rid1);
	if (i < eNOERROR) {
		printf("  This an empty file\n");
		return;
	}

	for(; i >= 0; rid1 = rid2) {	
		i = st_readrecord(ofn, &rid1, buf, LONG);
		STERROR("test2/st_readrecord", i);
		PRREC(buf);	/* print the record */
		i = st_prevfile(ofn, &rid1, &rid2); 
	}
}
	

main(argc,argv)
int	argc;
char	**argv;
{
	int i, j, vol, ofn;
	RID rid[10];	
	char rec[10][ 1001];

/* initilaize test records */

	for (i = 0; i < 10; i++) {
		sprintf(rec[i], "[record %d] (short)   ", i);
		for (j = SHORT; j < LONG-SHORT; j++)
			rec[i][j] = i + '0';
		sprintf(&rec[i][j], "   [end of record %d]", i);
	}

/* other initialization */

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test2/io_init", i);
	i = bf_init();		/* initialize level 1 */
	BFERROR("test2/bf_init", i);
	i = st_init();			/* initialize level 2 */
	STERROR("test2/st_init", i);
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test2/st_mount", vol);
/*	dump_directory(vol); */
	i = st_createfile(vol, FILENAME, 9, 90,90);
	STERROR("test2/st_createfile", i);
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test2/st_dismount", i);

/* testing */

/* 1. insert 10 short records */
	printf("- About to insert 10 short records\n");
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test2/st_mount", vol);
	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test2/st_openfile(for insertion)", ofn);
	for (i = 0; i < 10; i++) {
		j = st_insertrecord(ofn, rec[i], SHORT, NULL, &rid[i]);
		STERROR("test2/st_insertrecord", j);
	}

	printf(" after insertions, the order should be 0 - 9\n");
	i = st_closefile(ofn);
	STERROR("test2/st_closefile(for insertion)", i);
	DUMPFILE;
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test2/st_dismount", i);

/* 2. delete 5 short records */
	printf("- About to delete 5 records : 0,2,4,6,8\n");
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test2/st_mount", vol);
	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test2/st_openfile(for deletion)", ofn);
	for (i = 0; i < 10; i = i + 2) {
		j = st_deleterecord(ofn, &rid[i]);
		STERROR("test2/st_deleterecord", j);
	}
	printf(" after deletions, the order should be 1,3,5,7,9\n");
	i = st_closefile(ofn);
	STERROR("test2/st_closefile(for deletion)", i);
	DUMPFILE;
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test2/st_dismount", i);

/* 3. append 5 short records */
	printf("- About to append 5 short records : 0,2,4,6,8\n");
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test2/st_mount", vol);
	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test2/st_openfile(for append)", ofn);
	for (i = 0; i < 10; i = i + 2) {
		j = st_appendrecord(ofn, rec[i], SHORT, &rid[i]);
		STERROR("test2/st_appendrecord", j);
	}
	printf(" the order should be 1,3,5,7,9,0,2,4,6,8\n");
	i = st_closefile(ofn);
	STERROR("test2/st_closefile(for append)", i);
	DUMPFILE;
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test2/st_dismount", i);

/* 4. expand(write) 5 short records to long records */
	printf("- About to expand 5 records : 1,3,5,7,9\n");
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test2/st_mount", vol);
	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test2/st_openfile(for write)", ofn);
	for (i = 1; i <=  9; i = i + 2) {
		sprintf(&rec[i][12], "long");
		rec[i][16] = ' ';
		j = st_writerecord(ofn, &rid[i], rec[i], LONG);
		STERROR("test2/st_writerecord", j);
	}
	printf(" 1,3,5,7,9 should be long records\n");
	i = st_closefile(ofn);
	STERROR("test2/st_closefile(for write)", i);
	DUMPFILE;
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test2/st_dismount", i);

/* 5. expand(write) 5 short records to long records */
	printf("- About to expand 5 records : 0,2,4,6,8\n");
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test2/st_mount", vol);
	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test2/st_openfile(for write)", ofn);
	for (i = 0; i < 10; i = i + 2) {
		sprintf(&rec[i][12], "long");
		rec[i][16] = ' ';
		j = st_writerecord(ofn, &rid[i], rec[i], LONG) ;
		STERROR("test2/st_writerecord", j);
	}
	printf("all should be long records\n");
	i = st_closefile(ofn);
	STERROR("test2/st_closefile(for write)", i);
	DUMPFILE;
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test2/st_dismount", i);

/* 6. shrink(write) 5 short records to long records */
	printf("- About to shrink 5 records : 0,2,4,6,8\n");
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test2/st_mount", vol);
	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test2/st_openfile(for write)", ofn);
	for (i = 0; i < 10; i = i + 2) {
		sprintf(&rec[i][12], "short");
		rec[i][17] = ')';
		j = st_writerecord(ofn, &rid[i], rec[i], SHORT) ;
		STERROR("test2/st_writerecord", j);
	}
	printf("0,2,4,6,8 should be short\n");
	i = st_closefile(ofn);
	STERROR("test2/st_closefile(for write)", i);
	DUMPFILE;
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test2/st_dismount", i);

/* 7. delete all records */
	printf("- About to delete all records\n");
	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test2/st_mount", vol);
	ofn = st_openfile(vol, FILENAME, WRITE);
	STERROR("test2/st_openfile(for deletion)", ofn);
	for (i = 0; i < 10; i++) {
		j = st_deleterecord(ofn, &rid[i]);
		STERROR("test2/st_deleterecord", j);
	}
	printf(" after deletions, there should be no records left\n");
r_dumpfile(ofn);
	i = st_closefile(ofn);
	STERROR("test2/st_closefile(for deletion)", i);
	DUMPFILE;
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test2/st_dismount", i);

/* finalization */

	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test2/st_mount", vol);
	i = st_destroyfile(vol, FILENAME);
	STERROR("test2/st_destroyfile", i);
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test2/st_dismount", i);

	printf("That's all folks\n");

}
