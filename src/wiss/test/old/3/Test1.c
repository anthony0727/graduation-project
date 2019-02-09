/* program to test record routines */
/* 	
   This is only a simple test to the following routines, and no boolean
    expressions for opening scans.
	am_init
	am_openfile
	am_closefile
	am_openfilescan
	am_closescan
	am_insertscan
	am_fetchfirst
	am_fetchnext
	am_fetchprev
	am_fetchlast
	
*/

#include <wiss_r.h>
#include <wiss.h>
#include <lockquiz.h>
#include <locktype.h>

#define SHORT		20
#define LONG		1000
#define	WISSERROR(p,c)	if((int)(c)<0) am_fatalerror(p,(int)(c))
#define	ERROR(p,c)	if((int)(c)<0) am_error(p,(int)(c))
#define PRREC(rec) 	if (*(rec+12) == 's') printf("%20.20s\n", rec);\
			else printf("%20.20s...%20.20s\n",rec,rec +980)
#define TESTFILE	"tfile1"

dumpfile(vol,bp,trans_id)
int	vol;
BOOLEXP	*bp;
int     trans_id;

{
	int	ofn;
	int 	scanid;
	int	i;

	ofn = am_openfile(vol, TESTFILE, READ);
	WISSERROR("test1/am_openfile", ofn);

        i = wiss_lock_file(trans_id, ofn, l_IS, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	scanid = am_openfilescan(ofn,bp,trans_id,TRUE,l_S,FALSE); 
	WISSERROR("test1/am_openfilescan", scanid);

	dumpfile_forward(ofn,scanid,trans_id,TRUE,l_S,FALSE);
	dumpfile_backward(ofn,scanid,trans_id,TRUE,l_S,FALSE);
	i = am_closescan(scanid);
	WISSERROR("test1/am_closescan", i);

	i = am_closefile(ofn);
	WISSERROR("test1/am_closefile", i);
}


dumpfile_forward(ofn,scanid,trans_id,lockup,mode,cond)
int	ofn;			/* open file number */
int	scanid;			/* index into scan table */
int	trans_id,lockup,mode,cond;
{
	int	i,j;
	RID	rid1, rid2;
	char	buf[LONG];

	printf("dumpfile_forward - ofn = %d, scanid = %d\n", ofn, scanid);

	i = am_fetchfirst(scanid, &rid1);
	WISSERROR("test1/am_fetchfirst", i);

	for(; i >= 0;)
	{	
		for (j = 0; j < LONG; buf[j++] = ' ');
		i = st_readrecord(ofn, &rid1, buf, LONG,trans_id,lockup,mode,cond);
		WISSERROR("test1/st_readrecord", i);
		PRREC(buf);	/* print the record */

		i = am_fetchnext(scanid, &rid1);
	}

	ERROR("test1/am_fetchnext", i);

} /* dumpfile_forward */
	
/*	This procedure prints all the records in the scan whose scanid
**	is passed in as a parameter, starting from the end of the file
**	and going backwards to the beginning.  If the Boolean expression
**	is null, it will be vacuously true for all records, and the entire
**	file will be printed.
*************************/


dumpfile_backward(ofn,scanid,trans_id,lockup,mode,cond)
int	ofn;			/* open file number */
int	scanid;			/* index into scan table */
int     trans_id,lockup,mode,cond;

{
	int	i,j;
	RID	rid1, rid2;
	char	buf[LONG];

	printf("dumpfile_backward - ofn = %d, scanid = %d\n", ofn, scanid);


	i = am_fetchlast(scanid, &rid1);
	WISSERROR("test1/am_fetchlast", i);

	for(; i >= 0;)
	{	
		for (j = 0; j < LONG; buf[j++] = ' ');
		i = st_readrecord(ofn, &rid1, buf, LONG,trans_id,lockup,mode,cond);
		WISSERROR("test1/st_readrecord", i);
		PRREC(buf);	/* print the record */

		i = am_fetchprev(scanid, &rid1);
	}

	ERROR("test1/am_fetchprev", i);

}


main(argc,argv)
int	argc;
char	**argv;
{
	int i, j, vol, ofn, scanid, trans_id;
	RID rid[10];	
	char rec[10][ 1001];

/* initilaize test records */

	for (i = 0; i < 10; i++)
	{
		sprintf(rec[i], "[record %d] (short)   ", i);
		for (j = SHORT; j < LONG-SHORT; j++)
			rec[i][j] = i + '0';
		sprintf(&rec[i][j], "   [end of record %d]", i);
	}

/* other initialization */


	wiss_checkflags(&argc,&argv);

	(void) wiss_init();			/* initialize level 3 */

	trans_id = begin_trans();            	/* inserted by LSM */
	printf("new transaction id = %d\n",trans_id);

	vol = st_mount("wiss3");  	/* mount the device wiss3 */
	WISSERROR("test1/st_mount", vol);

	i = st_createfile(vol, TESTFILE, 9, 90,90);
	WISSERROR("test1/st_createfile", i);

	i = st_dismount("wiss3");  /* dismount the device wiss3 */
	WISSERROR("test1/st_dismount", i);

/* testing */

/* 1. insert 10 short records */
	printf(" about to insert 10 short records\n");

	vol = st_mount("wiss3");  	/* mount the device wiss3 */
	WISSERROR("test1/st_mount", vol);

	ofn = am_openfile(vol, TESTFILE, WRITE);
	WISSERROR("test1/am_openfile(for insertion)", ofn);

	      /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, ofn, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	scanid = am_openfilescan(ofn, NULL,trans_id,TRUE, l_IX,  FALSE);
	WISSERROR("test1/am_openfilescan(for insertion)", scanid);

	for (i = 0; i < 10; i++)
	{
		j = am_insertscan(scanid, rec[i], SHORT, &rid[i]);
		WISSERROR("test1/am_insertscan", j);
	}

	printf(" after insertions, the order should be 0 - 10\n");

	i = am_closescan(scanid);
	WISSERROR("test1/am_closescan(for insertion)", i);

	i = am_closefile(ofn);
	WISSERROR("test1/am_closefile(for insertion)", i);

	dumpfile(vol,NULL,trans_id);

	i = st_dismount("wiss3");  /* dismount the device wiss3 */
	WISSERROR("test1/st_dismount", i);

/* 2. delete 5 short records */
	printf(" about to delete 5 short records\n");

	vol = st_mount("wiss3");  	/* mount the device wiss3 */
	WISSERROR("test1/st_mount", vol);

	ofn = am_openfile(vol, TESTFILE, WRITE);
	WISSERROR("test1/am_openfile(for deletion)", ofn);

        i = wiss_lock_file(trans_id, ofn, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	scanid = am_openfilescan(ofn, NULL,trans_id,TRUE,l_IX,FALSE);
	WISSERROR("test1/am_openfilescan(for deletion)", scanid);

	j = am_fetchfirst(scanid, NULL);
	WISSERROR("test1/am_fetchfirst(for deletion)", j);

	for (i = 0; i < 5; i++)
	{
		j = am_deletescan(scanid, &rid[i]);
		WISSERROR("test1/am_deletescan", j);

		j = am_fetchnext(scanid, NULL);
		WISSERROR("test1/am_fetchnext(for deletion)", j);

	}

	i = am_closescan(scanid);
	WISSERROR("test1/am_closescan(for deletion)", i);

	i = am_closefile(ofn);
	WISSERROR("test1/am_closefile(for deletion)", i);

	printf(" after deletions, the order should be 5 - 9\n");
	dumpfile(vol,NULL,trans_id);

	i = st_destroyfile(vol,TESTFILE);
	WISSERROR("test1/st_destroyfile", i);

	 /* now commit the transaction */
         i = commit_trans(trans_id);
         if (i != 1)
            printf("error status return from commit_trans = %d\n", i);
         else printf("commit ok\n");

	i = wiss_dismount("wiss3");  /* dismount the device wiss3 */

	(void) wiss_final();
	printf(" I think that's about it for the first test\n");
}
