/* program to test file renaming */
/* A disk with at least 16 extents is required */


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
	int	 j, i, vol;
	char	filename[100];
	char	newname[100];

	printf("This test requires a disk with AT LEAST 16 extents\n");

/* initialization */

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test16/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test16/bf_init", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test16/st_init", i);

/* creating files */

	printf("About to create 12 files \n");
	vol = st_mount("wiss2");  /* mount the device wiss2 */
	STERROR("test16/st_mount", vol);
	for (j = 0; j < 12; j++)
	{
		sprintf(filename, "tfile%2d", j);
		i = st_createfile(vol, filename, 1, 90,90);
		STERROR("test16/st_create", i);
	}
	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test16/st_dismount", i);

/* renaming files */

	i = st_init();		/* initialize level 2 */
	STERROR("test16/st_init", i);

	vol = st_mount("wiss2");  /* mount the device wiss2 */
	STERROR("test16/st_mount", vol);

	printf("About to rename 12 files \n");
	for (j = 0; j < 12; j++)
	{
		sprintf(filename, "tfile%2d", j);
		sprintf(newname, "newname%2d", j);
		i = st_rename(vol, newname, filename);
		STERROR("test16/st_rename", i);
	}

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test16/st_dismount", i);

/* destroying files */

	i = st_init();		/* initialize level 2 */
	STERROR("test16/st_init", i);

	vol = st_mount("wiss2");  /* mount the device wiss2 */
	STERROR("test16/st_mount", vol);

	printf("About to destroy 12 files \n");
	for (j = 0; j < 12; j++)
	{
		sprintf(newname, "newname%2d", j);
		i = st_destroyfile(vol, newname);
		STERROR("test16/st_destroyfile", i);
	}

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test16/st_dismount", i);

	printf(" END OF TEST !\n");

}

