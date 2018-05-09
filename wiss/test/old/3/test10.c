
#define		NUMREC	1500
#define		RECSIZE	50

#define		TESTFILE	"test10"
#define		DEVICE		"wiss3"
#define		WISS_ERROR(m,e)	if ((e) < eNOERROR) wiss_fatalerror(m,e)

#include	<wiss.h>
#include	<wiss_r.h>
#include	<lockquiz.h>
#include 	<locktype.h>

main(argc, argv)
int	argc;	
char	**argv;
{
	int	i, j, k;
	int	e;
	int	vol;			/* volume id */
	int	scanid;
	int	record[NUMREC];
	RID	rid;
	KEYINFO	keyattr;
	char	buf[RECSIZE];
	int	bool[NUMREC];
	int	trans_id;

	/* warm up procedure for wiss */
	
	wiss_checkflags(&argc,&argv);
	wiss_init();			/* initialize level 2 */

        trans_id = begin_trans();               /* inserted by LSM */
        printf("new transaction id = %d\n",trans_id);

	/* mount the volume call DEVICE */
	vol = wiss_mount(DEVICE);	
	WISS_ERROR("test10/wiss_mount", vol);

	/* set_sortbuf(10); */ /* 10 page buffer for sort */

	/* populate the database - fill the file with
           randomly (without replacement) generated data */

	e = wiss_createfile(vol, TESTFILE, 6, 90,90);
	WISS_ERROR("test10/wiss_createfile", e);

	j = wiss_openfile(vol, TESTFILE, WRITE);
	WISS_ERROR("test10/wiss_openfile", j);

	for (i = 0; i < NUMREC; i++) bool[i] = 0;
		
	for (i = 0; i < NUMREC; i++)
	{
		record[i] = rand() % NUMREC;
		if (bool[record[i]] == 1)
		{
			for (k = record[i]; bool[k] == 1; 
				k = (k == NUMREC - 1) ? 0 : k + 1);
			record[i] = k;
		}
		bool[record[i]] = 1;	/* avoid duplicates */

/*
		printf("record[%3d] = %3d, ", i, record[i]);
		if (i % 4 == 3) printf("\n");
*/

		*(int *) buf = record[i];
		e = wiss_insertrecord(j, buf, RECSIZE, NULL, &rid, trans_id, TRUE, FALSE);
		WISS_ERROR("test10/wiss_insertrecord", e);
		if (e < eNOERROR) break;

	}

	e = wiss_closefile(j);
	WISS_ERROR("test10/wiss_closefile", e);

	j = wiss_openfile(vol, TESTFILE, WRITE);
	WISS_ERROR("test10/wiss_openfile", j);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, j, l_IX, COMMIT, FALSE);
        WISS_ERROR("test1/lockfile",i);

	scanid = wiss_openfilescan(j, NULL, trans_id, TRUE, l_IX, FALSE);
	WISS_ERROR("test10/wiss_openfilescan", scanid);

	e = wiss_fetchfirst(scanid, NULL);
	WISS_ERROR("test10/wiss_fetchfirst", e);

	for ( i = 0; e >= eNOERROR; i++ )
	{
		e = wiss_readscan(scanid, &record[0], sizeof(int));
		WISS_ERROR("test10/wiss_readscan", e);
		
/*
		printf("record[%3d] = %3d, ", i, record[0]);
		if (i % 4 == 3) printf("\n");
*/

		e = wiss_fetchnext(scanid, NULL);
		if (e < eNOERROR) break;
	}

	e = wiss_closefile(j);
	WISS_ERROR("test10/wiss_closefile", e);

	keyattr.offset = 0;
	keyattr.type = TINTEGER;
	keyattr.length = sizeof(int); 

	e = wiss_sort(vol, TESTFILE, &keyattr, 1, trans_id, TRUE, FALSE);
	WISS_ERROR("test10/wiss_sort", e);

	printf(" After sorting : \n");

	j = wiss_openfile(vol, TESTFILE, WRITE);
	WISS_ERROR("test10/wiss_openfile", j);

              /* lock the file in IX mode */
        i = wiss_lock_file(trans_id, j, l_IX, COMMIT, FALSE);
        WISS_ERROR("test1/lockfile",i);

	scanid = wiss_openfilescan(j, NULL, trans_id, TRUE, l_IX, FALSE);
	WISS_ERROR("test10/wiss_openfilescan", scanid);

	e = wiss_fetchfirst(scanid, NULL);
	WISS_ERROR("test10/wiss_fetchfirst", e);

	for ( i = 0; e >= eNOERROR; i++ )
	{
		e = wiss_readscan(scanid, &record[i], sizeof(int));
		WISS_ERROR("test10/wiss_readscan", e);

		if (record[i] != i)
		{
			printf(" ** BUG in sort!\n");
			printf(" bug found in accessing record %d\n", i);
			exit(-1);
			break;
		}
		
/*
		printf("record[%3d] = %3d, ", i, record[0]);
		if (i % 4 == 3) printf("\n");
*/

		e = wiss_fetchnext(scanid, NULL);
		if (e < eNOERROR) break;

	}

	e = wiss_closefile(j);
	WISS_ERROR("test10/wiss_closefile", e);


	/* remove the file since the test in done */
	e = wiss_destroyfile(vol, TESTFILE, trans_id, TRUE, FALSE);
	WISS_ERROR("test10/wiss_destroyfile", e);

         /* now commit the transaction */
         i = commit_trans(trans_id);
         if (i != 1)
            printf("error status return from commit_trans = %d\n", i);
         else printf("commit ok\n");

	/* dismount the volume call DEVICE */
	e = wiss_dismount(DEVICE);  
	WISS_ERROR("test10/wiss_dismount", e);
        (void) wiss_final();

	printf(" All is well ! That's it!\n");

}
