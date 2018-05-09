/* program to test mounting and dismounting a device */
/* remounting it and then exiting with the device mounted */

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
	int i;

	wiss_checkflags(&argc,&argv);
	i = io_init();			/* initialize level 0 */
	IOERROR("test4/io_init", i);
	
	i = bf_init();		/* initialize level 1 */
	BFERROR("test4/bf_init", i);

	i = st_init();		/* initialize level 2 */
	STERROR("test4/st_init", i);

	i = st_mount("wiss2");  /* mount the device wiss2 */
	STERROR("test4/st_mount", i);

	i = st_dismount("wiss2");  /* dismount the device wiss2 */
	STERROR("test4/st_dismount", i);

	printf(" exit with the device mounted\n");
	i = st_mount("wiss2");  /* mount the device wiss2 */
	STERROR("test4/st_mount", i);

	/* exit ! */

}
