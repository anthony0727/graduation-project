#include <wiss.h>

/* program to test readpage and writepage */
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
	int i,j,k,l;
	int volid;
	FID fileid[15];
	PID pids[20] [20]; /* i,j: i is for file, j is for pages  */
	int pages[10] [1024];  /* a page full of integers */
	int buf[1024];
	int sem;

/* initialize all data pages */
	for (j=0;j<10;j++) /* for all pages */
		for (i=0;i<1024;i++) pages[j][i] = j;

/* warm up */
	wiss_checkflags(&argc,&argv);
	e = io_init();  /* initialize level 0 */
	IO_FatalError("test4/io_init",e);
	volid = io_mount(DEVICENAME);  /* mount the device */
	IO_FatalError("test4/io_mount",volid);

	printf ("\n Device %s should have 11 extents of 10 pages each\n", 
		DEVICENAME);
	printf (" 1 is used for volume header, 10 for data\n");

/* Create files */
	printf ("\n Create 10 files of 10 pages each\n");
	printf (" Each file will require 1 extent \n");
	for (j=0;j<10;j++)
	{
		e = io_createfile(volid, 10, 100, &fileid[j]);
		IO_FatalError("test4",e);
		e = io_allocpages(&(fileid[j]), NULL, 10, &pids[j][0]);
		IO_FatalError("test4/io_allocpages",e);
	}

/* Write data to disk pages */
	printf("\n Next fill & write each page of each file full of data\n");
	for (j=0;j<10;j++)
	{
		l=j+1;
		if (l==10) l=0;
		for (i=0;i<10;i++)
		{
		    e = io_writepage(&pids[j][i],(char *)pages[l],SYNCH,&sem);
		    IO_FatalError("test4/io_writepage",e);
		    if (++l==10) l=0;
		}
	}

	printf("\n Before checking, dismount the device, do an io_init\n");
	printf(" and then remount it\n");

/* Clean the memory state */
	e = io_dismount(DEVICENAME);  /* unmount the device */
	IO_FatalError("test4/io_dismount",e);
	e = io_init();  /* initialize level 0 */
	IO_FatalError("test4/io_init",e);
	volid = io_mount(DEVICENAME);  /* mount the device */
	IO_FatalError("test4/io_mount",volid);

/* Read and check pages */
	printf("\n Next read each page of every file and check it\n");
	for (j=0;j<10;j++)
	{
		l=j+1;
		if (l==10) l=0;
		for (i=0;i<10;i++)
		{
		    	e = io_readpage(&pids[j][i],(char *)buf);
		    	IO_FatalError("test4/io_readpage",e);
		    	/* now check buf against page l */
		    	for (k=0;k<1024;k++) 
				if(pages[l][k] != buf[k]) 
		    printf("ERROR! data read doesn't match what was written\n");
		    	if (++l==10) l=0;
		}
	}

/* Try another set of data */
	printf("\n Next fill & write each page of each file full of data\n");
	printf(" with different data and check this\n");
	for (j=0;j<10;j++)
	{
		l=j+4;
		if (l>=10) l -= 10;
		for (i=0;i<10;i++)
		{
		    e = io_writepage(&pids[j][i],(char *)pages[l], SYNCH, &sem);
		    IO_FatalError("test4/io_writepage",e);
		    if (++l==10) l=0;
		}
	}

	printf("\n Before checking dismount the device, do an io_init\n");
	printf(" and then remount it\n");

	e = io_dismount(DEVICENAME);  /* unmount the device */
	IO_FatalError("test4/io_dismount",e);
	e = io_init();  /* initialize level 0 */
	IO_FatalError("test4/io_init",e);
	volid = io_mount(DEVICENAME);  /* mount the device */
	IO_FatalError("test4/io_mount",volid);

	printf("\n Next read each page of every file and check it\n");
	for (j=0;j<10;j++)
	{
		l=j+4;
		if (l>=10) l -= 10;
		for (i=0;i<10;i++)
		{
		    e = io_readpage(&pids[j][i],(char *)&buf[0]);
		    IO_FatalError("test4/io_readpage",e);
		    /* now check buf against page l */
		    for (k=0;k<1024;k++) 
			if(pages[l][k] != buf[k]) 
		    printf("ERROR! data read doesn't match what was written\n");
		    if (++l==10) l=0;
		}
	}
	e = io_checker(volid);

/* Clean up the disk */
	for (j=0;j<10;j++)
	{
		e = io_destroyfile(&fileid[j]);
		IO_FatalError("test4/io_destroyfile",e);
	}

	i = io_dismount(DEVICENAME);  /* unmount the device */
	IO_FatalError("test4/io_dismount",i);

	printf("\n End of test4\n");

}
