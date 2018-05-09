/* program to test creating and destroying a file */

extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, bf_error((int)(c)));bf_final(); io_final(); exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();}
#define TERROR(p,c) 	if((int)(c)<0) ERROR(p,(int)(c));\
			else printf("%s did not catch error !\n", p)

main(argc,argv)
int	argc;
char	**argv;
{
	int i, vol;

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test7/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test7/bf_init", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test7/st_init", i);

	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test7/st_mount", vol);

/* create the same file twice */
	printf(" create tfile1 for the first time\n");
	i = st_createfile(vol, "tfile1", 9, 90,90);
	STERROR("test7/st_createfile", i);
	printf(" create tfile1 for the second time\n");
	i = st_createfile(vol, "tfile1", 9, 90,90);
	TERROR("test7/st_createfile", i);

	i = st_destroyfile(vol, "tfile1");
	STERROR("test8/st_destroyfile", i);

/* create a file on an unmounted device */
	printf(" create tfile1 on an unmounted device\n");
	i = st_createfile(vol + 1, "tfile1", 9, 90,90);
	TERROR("test7/st_createfile", i);

/* test parameter NumPages */
	printf(" create a file with 1000 pages\n");
	i = st_createfile(vol, "tfile1", 1000, 90,90);
	TERROR("test7/st_createfile", i);

	printf(" create a file with extent fill factor 0\n");
	i = st_createfile(vol, "tfile1", 10, 0,90);
	TERROR("test7/st_createfile", i);

/* test parameter PageFillFactor */
	printf(" create a file with Page fill factor 0\n");
	i = st_createfile(vol, "tfile1", 10, 10, 0);
	TERROR("test7/st_createfile", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test7/st_dismount", i);
}
