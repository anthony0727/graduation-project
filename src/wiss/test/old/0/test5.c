#include <wiss.h>

/* program to test automatic extension through allocation */
/* and to test readpage and writepage */
/* Proper test requires that wiss0 contain 11 extents of 10 pages each*/
/* 1 extents are used for the volume header so 10 are available for data*/

extern	char	*io_error();
#define	DEVICENAME	"wiss0"
#define	IO_FatalError(p,c) if((int)(c)<0) \
		{printf("%s %s\n", p, io_error(c));io_final();exit(-1);}

main(argc,argv)
int	argc;
char	**argv;
{
	int e;
	int i,ii,j,k,l;
	int volid;
	FID fileid[5];
	PID pids[5] [50]; /* i,j: i is for file, j is for pages  */
	int pages[50] [1024];  /* a page full of integers */
	int buf[1024];
	int sem;

/* initialize all data pages */
	for (j=0;j<11;j++) /* for all pages */
		for (i=0;i<1024;i++) pages[j][i] = j;

/* warm up */
	wiss_checkflags(&argc,&argv);
	e = io_init();  /* initialize level 0 */
	IO_FatalError("test5/io_init",e);
	volid = io_mount(DEVICENAME);  /* mount the device */
	IO_FatalError("test5/io_mount",volid);

	printf ("\n Device %s should have 11 extents of 10 pages each\n", 
		DEVICENAME);
	printf (" 1 is used for volume header, 10 for data\n");

/* Create 2 files */
	printf ("\n Create 2 files of 11 pages each\n");
	printf (" Each file will require two extents\n");
	for (j=0;j<2;j++)
	{
		e = io_createfile(volid, 11, 100, &fileid[j]);
		IO_FatalError("test5",e);
		e = io_allocpages(&(fileid[j]), NULL, 11, pids[j]);
		IO_FatalError("test5/io_allocpages",e);
	}

/* Write data to disk pages */
	printf("\n Next fill & write each page of each file full of data\n");
	for (j=0;j<2;j++)
	{
		for (i=0;i<11;i++)
		{
		    e = io_writepage(&pids[j][i],(char *) pages[i],SYNCH, &sem);
		    IO_FatalError("test5/io_writepage",e);
		}
	}

	printf("\n Before checking dismount the device, do an io_init ");
	printf("and then remount it\n");

/* Clean the memory state */
	e = io_dismount(DEVICENAME);  /* unmount the device */
	IO_FatalError("test5/io_dismount",e);
	e = io_init();  /* initialize level 0 */
	IO_FatalError("test5/io_init",e);
	volid = io_mount(DEVICENAME);  /* mount the device */
	IO_FatalError("test5/io_mount",volid);

/* Read & verify the pages */
	printf("\n Next read each page of every file and check it\n");
	for (j=0;j<2;j++)
	{
		for (i=0;i<11;i++)
		{
		    e = io_readpage(&pids[j][i],(char *)buf);
		    IO_FatalError("test5/io_readpage",e);
		    /* now check buf against page i */
		    for (k=0;k<1024;k++) 
			if(pages[i][k] != buf[k]) 
			{
		    printf("ERROR! data read doesn't match what was written\n");
				exit(-1);
			}
		}
	}
	e = io_checker(volid);

/* allocate and write more pages for each file */
	printf("\n Now each of the two files owns two extents");
	printf(" and has allocated 11 pages\n Next try to allocate");
	printf(" and write 39 more pages for each file\n");

	for (ii=0;ii<39;ii++)
	{
	  for (j=0;j<2;j++)
	  {
		e = io_allocpages(&(fileid[j]), pids[j],1,&pids[j][ii+11]);
		IO_FatalError("test5/io_allocpages",e);
		e=io_writepage(&pids[j][ii+11],(char *)pages[ii+11],SYNCH,&sem);
		IO_FatalError("test5/io_writepage",e);
	   }
	}

/* Read & verify all the pages */
	printf("\n Next read all 50 pages of every file and check them\n");
	for (j=0;j<2;j++)
	{
		for (i=0;i<50;i++)
		{
		    e = io_readpage(&pids[j][i],(char *)buf);
		    IO_FatalError("test5/io_readpage",e);
		    /* now check buf against page l */
		    for (k=0;k<1024;k++) 
			if(pages[i][k] != buf[k]) 
		    {
		    printf("ERROR! data read doesn't match what was written\n");
			exit(-1);
		    }
		}
	}

	printf("\n Check of both files was successful\n");
	printf("\n Volume should be full now. make sure by");
	printf("\n creating one more file.  Should get an error return\n");

	e = io_createfile(volid, 1, 100, &fileid[3]);
	if (e >= 0) printf(" io_createfile didn't catch the error!\n");
	else printf(" io_createfile correctly returned an error code %s\n", 
		io_error(e));
	e = io_checker(volid);

/* Clean up the disk */
	for (j=0;j<2;j++)
	{
		e = io_destroyfile(&fileid[j]);
		IO_FatalError("test5/io_destroyfile",e);
	}

	e = io_dismount(DEVICENAME);  /* unmount the device */
	IO_FatalError("test5/io_dismount",e);

	printf("\n End of test5\n");

}
