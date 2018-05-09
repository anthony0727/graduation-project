
/* This program is for test3ing boolean expression routine - AM_apply */

/* In this test3, records are set up such that some fields are not properly
   aligned on word boundary.
   Simple expressions consist of a single term are test3ed first.
   Then, an expression that contains all data type is test3ed.
*/

#include <wiss.h>
#include <lockquiz.h>
#include <locktype.h>

#define SHORT		20
#define XLONG		1000
#define INTOFFSET	0
#define STRINGOFFSET	sizeof(int)
#define SHORTOFFSET	STRINGOFFSET+ 5 * sizeof(char)
#define FLOATOFFSET	SHORTOFFSET+sizeof(short)
#define LONGOFFSET	FLOATOFFSET+sizeof(float)
#define DOUBLEOFFSET	LONGOFFSET+sizeof(long)
#define	BUFLEN		DOUBLEOFFSET+sizeof(double)
#define	WISSERROR(p,c)	if((int)(c)<0) am_fatalerror(p,(int)(c))
#define	ERROR(p,c)	if((int)(c)<0) am_error(p,(int)(c))
#define PRREC(rec) 	{ float f; double d;char *p;\
			p = (char *)(&f);\
			movebytes(p, &rec[FLOATOFFSET],sizeof(float));\
			p = (char *)&d;\
			movebytes(p, &rec[DOUBLEOFFSET],sizeof(double));\
			printf("|%4.4d|%5.5s|%2.2d|%4.0f|%4.4ld|%8.0lf|\n", \
				(int)rec[INTOFFSET], &rec[STRINGOFFSET], \
				(short)rec[SHORTOFFSET],\
				f, (long)rec[LONGOFFSET], d);}
#define TESTFILE	"tfile1"

