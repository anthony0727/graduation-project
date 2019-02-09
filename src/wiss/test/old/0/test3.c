#include <wiss.h>

/* program to test space allocation within a volume */
/* Proper test requires that wiss0 contain 11 extents of 10 pages each*/
/* 1 extents are used for the volume header so 10 are available for data*/
/* This program tests io_freepage thoroughly */

extern	char	*io_error();
#define	DEVICENAME	"wiss0"
#define	IO_FatalError(p,c) if((int)(c)<0) \
		{printf("%s %s\n", p, io_error(c));io_final();exit(-1);}

main(argc,argv)
int	argc;
char	**argv;
{
	int e;
	int i,j,k;
	int volid;
	FID fileid[15];
	PID pids[20] [20]; /* i,j: i is for file, j is for pages  */

/* warm up */
	wiss_checkflags(&argc,&argv);
	e = io_init();  /* initialize level 0 */
	IO_FatalError("test3/io_init",e);
	volid = io_mount(DEVICENAME);  /* mount the device */
	IO_FatalError("test3/io_mount",volid);

	printf ("\n Device %s should have 11 extents of 10 pages each\n", 
		DEVICENAME);
	printf (" 1 is used for volume header, 10 for data\n");

/* Create the files */
	printf ("\n Create 5 files of 11 pages each\n");
	printf (" Each file will require two extents \n");
	for (j=0;j<5;j++)
	{
		e = io_createfile(volid, 11, 100, &fileid[j]);
		IO_FatalError("test3/io_createfile",e);
		e = io_allocpages(&(fileid[j]), NULL, 11, &pids[j][0]);
		IO_FatalError("test3/io_allocpage",e);
	}
	e = io_checker(volid);

/* Free some pages */
	printf (" Now free pages 0,2,4,6,8,10 file\n");
	for (j=0;j<5;j++)
	{
		for (i=0;i<12;i=i+2)
		{
			e=io_freepage(&fileid[j],&pids[j][i]);
			IO_FatalError("test3/io_freepage",e);
		}
	}
	printf("\n  6 pages should now have been freed in each file\n");
	printf("  Attempt to reallocate them in one shot\n");

/* Get those pages back */
	for (j=0;j<5;j++)
	{
		e = io_allocpages(&fileid[j], &pids[j][1], 6, &pids[j][10]);
		IO_FatalError("test3/io_freepage",e);
	}

/* Free all the pages */
	printf ("\n Now free all 11 pages in the file\n");
	printf (" Do pages 1,3,5,7,9 first\n");
	for (j=0;j<5;j++)
	{
		for (i=1;i<10;i=i+2)
		{
			e=io_freepage(&fileid[j],&pids[j][i]);
			IO_FatalError("test3/io_freepage",e);
		}
	}
	printf (" Then do pages the last 6 pages\n");
	for (j=0;j<5;j++)
	{
		for (i=10;i<16;i++)
		{
			e=io_freepage(&fileid[j],&pids[j][i]);
			IO_FatalError("test3/io_freepage",e);
		}
	}


/* Clean up the memory state, and test again */
	e = io_dismount(DEVICENAME);  /* unmount the device */
	IO_FatalError("test3/io_dismount",e);
	e = io_init();  /* initialize level 0 */
	IO_FatalError("test3/io_init",e);
	volid = io_mount(DEVICENAME);  /* mount the device */
	IO_FatalError("test3/io_mount",volid);


/* Allocate more for each file */
	printf("\n Now each file has no pages left across two extents\n"); 
	printf(" Try allocating 20 pages in one shot\n");
	for (j=0;j<5;j++)
	{
		e = io_allocpages(&fileid[j], NULL, 20, &pids[j][0]);
		IO_FatalError("test3/io_freepage",e);
	}

/* Free all the pages */
	printf ("\n Now free all 20 pages in the file\n");
	for (j=0;j<5;j++)
	{
		for (i=0;i<20;i=i++)
		{
			e=io_freepage(&fileid[j],&pids[j][i]);
			IO_FatalError("test3/io_freepage",e);
		}
	}

/* Clean up the disk */
	for (j=0;j<5;j++)
	{
		e = io_destroyfile(&fileid[j]);
		IO_FatalError("test3/io_destroyfile",e);
	}

	e = io_dismount(DEVICENAME);  /* unmount the device */
	IO_FatalError("test3/io_dismount",e);

	printf("\n End of test3\n");
}
