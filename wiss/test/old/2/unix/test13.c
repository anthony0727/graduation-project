/* program to test creating and destroying files */

/* repeatly creating files and then destorying them, 
   this test is primary used for testing level 2 file directory routines.
   this test requires a disk which contains at least 30 extents
   and the routines in concern (ST_directory.c) must be compiled 
   with "DEBUG" flag
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
	int 	j, i, vol;
	char	filename[100];

	printf("This test requires a disk with AT LEAST 30 extents\n");

/* initialization */

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test13/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test13/bf_init", i);

	i = st_init();			/* initialize level 2 */
	STERROR("test13/st_init", i);

/* testing  st_createfile */

	printf("About to create 25 files \n");
	for (j = 0; j < 25; j++)
	{

		vol = st_mount("wiss2");  /* mount the device wiss2 */
		STERROR("test13/st_mount", vol);

		sprintf(filename, "tfile%2d", j);
		i = st_createfile(vol, filename, 1, 90,90);
		STERROR("test13/st_createfile", i);

		i = st_dismount("wiss2");  /* dismount the device wiss2 */
		STERROR("test13/st_dismount", i);

	}

/* testing st_destroyfile */

	i = st_init();			/* reinitialize level 2 */
	STERROR("test13/st_init", i);

	vol = st_mount("wiss2");  	/* mount the device wiss2 */
	STERROR("test13/st_mount", vol);

	printf("About to destroy 25 files \n");
	for (j = 0; j < 25; j++)
	{
		sprintf(filename, "tfile%2d", j);
		i = st_destroyfile(vol, filename);
		STERROR("test13/st_destroyfile", i);
	}

	i = st_dismount("wiss2"); 	 /* dismount the device wiss2 */
	STERROR("test13/st_dismount", i);

	printf(" END OF TEST !\n");

}
