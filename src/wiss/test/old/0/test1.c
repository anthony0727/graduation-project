#
/* program to test mounting and dismounting a device */

extern	char	*io_error();

#define	DEVICENAME	"wiss0"
#define	IO_FatalError(p,c) if((int)(c)<0) \
		{printf("%s %s\n", p, io_error(c));io_final();exit(-1);}

main(argc,argv)
int	argc;
char	**argv;
{
	int 	i;
	int	e;	/* for returned status */
	int	volid;

/* warm up */
	wiss_checkflags(&argc,&argv);
	e = io_init();			/* initialize level 0 */
	IO_FatalError("test1/io_init", e);

/* simply test for mounting and dismounting */
	printf("About to mount and dismount %s 10 times\n", DEVICENAME);
	for (i = 0; i < 10; i++)
	{
		e = io_mount(DEVICENAME);  /* mount the device */
		IO_FatalError("test1/io_mount",e);
		e = io_dismount(DEVICENAME);  /* unmount the device */
		IO_FatalError("test1/io_dismount",e);
	}

/* dismount an unmounted device */
	printf("\nAbout to dismount an unmounted device - %s\n", DEVICENAME);
	e = io_dismount(DEVICENAME);  /* unmount a device */
	if (e >= 0) printf(" didn't catch the error\n");
	else printf(" error code correctly returned %s\n", io_error(e));

/* tests for io_volid */
	printf("\nDismount %s after it has been mounted and get its ID\n", 
		DEVICENAME);
	printf("  should get an error code\n");

	e = io_mount(DEVICENAME);  /* mount the device */
	IO_FatalError("test1/io_mount",e);
	volid = io_volid (DEVICENAME);  /* get the volume id */
	IO_FatalError("test1/io_volid",volid);
	if (e != volid) printf(" inconsistent volume ID %d or %d\n", e, volid);
	e = io_dismount(DEVICENAME);  /* unmount the device */
	IO_FatalError("test1/io_dismount",e);
	volid = io_volid (DEVICENAME);  /* get the volume id */
	if (volid>= 0) printf(" didn't catch the error\n");
	else printf(" error code correctly returned %s\n", 
		io_error(volid));

	printf("\nEnd of test1\n");

}

