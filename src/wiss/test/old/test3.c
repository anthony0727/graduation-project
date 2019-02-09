
/********************************************************/
/*							*/
/*		Wisconsin Storage System		*/
/*		Version 2.0 July, 1984			*/
/*							*/
/*		COPYRIGHT (C) 1984			*/
/*		Computer Sciences Department		*/
/*		University of Wisconsin			*/
/*		Madison, WI 53706			*/
/*							*/
/********************************************************/


/********************************************************/
/*							*/
/*		Wisconsin Storage System		*/
/*		Version 2.0 July, 1984			*/
/*							*/
/*		COPYRIGHT (C) 1984			*/
/*		Computer Sciences Department		*/
/*		University of Wisconsin			*/
/*		Madison, WI 53706			*/
/*							*/
/********************************************************/


/* Test Program 3.

   This program reads the its own source file and stores it in a 
   long data item.
   It then compares both copies of the source file and reports
   any inconsistency.
   This test requires a wiss volume stored in a UNIX file called
   "wiss_disk."

*/


#include <wiss.h>
#include <wiss_r.h>
#include <lockquiz.h>
#include <locktype.h>


#undef NULL
#include <stdio.h>


/* error handler */
#define	WISSERROR(p,c)	if((int)(c)<0) wiss_fatalerror(p,(int)(c))
/* the name of the device we are testing */
#define DEVICENAME	"wiss_disk"
/* the name of the wiss file */
#define	TESTFILE	"text"


main(argc, argv)
int	argc;	
char	**argv;
{

	register i;		/* loop index */
	int	status;		/* for returned flags */
	int	vol;		/* volume id */
	int	filenum;	/* open file number */
	int	scanid;		/* scan ID */
	int	byte_count;	/* byte count */
	int	total;		/* accumulated byte count */
	char	line[256];	/* line buffer */
	char	bytes[256];	/* line buffer */
	RID	dir;		/* directory of the long item */
	FILE	*fp, *fopen();	/* for the  UNIX file */
	int	trans_id;

	/* warm up */
	(void) wiss_init();

        trans_id = begin_trans();               /* inserted by LSM */
        printf("new transaction id = %d\n",trans_id);


	/* mount the device called DEVICENAME */
	vol = wiss_mount(DEVICENAME);	
	WISSERROR("test3/wiss_mount", vol);

	/* create a wiss file which has 6 pages initially, both the 
	   extent fill factor and page fill factor are 90%         
	*/
	status = wiss_createfile(vol, TESTFILE, 6, 90, 90);
	WISSERROR("test3/wiss_createfile", status);

	/* open the file where the text is to be stored */
	filenum = wiss_openfile(vol, TESTFILE, WRITE);
	WISSERROR("test3/wiss_openfile", filenum);

/* read the source and store it in a long data item */

	/* create a long data item */
	status = wiss_createlong(filenum, &dir, trans_id, TRUE, FALSE);
	WISSERROR("test3/wiss_createlong", status);
        i = wiss_lock_file(trans_id, filenum, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a scan on the long data item */
	scanid = wiss_openlongscan(filenum, &dir,trans_id,TRUE,l_IX,FALSE);
	WISSERROR("test3/wiss_openlongscan", scanid);

	if ((fp = fopen("test3.c", "r")) == NULL)
	{
		printf(" can't open %s\n", "test3.c");
		exit(-1);
	}

	total = 0;
	while(fgets(line, 256, fp) != NULL)
	{
		byte_count = strlen(line);
		status = wiss_insertlong(scanid, FALSE, line, byte_count);
		WISSERROR("test3/wiss_insertlong", status);
		
		total += byte_count;
		status = wiss_setcursor(scanid, total, 0);
		WISSERROR("test3/wiss_setcursor", status);
	}

	fclose(fp);

	printf(" %d bytes read\n", total);
	
	/* close the scan */
	status = wiss_closescan(scanid);
	WISSERROR("test1/wiss_closescan", status);


/* read the source again, and compare it with the stored copy */

        i = wiss_lock_file(trans_id,filenum, l_IX, COMMIT, FALSE);
        WISSERROR("test1/lockfile",i);

	/* open a scan on the long data item */
	scanid = wiss_openlongscan(filenum, &dir,trans_id,TRUE,l_IX,FALSE);
	WISSERROR("test3/wiss_openlongscan", scanid);

	if ((fp = fopen("test3.c", "r")) == NULL)
	{
		printf(" can't open %s\n", "test3.c");
		exit(-1);
	}

	total = 0;
	while(fgets(line, 256, fp) != NULL)
	{
		byte_count = strlen(line);
		status = wiss_readlong(scanid, bytes, byte_count);
		WISSERROR("test3/wiss_insertlong", status);

		for (i = 0; i < byte_count; i++)
			if (line[i] != bytes[i])
			{
				printf(" errors in long item scan\n");
				exit(-1);
			}
		
		total += byte_count;
		status = wiss_setcursor(scanid, total, 0);
		WISSERROR("test3/wiss_setcursor", status);
	}

	fclose(fp);

	printf(" %d bytes examined \n", total);

	/* close the scan */
	status = wiss_closescan(scanid);
	WISSERROR("test1/wiss_closescan", status);


	/* close the file */
	status = wiss_closefile(filenum);
	WISSERROR("test1/wiss_closefile", status);


	/* remove the file since the test in done */
	status = wiss_destroyfile(vol, TESTFILE,trans_id,TRUE,FALSE);
	WISSERROR("test3/wiss_destroyfile", status);

         /* now commit the transaction */
         i = commit_trans(trans_id);
         if (i != 1)
            printf("error status return from commit_trans = %d\n", i);
         else printf("commit ok\n");


	/* dismount the device */
	status = wiss_dismount(DEVICENAME);  
	WISSERROR("test3/wiss_dismount", status);

	(void) wiss_final();


} /* end of main */

