
/* program to test some level 3 routines */
/* and exercise scan table routines      */ 	

#include <wiss.h>
#include <lockquiz.h>
#include <locktype.h>

/* inserted by LSM */
static  KEYINFO keyattr =
                               { 0, sizeof(int), TINTEGER};

static  KEY     key =
                                { TINTEGER, "",  sizeof(int)};

#define	INDEXNO	0
	
#define	WISSERROR(p,c)	if((int)(c)<0) am_fatalerror(p,(int)(c))

main(argc,argv)
int	argc;
char	**argv;
{
	int	i, j, vol;
	int 	ofn[4], scanid[20];
	RID 	rid[10];	
	static char	*TESTFILE[] = {"tfile1","tfile2","tfile3","tfile4"};
	int	trans_id,e;
	int	indexfile_number ;

/* initialization */

	wiss_checkflags(&argc,&argv);

	 (void) wiss_init();			/* initialize level 3 */

        trans_id = begin_trans();               /* inserted by LSM */
        printf("new transaction id = %d\n",trans_id);

	vol = st_mount("wiss3");  	/* mount the device wiss3 */
	WISSERROR("test2/st_mount", vol);

/* create files */

	printf(" about to create 4 files\n");
	for (i = 0; i < 3; i++)
	{
		j = st_createfile(vol, TESTFILE[i], 1, 90, 90);
		WISSERROR("test2/st_createfile", j);
	}

	e = st_createindex(vol,TESTFILE[1],INDEXNO,&keyattr,100,FALSE, FALSE, trans_id, TRUE, FALSE);  /* inserted by LSM */ 
        WISSERROR("test8/wiss_createindex", e);

/* testing */

/* 1. open and close scans on one file */

	ofn[0] = am_openfile(vol, TESTFILE[0], WRITE);
	WISSERROR("test2/am_openfile", ofn[0]);

	printf(" about to open 10 scans on file %d\n", ofn[0]);
	for (i = 0; i < 10; i++)
	{

              /* lock the file in IX mode */
	        j = wiss_lock_file(trans_id, ofn[0], l_IX, COMMIT, FALSE);
       	 	WISSERROR("test1/lockfile",j);
		scanid[i] = am_openfilescan(ofn[0], NULL, trans_id, TRUE, l_IX, FALSE);
		WISSERROR("test2/am_openfilescan", scanid[i]);
	}
	printf(" 10 scans on file %d opened\n", ofn[0]);
	AM_dumpscantable();

	printf(" about to delete scans %d, %d, %d\n", 
			  scanid[0], scanid[4], scanid[9]);
	j = am_closescan(scanid[0]);
	WISSERROR("test2/am_closescan", j);
	j = am_closescan(scanid[4]);
	WISSERROR("test2/am_closescan", j);
	j = am_closescan(scanid[9]);
	WISSERROR("test2/am_closescan", j);
	AM_dumpscantable();

	printf(" open 5 more scans on file %d\n", ofn[0]);
	for (i = 10; i < 15; i++)
	{

              /* lock the file in IX mode */
        	j = wiss_lock_file(trans_id, ofn[0], l_IX, COMMIT, FALSE);
        	WISSERROR("test1/lockfile",j);

		scanid[i] = am_openfilescan(ofn[0], NULL, trans_id, TRUE, l_IX, FALSE);
		WISSERROR("test2/am_openfilescan", scanid[i]);
	}
	AM_dumpscantable();

	printf(" close file %d with some leftovers\n", ofn[0]);
	i = am_closefile(ofn[0]);
	WISSERROR("test2/am_closefile", i);
	AM_dumpscantable();

/* 2. test different scans on several files */

	printf(" about to open 4 different files\n");

	for (i = 0; i < 3; i++)
	{
		ofn[i] = am_openfile(vol, TESTFILE[i], WRITE);
		WISSERROR("test2/am_openfile(for insertion)", ofn[i]);
	}
        /* open the wiss index file with WRITE permission */
        ofn[3] = am_openindex(vol, TESTFILE[1], INDEXNO, WRITE);
        WISSERROR("test8/am_openindex", ofn[3]);



	printf(" about to create 4 long data item on file %d\n", ofn[1]);
	for (i = 0; i < 4; i++)
	{
		j = st_createlong(ofn[1], &rid[i], trans_id, TRUE, FALSE);
		WISSERROR("test2/st_createlong", j);
	}

	printf(" about to create 5 scans on each file\n");
	for (j = 0; j < 20; j+=4)
	{

              /* lock the file in IX mode */
        	i = wiss_lock_file(trans_id, ofn[0], l_IX, COMMIT, FALSE);
	        WISSERROR("test1/lockfile",i);

		scanid[j] = am_openfilescan(ofn[0], NULL, trans_id, TRUE, l_IX, FALSE);
		WISSERROR("test2/am_openfilescan", scanid[j]);
		
        	i = wiss_lock_file(trans_id, ofn[1], l_IX, COMMIT, FALSE);
	        WISSERROR("test1/lockfile",i);

		scanid[j+1] = am_openlongscan(ofn[1], &rid[j / 4], trans_id, TRUE, l_IX, FALSE);
		WISSERROR("test2/am_openlongscan", scanid[j+1]);

        	i = wiss_lock_file(trans_id, ofn[2], l_IX, COMMIT, FALSE);
	        WISSERROR("test1/lockfile",i);

		scanid[j+2] = am_openfilescan(ofn[2], NULL, trans_id, TRUE, l_IX, FALSE);
		WISSERROR("test2/am_openfilescan", scanid[j+2]);


        	i = wiss_lock_file(trans_id, ofn[3], l_IX, COMMIT, FALSE);
	        WISSERROR("test1/lockfile",i);
		scanid[j+3] = am_openindexscan(ofn[0],ofn[3],
				keyattr, NULL, NULL, NULL, trans_id, TRUE, l_IX, FALSE);
		WISSERROR("test2/am_openindexscan", scanid[j+3]);
	
		printf(" open sequential scans %d and %d on file %d and %d",
			 scanid[j], scanid[j+2], ofn[0], ofn[2]);
		printf(" , long scans %d on file %d", scanid[j+1], ofn[1]); 
		printf(" , index scans %d on file %d\n", scanid[j+3], ofn[3]); 
	}
	AM_dumpscantable();

	printf(" about to delete 10 scans :");
	for ( i = 5; i < 15; i++)
		printf(" %d,", scanid[i]);
	printf("\n");

	for ( i = 5; i < 15; i++)
	{
		j = am_closescan(scanid[i]);
		WISSERROR("test2/am_closescan", j);
	}
	AM_dumpscantable();

	printf(" close file %d first \n", ofn[2]);
	j = am_closefile(ofn[2]);
	WISSERROR("test2/am_closefile", j);
	AM_dumpscantable();

	printf(" close all files\n");
	for ( i = 0; i < 4; i++)
	{
		if (i== 2) i++;
		j = am_closefile(ofn[i]);
		WISSERROR("test2/am_closefile", j);
	}
	AM_dumpscantable();

        /*  drop the index */
	e = st_dropbtree(vol, TESTFILE[1], INDEXNO, trans_id, TRUE, FALSE);
        WISSERROR("test8/st_dropbtree", e);

	for ( i = 0; i < 3; i++)
	{
		j = st_destroyfile(vol, TESTFILE[i], trans_id, TRUE, FALSE);
		WISSERROR("test2/st_destroyfile", j);
	}


         i = commit_trans(trans_id);
         if (i != 1)
            printf("error status return from commit_trans = %d\n", i);
         else printf("commit ok\n");


	i = st_dismount("wiss3");  /* dismount the device wiss3 */
	WISSERROR("test2/st_dismount", i);

        (void) wiss_final();
	printf(" That's all folks\n");
}
