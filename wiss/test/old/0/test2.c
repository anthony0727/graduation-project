#include <wiss.h>

/* program to test space allocation within a volume */
/* to test destroying a file with allocated pages in it */
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
	int i, j, k;
	int volid;
	int e;
	FID fileid[15];
	PID pids[20] [20]; /* i,j: i is for file, j is for pages  */

/* warm up */
	wiss_checkflags(&argc,&argv);
	e = io_init();  /* initialize level 0 */
	IO_FatalError("test2/io_init",e);
	volid = io_mount(DEVICENAME);  /* mount the device wiss0 */
	IO_FatalError("test2/io_mount",volid);

	printf ("\n Device %s should have 11 extents of 10 pages each\n", 
		DEVICENAME);
	printf (" 1 is used for volume header, 10 for data\n");

/* Create 10 files */
	printf ("\n Create 10 files of 10 pages each");
	for (j=0;j<10;j++)
	{
		e = io_createfile(volid, 10, 100, &fileid[j]);
		IO_FatalError("test2/io_createfile",e);
		i = io_allocpages(&fileid[j], NULL, 10, &pids[j][0]);
		IO_FatalError("test2/io_allocpages",i);
	}
	io_checker(volid);

/* make sure no leftovers */
	e = io_dismount(DEVICENAME);
	IO_FatalError("test2/io_dismount",e);
	e = io_init();
	IO_FatalError("test2/io_init",e);
	volid = io_mount(DEVICENAME);  /* mount the device wiss0 */
	IO_FatalError("test2/io_mount",volid);

/* Destroys all the files */
	printf("\n Next destroy files 1-10 each which contain 10 pages\n");
	for (j=0;j<10;j++)
	{
		e = io_destroyfile(&fileid[j]);
		IO_FatalError("test2/io_destroyfile",e);
	}
	e = io_checker(volid);
	IO_FatalError("test2/io_checker",e);

/* make sure no leftovers */
	e = io_dismount(DEVICENAME);
	IO_FatalError("test2/io_dismount",e);
	e = io_init();
	IO_FatalError("test2/io_init",e);
	volid = io_mount(DEVICENAME);  /* mount the device wiss0 */
	IO_FatalError("test2/io_mount",volid);

/* Create 5 files */
	printf ("\n Create 5 files of 20 pages each\n");
	printf (" Each file will require two extents\n");
	for (j=0;j<5;j++)
	{
		e = io_createfile(volid, 20, 100, &fileid[j]);
		IO_FatalError("test2/io_createfile",e);
		e = io_allocpages(&(fileid[j]), NULL, 20, &pids[j][0]);
		IO_FatalError("test2/io_allocpages",e);
	}
	e = io_checker(volid);
	IO_FatalError("test2/io_checker",e);

/* make sure no leftovers */
	e = io_dismount(DEVICENAME);
	IO_FatalError("test2/io_dismount",e);
	e = io_init();
	IO_FatalError("test2/io_init",e);
	volid = io_mount(DEVICENAME);  /* mount the device wiss0 */
	IO_FatalError("test2/io_mount",volid);

/* Destroys all the files */
	printf("\n Next destroy files 1-5 each which contain 20 pages\n");
	for (j=0;j<5;j++)
	{
		e = io_destroyfile(&fileid[j]);
		IO_FatalError("test2/io_destroyfile",e);
	}
	e = io_checker(volid);
	IO_FatalError("test2/io_checker",e);

	e = io_dismount(DEVICENAME);  /* unmount the device wiss0 */
	IO_FatalError("test2/io_dismount",e);

	printf("\n End of test2\n");
}
