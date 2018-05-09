/* program to test creating and destroying a file */
/* (1) create a file
   (2) destroy the file just created
*/


extern char *io_error(), *bf_error(), *st_error();

#define	IOERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, io_error((int)(c)));io_final(); exit(-1);}
#define	BFERROR(p,c)	if((int)(c)<0) \
		{printf("%s %s\n", p, bf_error((int)(c)));bf_final(); io_final(); exit(-1);}

#define	ERROR(p,c)	if((int)(c)<0) printf("%s %s\n", p, st_error((int)(c)))
#define	STERROR(p,c)	if((int)(c)<0) {ERROR(p, c); st_final(); bf_final();\
				io_final();}


main(argc,argv)
int	argc;
char	**argv;
{
	int i, vol;

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test6/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test6/bf_init", i);

	i = st_init();		/* initialize level 2 */
	STERROR("test6/st_init", i);

/* testing of createfile */
	vol = st_mount("wiss2");  /* mount the device wiss2 */
	STERROR("test6/st_mount", vol);

	printf("About to create the file - tfile1\n");
	i = st_createfile(vol, "tfile1", 9, 90,90);
	STERROR("test6/st_createfile", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test6/st_dismount", i);

/* testing of destroyfile */
	/* clear file table */
	i = st_init();		/* initialize level 2 */
	STERROR("test6/st_init", i);

	vol = st_mount("wiss2");  /* mount the device wiss2 */
	STERROR("test6/st_mount", vol);

	printf("About to destory the file - tfile1\n");
	i = st_destroyfile(vol, "tfile1");
	STERROR("test6/st_destroyfile", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test6/st_dismount", i);
}