dump_forward(vol, filename, bp, trans_id)
int	vol;
char	*filename;
BOOLEXP	*bp;
int	trans_id;
{
	int	ofn;
	int	scanid;
	int	i,j;
	char	buf[XLONG];

	ofn = am_openfile(vol, filename, READ);
	WISSERROR("test3/am_openfile", ofn);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, ofn, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	scanid = am_openfilescan(ofn, bp, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test3/am_openfilescan", scanid);

	printf("dump file forward - ofn = %d, scanid = %d\n", ofn, scanid);

	i = am_fetchfirst(scanid, NULL,AND);
	ERROR("test3/am_fetchfirst", i);

	for(; i >= 0;)
	{	
		for (j = 0; j < XLONG; buf[j++] = ' ');
		i = am_readscan(scanid, buf, XLONG);
		WISSERROR("test3/am_readscan", i);
		PRREC(buf);	/* print the record */

		i = am_fetchnext(scanid, NULL, AND);
	}

	ERROR("test3/am_fetchnext", i);

	i = am_closefile(ofn);
	WISSERROR("test3/am_closefile", i);

} /* dump_forward */
	

dump_backward(vol, filename, bp, trans_id)
int	vol;
char	*filename;
BOOLEXP	*bp;
int	trans_id;
{
	int	i,j;
	RID	rid1, rid2;
	char	buf[XLONG];
	int	ofn;
	int	scanid;

	ofn = am_openfile(vol, filename, READ);
	WISSERROR("test3/am_openfile", ofn);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, ofn, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	scanid = am_openfilescan(ofn, bp, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test3/am_openfilescan", scanid);

	printf("dump file backward - ofn = %d, scanid = %d\n", ofn, scanid);

	i = am_fetchlast(scanid, &rid1, AND);
	ERROR("test3/am_fetchlast", i);

	for(; i >= 0;)
	{	
		for (j = 0; j < XLONG; buf[j++] = ' ');
		i = st_readrecord(ofn, &rid1, buf, XLONG, trans_id, TRUE, l_X, FALSE);
		WISSERROR("test3/st_readrecord", i);
		PRREC(buf);	/* print the record */

		i = am_fetchprev(scanid, &rid1, AND);
	}

	ERROR("test3/am_fetchprev", i);

	i = am_closefile(ofn);
	WISSERROR("test3/am_closefile", i);
}

main(argc,argv)
int	argc;
char	**argv;
{
	int i, j, vol, ofn, scanid;
	int	trans_id;
	RID rid[10];	
	char rec[10][BUFLEN];
	int temp_int;
	short temp_short;
	float temp_float;
	long temp_long;
	double temp_double;
	char tempbuf[2 * BUFLEN];
static 	BOOLEXP Bool_expr[6] =
	{
		{LE, {INTOFFSET, sizeof(int), TINTEGER}, NULL, "\0"}, 
		{NE, {STRINGOFFSET, 5, TSTRING}, NULL, "\0"}, 
		{LT, {SHORTOFFSET, sizeof(short), TSHORT}, NULL, "\0"}, 
		{GE, {FLOATOFFSET, sizeof(float), TFLOAT}, NULL, "\0"}, 
		{NE, {LONGOFFSET, sizeof(long), TLONG}, NULL, "\0"}, 
		{GT, {DOUBLEOFFSET, sizeof(double), TDOUBLE}, NULL, "\0"}, 
	 };
	char *p;

/* initialize test3 records */

	for (i = 0; i < 10; i++)
	{
		temp_int = i;
		temp_short = (short) i/2;
		temp_float = (float) (i + 1);
		temp_long = (long) (i * 2);
		temp_double = (double) (i * i);
		p = &rec[i][INTOFFSET];
		movebytes(p, (char *) &temp_int, sizeof(int));
		sprintf(&(rec[i][STRINGOFFSET]),"key-%d", i);
		p = &rec[i][SHORTOFFSET];
		movebytes(p, (char *) &temp_short, sizeof(short));
		p = &rec[i][FLOATOFFSET];
		movebytes(p, (char *) &temp_float, sizeof(float));
		p = &rec[i][LONGOFFSET];
		movebytes(p, (char *) &temp_long, sizeof(long));
		p = &rec[i][DOUBLEOFFSET];
		movebytes(p, (char *) &temp_double, sizeof(double));
	}

	printf(" the record format used in this test3 is (type:length) : \n");
	printf("(int:4), (char:5), (short:2), (float:4), (long:4), (double:8)\n");


/* other initialization */

	wiss_checkflags(&argc,&argv);

        (void) wiss_init();                     /* initialize level 3 */

        trans_id = begin_trans();               /* inserted by LSM */
        printf("new transaction id = %d\n",trans_id);

	vol = st_mount("wiss3");  	/* mount the device wiss3 */
	WISSERROR("test3/st_mount", vol);


	i = st_createfile(vol, TESTFILE, 9, 90,90);
	WISSERROR("test3/st_createfile", i);

	ofn = am_openfile(vol, TESTFILE, WRITE);
	WISSERROR("test3/am_openfile(for insertion)", ofn);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, ofn, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	scanid = am_openfilescan(ofn, NULL, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test3/am_openfilescan(for insertion)", scanid);

	for (i = 0; i < 10; i++)
	{
		j = am_insertscan(scanid, rec[i], BUFLEN, &rid[i]);
		WISSERROR("test3/am_insertscan", j);
	}

	i = am_closescan(scanid);
	WISSERROR("test3/am_closescan(for insertion)", i);

	i = am_closefile(ofn);
	WISSERROR("test3/am_closefile(for deletion)", i);

	printf("the entire file is dumped...\n");
	dump_forward(vol, TESTFILE, NULL, trans_id);
	dump_backward(vol, TESTFILE, NULL, trans_id);

/* 2. build Boolean expressions  */

	*(int *)Bool_expr[0].value = 5;
	strcpy(Bool_expr[1].value, "key-7");
	*(short *)Bool_expr[2].value = 3;
	*(float *)Bool_expr[3].value = 6.0;
	*(long *)Bool_expr[4].value = 6;
	*(double *)Bool_expr[5].value = 4.5;

	for (i = 0; i < 6; i++)
	{
		printf(" try simple boolean expression : ");
		AM_dumpboolean(&Bool_expr[i]);
		printf("\n");

		dump_forward(vol, TESTFILE, &Bool_expr[i], trans_id);
	}


	for (i = 0; i < 5; i++)
		Bool_expr[i].next = sizeof(BOOLEXP);

	printf(" about to delete records satisfying Boolean expression :\n");
	AM_dumpboolean(Bool_expr);
	printf("\n");

	ofn = am_openfile(vol, TESTFILE, WRITE);
	WISSERROR("test3/am_openfile", ofn);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, ofn, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	scanid = am_openfilescan(ofn, Bool_expr, trans_id, TRUE, l_IX, FALSE);
	WISSERROR("test3/am_openfilescan(for deletion)", scanid);

	j = am_fetchfirst(scanid, NULL, AND);
	ERROR("test3/am_fetchfirst(for deletion)", j);

	if (j < 0)
		printf(" nothing to delete\n");
	else
	do
	{
		j = am_deletescan(scanid);
		WISSERROR("test3/am_deletescan", j);
	} while (( j = am_fetchnext(scanid, NULL, AND)) == eNOERROR);


	i = am_closescan(scanid);
	WISSERROR("test3/am_closescan(for deletion)", i);

	i = am_closefile(ofn);
	WISSERROR("test3/am_closefile(for deletion)", i);

	printf("after deletion\n");
	dump_forward(vol, TESTFILE, NULL, trans_id);
	dump_backward(vol, TESTFILE, NULL, trans_id);

	i = st_destroyfile(vol, TESTFILE, trans_id, TRUE, FALSE);
	WISSERROR("test3/st_destroyfile", i);

         /* now commit the transaction */
         i = commit_trans(trans_id);
         if (i != 1)
            printf("error status return from commit_trans = %d\n", i);
         else printf("commit ok\n");

	i = st_dismount("wiss3");  /* dismount the device wiss3 */
	WISSERROR("test3/st_dismount", i);

        (void) wiss_final();
	printf(" I think that's about it for this test3\n");
}
